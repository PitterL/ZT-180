#!/bin/bash

#
# Build U-Boot image when `mkimage' tool is available.
#

MKIMAGE=$srctree/scripts/mkimage


# Call "mkimage" to create U-Boot image
${MKIMAGE} "$@"
