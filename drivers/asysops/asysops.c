/*
 * asysops.c
 *
 * This driver is designed to support Linux shared memory based,
 * thread and process communication mechanism. First step designed
 * to support shared memory, message, and lock. This char device 
 * drive is designed to create one device node.
 *
 * InfoTM All Rights Reserved.
 *
 * Sololz <sololz.luo@gmail.com>
 *
 * 2010-06-30
 * 	Create this char device driver.
 * 2010-07-01
 * 	Finish this char device driver, version 1.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/io.h>

#include <asm/uaccess.h>
#include <asm/pgtable.h>

#include "asysops.h"

/* GLOBAL VARIABLES */
static struct class	*asysops_class		= NULL;
static unsigned int 	asysops_inst_count	= 0;

static unsigned int	asysops_shmem_count	= 0;
static asysops_shmem_t	asysops_shmem[SHMEM_MAX_ALLOC_COUNT];

static unsigned int	asysops_lock_count	= 0;
static asysops_lock_t	asysops_lock[LOCK_MAX_ALLOC_COUNT];

static unsigned int	asysops_event_count	= 0;
static asysops_event_t	asysops_event[EVENT_MAX_ALLOC_COUNT];

static struct mutex	common_mutex;	/* Common asysops mutex lock, lock for asysops instance */
static struct mutex	shmem_mutex;	/* Lock shmem resource */
static struct mutex	event_mutex;	/* Lock message resource */
static struct mutex	lock_mutex;	/* Lock lock resource */

/*
 * asysops_open()
 *
 * System call open. Right now, shared memory, message and lock
 * have the same instance structure. So I set a instance type 
 * mark to sign them.
 */
static int asysops_open(struct inode *inode, struct file *file)
{
	int		ret	= 0;
	asysops_inst_t	*inst	= NULL;

	ops_debug("Get into asysops device driver\n");

	mutex_lock(&common_mutex);

	if(asysops_inst_count >= ASYSOPS_MAX_INST_COUNT)
	{
		ops_error("No instance for asysops open\n");
		ret = -EPERM;
		goto ops_open_return;
	}

	/* Alloc memory for a new asysops instance */
	inst = (asysops_inst_t *)kmalloc(sizeof(asysops_inst_t), GFP_KERNEL);
	if(inst == NULL)
	{
		ops_error("Allocate memory for a new asysops instance error\n");
		ret = -ENOMEM;
		goto ops_open_return;
	}
	memset(inst, 0x00, sizeof(asysops_inst_t));

	file->private_data = inst;
	asysops_inst_count++;

ops_open_return:
	mutex_unlock(&common_mutex);

	ops_debug("Open asysops device node OK\n");

	return ret;
}

/*
 * asysops_release()
 *
 * System call release. Release should check all global resoures, 
 * if some are idle, release function will release them. All resources
 * for shared memory, message and lock is not suggested to free in 
 * user space level. Because, user never know the suitable time to 
 * release them, this release function will recycle all memory resources
 * related to shmem, msg, lock if current instance is not been using.
 */
static int asysops_release(struct inode *inode, struct file *file)
{
	int		i	= 0;
	int		ret	= 0;
	asysops_inst_t	*inst	= NULL;

	mutex_lock(&common_mutex);

	inst = (asysops_inst_t *)(file->private_data);
	if(inst == NULL)
	{
		ops_debug("File data unexpected to be NULL\n");
		ret = -EFAULT;
		goto ops_release_return;
	}

	if(inst->type == ASYSOPS_SHMEM_INST)
	{
		for(i = 0; i < SHMEM_MAX_ALLOC_COUNT; i++)
		{
			if(asysops_shmem[i].key == inst->shmem_key)
			{
				if(asysops_shmem[i].count > 0)
					asysops_shmem[i].count--;

				if((asysops_shmem[i].count == 0) && (asysops_shmem[i].virt != NULL))
				{
					kfree(asysops_shmem[i].virt);
					memset(&asysops_shmem[i], 0x00, sizeof(asysops_shmem_t));
					if(asysops_shmem_count > 0)
						asysops_shmem_count--;
				}

				break;
			}
		}
	}
	else if(inst->type == ASYSOPS_EVENT_INST)
	{
		for(i = 0; i < EVENT_MAX_ALLOC_COUNT; i++)
		{
			if(asysops_event[i].key == inst->event_key)
			{
				if(asysops_event[i].count > 0)
					asysops_event[i].count--;

				if(asysops_event[i].count == 0)
				{
					if(asysops_event[i].virt != NULL)
						kfree(asysops_event[i].virt);

					if(asysops_event[i].data_virt != NULL)
						kfree(asysops_event[i].data_virt);

					mutex_destroy(asysops_event[i].mutex);
					kfree(asysops_event[i].event);
					memset(&asysops_event[i], 0x00, sizeof(asysops_event_t));

					if(asysops_event_count > 0)
						asysops_event_count--;
				}

				break;
			}
		}
	}
	else if(inst->type == ASYSOPS_LOCK_INST)
	{
		for(i = 0; i < LOCK_MAX_ALLOC_COUNT; i++)
		{
			if(asysops_lock[i].key == inst->lock_key)
			{
				if(asysops_lock[i].count > 0)
					asysops_lock[i].count--;

				if(asysops_lock[i].count == 0)
				{
					if(asysops_lock[i].virt != NULL)
						kfree(asysops_lock[i].virt);

					mutex_destroy(asysops_lock[i].lock);
					kfree(asysops_lock[i].lock);
					memset(&(asysops_lock[i]), 0x00, sizeof(asysops_lock_t));
					if(asysops_lock_count > 0)
						asysops_lock_count--;
					ops_debug("Lock freed, %d\n", i);
				}

				break;
			}
		}
	}
	else if(inst->type == ASYSOPS_NONE_INST)
	{
		/* Quite normal */
	}
	else 
	{
		ops_error("Error instance type\n");
		ret = -EFAULT;
		goto ops_release_return;
	}

	if(asysops_inst_count > 0)
		asysops_inst_count--;

ops_release_return:
	/* Free an asysops instance */
	if(inst != NULL)
		kfree(inst);

	mutex_unlock(&common_mutex);

	return ret;
}

/*
 * asysops_ioctl()
 *
 * System call ioctl.
 */
static int asysops_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int		i	= 0;
	int		loc	= 0;
	int		ret	= 0;
	int		out	= 0;
	asysops_inst_t	*inst	= NULL;
	unsigned int	key	= 0;
	unsigned int	timeout	= 0;
	unsigned int	size	= 0;
	shmem_param_t	shmem;
	event_param_t	event;
	lock_param_t	lock;

	memset(&shmem, 0x00, sizeof(shmem_param_t));
	memset(&event, 0x00, sizeof(event_param_t));

	ops_debug("Get asysops ioctl\n");

	inst = (asysops_inst_t *)(file->private_data);
	if(inst == NULL)
	{
		ops_error("File data is NULL in ioctl\n");
		return -EFAULT;
	}

	switch(cmd)
	{
		/* Get an exits shared memory or create new one */
		case ASYSOPS_CMD_GET_SHMEM:
			ops_debug("ASYSOPS_CMD_GET_SHMEM\n");

			/* Check instance type */
			if((inst->type != ASYSOPS_NONE_INST) && (inst->type != ASYSOPS_SHMEM_INST))
			{
				ops_error("Instance is not a shmem instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}
			inst->type = ASYSOPS_SHMEM_INST;

			out = copy_from_user(&shmem, (shmem_param_t *)arg, sizeof(shmem_param_t));
			key = shmem.key;
			if(key == 0)
			{
				ops_error("Input key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			size = shmem.size;
			if((size == 0) || (size > SHMEM_MAX_SIZE))
			{
				ops_error("Input size is invalid\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			mutex_lock(&shmem_mutex);
			for(i = 0; i < SHMEM_MAX_ALLOC_COUNT; i++)
			{
				/* Check whether key exists */
				if(asysops_shmem[i].key == key)
				{
					asysops_shmem[i].count++;
					inst->shmem_key = key;
					shmem.phys = asysops_shmem[i].phys;
					ret = ASYSOPS_RET_EXIST;

					break;
				}

				/* Create a new shmem block */
				if(i == SHMEM_MAX_ALLOC_COUNT - 1)
				{
					if(asysops_shmem_count >= SHMEM_MAX_ALLOC_COUNT)
					{
						ops_error("No more shmem resource to create a new one\n");
						ret = -EFAULT;
						mutex_unlock(&shmem_mutex);
						goto ops_ioctl_return;
					}

					for(loc = 0; loc < SHMEM_MAX_ALLOC_COUNT; loc++)
					{
						if(asysops_shmem[loc].key == 0)
							break;

						if(loc == SHMEM_MAX_ALLOC_COUNT - 1)
						{
							ops_error("No more shmem resource to create a new one\n");
							ret = -EFAULT;
							mutex_unlock(&shmem_mutex);
							goto ops_ioctl_return;
						}
					}

					/* Alloc shared memory */
					asysops_shmem[loc].virt = (unsigned int *)kmalloc(size, GFP_KERNEL);
					if(asysops_shmem[loc].virt == NULL)
					{
						ops_error("Allocate for shared memory error\n");
						ret = -ENOMEM;
						mutex_unlock(&shmem_mutex);
						goto ops_ioctl_return;
					}
					memset(asysops_shmem[loc].virt, 0x00, size);
					asysops_shmem[loc].phys = virt_to_phys((void *)(asysops_shmem[loc].virt));
					shmem.phys = asysops_shmem[loc].phys;

					inst->shmem_key			= key;
					asysops_shmem[loc].key		= key;
					asysops_shmem[loc].count	= 1;
					asysops_shmem_count++;
				}
			}
			mutex_unlock(&shmem_mutex);

			out = copy_to_user((shmem_param_t *)arg, &shmem, sizeof(shmem_param_t));

			break;

		/* Free an exits shared memory */
		case ASYSOPS_CMD_FREE_SHMEM:
			ops_debug("ASYSOPS_CMD_FREE_SHMEM\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_SHMEM_INST)
			{
				ops_error("Instance is not a shmem instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			if(asysops_shmem_count == 0)
			{
				ops_debug("No shmem to free\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			out = copy_from_user(&key, (unsigned int *)arg, 4);
			if(key == 0)
			{
				ops_error("Input key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			mutex_lock(&shmem_mutex);
			for(i = 0; i < SHMEM_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_shmem[i].key == key)
				{
					if(asysops_shmem[i].virt != NULL)
						kfree(asysops_shmem[i].virt);
					memset(&(asysops_shmem[i]), 0x00, sizeof(asysops_shmem_t));
					asysops_shmem_count--;

					break;
				}

				if(i == SHMEM_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Key correspond shmem not exits\n");
					ret = -EFAULT;
					mutex_unlock(&shmem_mutex);
					goto ops_ioctl_return;
				}
			}
			mutex_unlock(&shmem_mutex);

			break;

		/* Get an exits lock or create a new one */
		case ASYSOPS_CMD_GET_LOCK:
			ops_debug("ASYSOPS_CMD_GET_LOCK\n");

			/* Check instance type */
			if((inst->type != 0) && (inst->type != ASYSOPS_LOCK_INST))
			{
				ops_error("Instance is not a lock instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}
			inst->type = ASYSOPS_LOCK_INST;

			out = copy_from_user(&lock, (lock_param_t *)arg, sizeof(lock_param_t));	/* FIXME: unsigned int type sometime is not 4 bytes */
			key = lock.key;
			if(key == 0)
			{
				ops_error("Lock key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			/* Find the key whether exits in global lock structure */
			mutex_lock(&lock_mutex);
			for(i = 0; i < LOCK_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_lock[i].key == key)
				{
					/* Find a lock the same key */
					asysops_lock[i].count++;
					inst->lock_key = key;
					lock.phys = asysops_lock[i].phys;
					ret = ASYSOPS_RET_EXIST;
					ops_debug("Get an exists lock, %d\n", i);

					break;
				}

				/* Create new one */
				if(i == LOCK_MAX_ALLOC_COUNT - 1)
				{
					if(asysops_lock_count >= LOCK_MAX_ALLOC_COUNT)
					{
						ops_error("No more lock resource to create a new one\n");
						ret = -EFAULT;
						mutex_unlock(&lock_mutex);
						goto ops_ioctl_return;
					}

					for(loc = 0; loc < LOCK_MAX_ALLOC_COUNT; loc++)
					{
						if(asysops_lock[loc].key == 0)
							break;

						if(loc == LOCK_MAX_ALLOC_COUNT - 1)
						{
							ops_error("No more lock resource to create a new one\n");
							ret = -EFAULT;
							mutex_unlock(&lock_mutex);
							goto ops_ioctl_return;
						}
					}

					asysops_lock[loc].lock = (struct mutex *)kmalloc(sizeof(struct mutex), GFP_KERNEL);
					if(asysops_lock[loc].lock == NULL)
					{
						ops_error("Allocate lock structure error\n");
						ret = -EFAULT;
						mutex_unlock(&lock_mutex);
						goto ops_ioctl_return;
					}
					mutex_init(asysops_lock[loc].lock);

					/* Alloc lock control data */
					if(lock.size != 0)
					{
						asysops_lock[loc].virt = (unsigned int *)kmalloc(lock.size, GFP_KERNEL);
						if(asysops_lock[loc].virt == NULL)
						{
							ops_error("Allocate for lock control data error\n");
							ret = -ENOMEM;
							mutex_unlock(&lock_mutex);
							goto ops_ioctl_return;
						}
						memset(asysops_lock[loc].virt, 0x00, lock.size);
						asysops_lock[loc].phys = (unsigned int)virt_to_phys(asysops_lock[loc].virt);
						asysops_lock[loc].size = lock.size;
						lock.phys = asysops_lock[loc].phys;
					}

					inst->lock_key		= key;
					asysops_lock[loc].key	= key;
					asysops_lock[loc].count = 1;
					asysops_lock_count++;
					ops_debug("Create a new exists lock, %d\n", loc);
				}
			}
			mutex_unlock(&lock_mutex);

			out = copy_to_user((lock_param_t *)arg, &lock, sizeof(lock_param_t));

			break;

		/*
		 * Free an exits lock.
		 * This ioctl command is not commend to call, unless if you really 
		 * exactly know when an lock can be freed. Normally, user don't have 
		 * to manully call this ioctl. When last instance relates to a lock
		 * release, I will check this and free lock.
		 */
		case ASYSOPS_CMD_FREE_LOCK:
			ops_debug("ASYSOPS_CMD_FREE_LOCK\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_LOCK_INST)
			{
				ops_error("Instance is not a lock instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			if(asysops_lock_count == 0)
			{
				ops_error("No lock to be freed\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			out = copy_from_user(&key, (unsigned int *)arg, 4);	/* FIXME: unsigned int type sometime is not 4 bytes */
			if(key == 0)
			{
				ops_error("Lock key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			mutex_lock(&lock_mutex);
			for(i = 0; i < LOCK_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_lock[i].key == key)
				{
					if(asysops_lock[i].lock != NULL)
					{
						mutex_destroy(asysops_lock[i].lock);
						kfree(asysops_lock[i].lock);
					}

					if(asysops_lock[i].virt != NULL)
						kfree(asysops_lock[i].virt);

					memset(&(asysops_lock[i]), 0x00, sizeof(asysops_lock_t));
					asysops_lock_count--;

					break;
				}

				if(i == LOCK_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Key correspond lock not exits\n");
					ret = -EFAULT;
					mutex_unlock(&lock_mutex);
					goto ops_ioctl_return;
				}
			}
			mutex_unlock(&lock_mutex);

			break;

		/* Hold a lock */
		case ASYSOPS_CMD_HOLD_LOCK:
			ops_debug("ASYSOPS_CMD_HOLD_LOCK\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_LOCK_INST)
			{
				ops_error("Instance is not a lock instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			key = inst->lock_key;
			if(key == 0)
			{
				ops_error("Lock key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			mutex_lock(&lock_mutex);
			for(i = 0; i < LOCK_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_lock[i].key == key)
					break;

				if(i == LOCK_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Key correspond lock not exists\n");
					ret = -EFAULT;
					mutex_unlock(&lock_mutex);
					goto ops_ioctl_return;
				}
			}

			if((asysops_lock[i].lock == NULL) || \
					(asysops_lock[i].count == 0))
			{
				ops_error("Lock with key %d error\n", asysops_lock[i].key);
				ret = -EFAULT;
				mutex_unlock(&lock_mutex);
				goto ops_ioctl_return;
			}
			mutex_unlock(&lock_mutex);

			/* Hold lock here */
			mutex_lock(asysops_lock[i].lock);
			ops_debug("Lock number %d\n", i);

			break;

		/* Post a lock */
		case ASYSOPS_CMD_POST_LOCK:
			ops_debug("ASYSOPS_CMD_POST_LOCK\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_LOCK_INST)
			{
				ops_error("Instance is not a lock instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			key = inst->lock_key;
			if(key == 0)
			{
				ops_error("Lock key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			mutex_lock(&lock_mutex);
			for(i = 0; i < LOCK_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_lock[i].key == key)
					break;

				if(i == LOCK_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Key correspond lock not exists\n");
					ret = -EFAULT;
					mutex_unlock(&lock_mutex);
					goto ops_ioctl_return;
				}
			}

			if((asysops_lock[i].lock == NULL) || \
					(asysops_lock[i].count == 0))
			{
				ops_error("Lock with key %d error\n", asysops_lock[i].key);
				ret = -EFAULT;
				mutex_unlock(&lock_mutex);
				goto ops_ioctl_return;
			}
			mutex_unlock(&lock_mutex);

			/* Unlock */
			mutex_unlock(asysops_lock[i].lock);

			break;

		/* Get an exits message or create a new one */
		case ASYSOPS_CMD_GET_EVENT:
			ops_debug("ASYSOPS_CMD_GET_EVENT\n");

			/* Check instance type */
			if((inst->type != 0) && (inst->type != ASYSOPS_EVENT_INST))
			{
				ops_error("Instance is not a lock instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}
			inst->type = ASYSOPS_EVENT_INST;

			out = copy_from_user(&event, (event_param_t *)arg, sizeof(event_param_t));
			key = event.key;
			if(key == 0)
			{
				ops_error("Lock key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			mutex_lock(&event_mutex);
			for(i = 0; i < EVENT_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_event[i].key == key)
				{
					/* Find an exits event */
					inst->event_key = key;
					(asysops_event[i].key)++;
					event.phys = asysops_event[i].phys;
					event.data_phys = asysops_event[i].data_phys;
					ret = ASYSOPS_RET_EXIST;

					break;
				}

				/* Create a new event */
				if(i == EVENT_MAX_ALLOC_COUNT - 1)
				{
					if(asysops_event_count >= EVENT_MAX_ALLOC_COUNT)
					{
						ops_error("No more evnet resource to create a new one\n");
						ret = -EFAULT;
						mutex_unlock(&event_mutex);
						goto ops_ioctl_return;
					}

					/* Find an not used evnet structure */
					for(loc = 0; loc < EVENT_MAX_ALLOC_COUNT; loc++)
					{
						if(asysops_event[loc].key == 0)
							break;

						if(loc == EVENT_MAX_ALLOC_COUNT - 1)
						{
							ops_error("No more event resource to create a new one\n");
							ret = -EFAULT;
							mutex_unlock(&event_mutex);
							goto ops_ioctl_return;
						}
					}

					asysops_event[loc].event = (wait_queue_head_t *)kmalloc(sizeof(wait_queue_head_t), GFP_KERNEL);
					if(asysops_event[loc].event == NULL)
					{
						ops_error("Allocate lock structure error\n");
						ret = -EFAULT;
						mutex_unlock(&event_mutex);
						goto ops_ioctl_return;
					}
					init_waitqueue_head(asysops_event[loc].event);
					mutex_init(&(asysops_event[loc].mutex));

					/* Allocate message control */
					if(event.size > 0)
					{
						asysops_event[loc].virt = (unsigned int *)kmalloc(event.size, GFP_KERNEL);
						if(asysops_event[loc].virt == NULL)
						{
							ops_error("Allocate event data error\n");
							ret = -ENOMEM;
							mutex_unlock(&event_mutex);
							goto ops_ioctl_return;
						}
						memset(asysops_event[loc].virt, 0x00, event.size);
						asysops_event[loc].phys = virt_to_phys(asysops_event[loc].virt);
						asysops_event[loc].size = event.size;
						event.phys = asysops_event[loc].phys;
					}

					/* Allocate data message */
					if(event.data_size)
					{
						asysops_event[loc].data_virt = (unsigned int *)kmalloc(event.data_size, GFP_KERNEL);
						if(asysops_event[loc].data_virt == NULL)
						{
							ops_error("Allocate event data error\n");
							ret = -ENOMEM;
							mutex_unlock(&event_mutex);
							goto ops_ioctl_return;
						}
						memset(asysops_event[loc].data_virt, 0x00, event.data_size);
						asysops_event[loc].data_phys = virt_to_phys(asysops_event[loc].data_virt);
						asysops_event[loc].data_size = event.data_size;
						event.data_phys = asysops_event[loc].data_phys;
					}

					inst->event_key	= key;
					asysops_event[loc].key = key;
					asysops_event[loc].count = 1;
					asysops_event_count++;
				}
			}

			mutex_unlock(&event_mutex);

			out = copy_to_user((event_param_t *)arg, &event, sizeof(event_param_t));

			break;

		/* Free an exits message */
		case ASYSOPS_CMD_FREE_EVENT:
			ops_debug("ASYSOPS_CMD_FREE_EVENT\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_EVENT_INST)
			{
				ops_error("Instance is not a event instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			if(asysops_event_count == 0)
			{
				ops_error("No lock to be freed\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			out = copy_from_user(&key, (unsigned int *)arg, 4);	/* FIXME: unsigned int type sometime is not 4 bytes */
			if(key == 0)
			{
				ops_error("Lock key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			mutex_lock(&event_mutex);
			for(i = 0; i < EVENT_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_event[i].key == key)
				{
					if(asysops_event[i].event != NULL)
					{
						if(asysops_event[i].virt != NULL)
							kfree(asysops_event[i].virt);

						if(asysops_event[i].data_virt != NULL)
							kfree(asysops_event[i].data_virt);

						mutex_destroy(&(asysops_event[i].mutex));
						kfree(asysops_event[i].event);
					}
					memset(&asysops_event[i], 0x00, sizeof(asysops_event_t));
					asysops_event_count++;

					break;
				}

				if(i == EVENT_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Key correspond event not exits\n");
					ret = -EFAULT;
					mutex_unlock(&event_mutex);
					goto ops_ioctl_return;
				}
			}
			mutex_unlock(&event_mutex);

			break;

		/* Send message */
		case ASYSOPS_CMD_SEND_EVENT:
			ops_debug("ASYSOPS_CMD_SEND_EVENT\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_EVENT_INST)
			{
				ops_error("Instance is not a event instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			/* Get event key */
			key = inst->event_key;
			if(key == 0)
			{
				ops_error("Instance is not a event instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			/* Find key correspond event */
			mutex_lock(&event_mutex);
			for(i = 0; i < EVENT_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_event[i].key == key)
					break;

				if(i == EVENT_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Key correspond event not exists\n");
					ret = -EFAULT;
					mutex_unlock(&event_mutex);
					goto ops_ioctl_return;
				}
			}

			/* Check event */
			if((asysops_event[i].event == NULL) || \
					(asysops_event[i].count == 0))
			{
				ops_error("Send event with key %d error\n", asysops_event[i].key);
				ret = -EFAULT;
				mutex_unlock(&event_mutex);
				goto ops_ioctl_return;
			}
			mutex_unlock(&event_mutex);

			mutex_lock(&(asysops_event[i].mutex));
			asysops_event[i].cond = 1;
			if(asysops_event[i].mark == 1)
				wake_up_interruptible(asysops_event[i].event);
			else
			{
				/* Wait has not been set, so no need to send */
			}
			asysops_event[i].mark = 0;
			mutex_unlock(&(asysops_event[i].mutex));

			break;

		/* Wait message */
		case ASYSOPS_CMD_WAIT_EVENT:
			ops_debug("ASYSOPS_CMD_WAIT_EVENT\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_EVENT_INST)
			{
				ops_error("Instance is not a event instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			out = copy_from_user(&timeout, (unsigned int *)arg, 4);

			key = inst->event_key;
			if(key == 0)
			{
				ops_error("Instance for event wait key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			/* Find event */
			mutex_lock(&event_mutex);
			for(i = 0; i < EVENT_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_event[i].key == key)
					break;

				if(i == EVENT_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Correspond event with key %d not exists\n", key);
					ret = -EFAULT;
					mutex_unlock(&event_mutex);
					goto ops_ioctl_return;
				}
			}
			mutex_unlock(&event_mutex);

			mutex_lock(&(asysops_event[i].mutex));
			if(asysops_event[i].cond == 0)
			{
				if(timeout == 0)
				{
					asysops_event[i].mark = 1;
					wait_event_interruptible(*(asysops_event[i].event), asysops_event[i].cond == 1);
				}
				else
				{
					unsigned int jiffies_timeout = 0;

					jiffies_timeout = (HZ * timeout) / 1000;
					if(jiffies_timeout < 1)
						jiffies_timeout = 1;

					/* Timeout is supposed to be 1ms unit */
					asysops_event[i].mark = 1;
					mutex_unlock(&(asysops_event[i].mutex));
					wait_event_interruptible_timeout(*(asysops_event[i].event), \
							asysops_event[i].cond == 1, \
							jiffies_timeout);
					mutex_lock(&(asysops_event[i].mutex));
					if(asysops_event[i].cond == 0)
						ret = ETIMEDOUT;
				}
			}
			else
			{
				/* Send has been called before wait, so don't wait anymore */
			}
			asysops_event[i].cond = 0;	/* Reset cond mark */
			mutex_unlock(&(asysops_event[i].mutex));

			break;

		case ASYSOPS_CMD_RESET_EVENT:
			ops_debug("ASYSOPS_CMD_RESET_EVENT\n");

			/* Check instance type */
			if(inst->type != ASYSOPS_EVENT_INST)
			{
				ops_error("Instance is not a event instance\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			key = inst->event_key;
			if(key == 0)
			{
				ops_error("Instance for event reset key is 0\n");
				ret = -EFAULT;
				goto ops_ioctl_return;
			}

			/* Find event */
			mutex_lock(&event_mutex);
			for(i = 0; i < EVENT_MAX_ALLOC_COUNT; i++)
			{
				if(asysops_event[i].key == key)
					break;

				if(i == EVENT_MAX_ALLOC_COUNT - 1)
				{
					ops_error("Key correspond event not exists\n");
					ret = -EFAULT;
					mutex_unlock(&event_mutex);
					goto ops_ioctl_return;
				}
			}
			asysops_event[i].cond = 0;
			asysops_event[i].mark = 0;
			mutex_unlock(&event_mutex);

			break;

		default:
			ops_error("Unknow asysops ioctl command %d\n", cmd);
			ret = -EFAULT;
			break;
	}

	ops_debug("Normal exit asysops ioctl, %d\n", ret);

ops_ioctl_return:
	return ret;
}

/*
 * asysops_mmap()
 *
 * System call mmap. This mmap can only handle page align start 
 * address memory block.
 */
static int asysops_mmap(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;

	vma->vm_flags |= VM_RESERVED | VM_IO;

	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if(remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, size, vma->vm_page_prot))
	{
		ops_error("Map for physical address error\n");
		return -EAGAIN;
	}

	return 0;
}


/* Shared memory file operation structure */
static struct file_operations asysops_ops = 
{
	.owner		= THIS_MODULE,
	.open		= asysops_open,
	.release	= asysops_release,
	.ioctl		= asysops_ioctl,
	.mmap		= asysops_mmap,
};

/*
 * asysops_init()
 *
 * Driver module init.
 */
static int __init asysops_init(void)
{
	/* Create char device class */
	asysops_class = class_create(THIS_MODULE, ASYSOPS_DEV_CLASS_NAME);
	if(asysops_class == NULL)
	{
		ops_error("Create asysops class error\n");
		return -EFAULT;
	}

	/* Register shared memory char device */
	if(register_chrdev(ASYSOPS_DEV_CLASS_MAJOR, ASYSOPS_DEV_CLASS_NAME, &asysops_ops))
	{
		ops_error("Register asysops char device driver error.\n");
		return -EFAULT;
	}

	/* Create shared memory device node */
	device_create(asysops_class, NULL, \
			MKDEV(ASYSOPS_DEV_CLASS_MAJOR, ASYSOPS_DEV_CLASS_MINOR), \
			NULL, ASYSOPS_DEV_CLASS_NAME);

	/* Initialize global variable structure */
	memset(asysops_shmem, 0x00, sizeof(asysops_shmem));
	memset(asysops_event, 0x00, sizeof(asysops_event));
	memset(asysops_lock, 0x00, sizeof(asysops_lock));

	/* Initialize mutex */
	mutex_init(&common_mutex);
	mutex_init(&shmem_mutex);
	mutex_init(&event_mutex);
	mutex_init(&lock_mutex);

	return 0;
}

/*
 * asysops_exit()
 *
 * Driver module exit.
 */
static void __exit asysops_exit(void)
{
	/* Destroy mutexes */
	mutex_destroy(&common_mutex);
	mutex_destroy(&shmem_mutex);
	mutex_destroy(&event_mutex);
	mutex_destroy(&lock_mutex);

	/* Destroy shared memory device node */
	device_destroy(asysops_class, MKDEV(ASYSOPS_DEV_CLASS_MAJOR, ASYSOPS_DEV_CLASS_MINOR));
	unregister_chrdev(ASYSOPS_DEV_CLASS_MAJOR, ASYSOPS_DEV_CLASS_NAME);
	class_destroy(asysops_class);
}

module_init(asysops_init);
module_exit(asysops_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz of InfoTM");
MODULE_DESCRIPTION("Shared memory, message and lock for Android");
