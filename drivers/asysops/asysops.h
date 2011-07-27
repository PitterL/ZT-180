/*
 * asysops.h
 *
 * Head file of operation infotm device driver.
 *
 * InfoTM All Rights Reserved.
 *
 * Sololz <sololz.luo@gmail.com>
 *
 * 2010-06-30
 * 	Create this head file.
 */

#ifndef __ASYSOPS_H__
#define __ASYSOPS_H__

#include <linux/mutex.h>
#include <linux/workqueue.h>

/* MACROS */

#define ASYSOPS_DEV_CLASS_NAME		"asysops"
#define ASYSOPS_DEV_CLASS_MAJOR		110	/* OK, it's perfect */
#define ASYSOPS_DEV_CLASS_MINOR		110	/* OK, it's perfect */

#define ASYSOPS_MAX_INST_COUNT		256	/* Max open count */

#define SHMEM_MAX_ALLOC_COUNT		256	/* Max alloc per instance */
#define SHMEM_MAX_SIZE			(8 * 1024 * 1024)	/* 8MB */
#define EVENT_MAX_ALLOC_COUNT		256
#define LOCK_MAX_ALLOC_COUNT		256

/* MESSAGE PRINT */

/*
 * Debug print message is control by Kconfig macro, error 
 * and information message should always be printed.
 */
#define ops_debug(debug, ...)		printk(KERN_DEBUG "[OPS DEBUG] " debug, ##__VA_ARGS__)
#define ops_error(error, ...)		printk(KERN_ERR "[OPS ERROR] %s line %d " error, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ops_info(info, ...)		printk(KERN_INFO "[OPS INFO] " info, ##__VA_ARGS__)

/* IOCTL COMMANDS */
#define ASYSOPS_CMD_MAGIC		'a'

#define ASYSOPS_CMD_GET_SHMEM		_IO(ASYSOPS_CMD_MAGIC, 1)
#define ASYSOPS_CMD_FREE_SHMEM		_IO(ASYSOPS_CMD_MAGIC, 2)

#define ASYSOPS_CMD_GET_LOCK		_IO(ASYSOPS_CMD_MAGIC, 3)
#define ASYSOPS_CMD_FREE_LOCK		_IO(ASYSOPS_CMD_MAGIC, 4)
#define ASYSOPS_CMD_HOLD_LOCK		_IO(ASYSOPS_CMD_MAGIC, 5)
#define ASYSOPS_CMD_POST_LOCK		_IO(ASYSOPS_CMD_MAGIC, 6)

#define ASYSOPS_CMD_GET_EVENT		_IO(ASYSOPS_CMD_MAGIC, 7)
#define ASYSOPS_CMD_FREE_EVENT		_IO(ASYSOPS_CMD_MAGIC, 8)
#define ASYSOPS_CMD_SEND_EVENT		_IO(ASYSOPS_CMD_MAGIC, 9)
#define ASYSOPS_CMD_WAIT_EVENT		_IO(ASYSOPS_CMD_MAGIC, 10)
#define ASYSOPS_CMD_RESET_EVENT		_IO(ASYSOPS_CMD_MAGIC, 11)

#define ASYSOPS_RET_EXIST		0x2b2b2b2b

/* Shared memory transfer structure */
typedef struct
{
	unsigned int	phys;
	unsigned int	size;
	unsigned int 	key;
}shmem_param_t;

/* Event transfer structure */
typedef struct
{
	unsigned int	phys;
	unsigned int	size;
	unsigned int	key;
	unsigned int	data_phys;
	unsigned int	data_size;
}event_param_t;

/* Lock transfer structure */
typedef struct
{
	unsigned int	phys;
	unsigned int	size;
	unsigned int	key;
}lock_param_t;

/* Shared memory block structure */
typedef struct
{
	unsigned int	phys;		/* Physical address of current shared memory block */
	unsigned int	*virt;		/* Kernel space virtual address */
	unsigned int	size;		/* Size of current shared memory block */
	unsigned int	count;		/* Record depend instance count */
	unsigned int	key;		/* Use this to mark current memory block */
}asysops_shmem_t;

/* Message structure */
typedef struct
{
	unsigned int	count;
	unsigned int	key;
	unsigned int 	mark;
	unsigned int 	cond;
	struct mutex	mutex;
	wait_queue_head_t *event;

	/*
	 * This is a data part of an event, this data is designed to
	 * write before send, read after wait. Using read() and write()
	 * system call to process this memory. Read and write will tell
	 * what to read and write by instance type.
	 */ 
	unsigned int	phys;
	unsigned int	*virt;
	unsigned int	size;

	unsigned int	data_phys;
	unsigned int	*data_virt;
	unsigned int	data_size;
}asysops_event_t;

/* Lock structure */
typedef struct
{
	unsigned int	count;
	unsigned int	key;
	struct mutex	*lock;

	unsigned int	phys;
	unsigned int	*virt;
	unsigned int	size;
}asysops_lock_t;

typedef enum
{
	ASYSOPS_NONE_INST	= 0,
	ASYSOPS_SHMEM_INST	= 1,
	ASYSOPS_EVENT_INST	= 2,
	ASYSOPS_LOCK_INST	= 3,
}asysops_type_t;

/* Asysops instance structure, a instance is desiged to be only one handle */
typedef struct 
{
	asysops_type_t	type;

	unsigned int	shmem_key;	/* Record related shared memory key */
	unsigned int	event_key;	/* Record related message key */
	unsigned int	lock_key;	/* Record related lock key */
}asysops_inst_t;

#endif	/* __ASYSOPS_H__ */
