# slubDebug
A kernel module for learning slub debugging.

WARNING: THIS PROGRAM MAY DAMAGE YOUR COMPUTER. DO NOT USE UNLESS YOU KNOW WHAT YOU ARE DOING!


## Build

    make

## Load

    ./install.sh

## Remove

    ./remove.sh

## Run Commands

    ./cmd.sh <xxx> <xxx> <xxx> ...

## Sample Commands

### Allocate a kmem cache

Allocate a kmem cache whose object size is 8 Bytes

    ./cmd.sh n 8

`dmesg` will show the slot number of the new allocated slab cache.

Users can also specify the slab cache flags. E.g.,

    ./cmd.sh n fzptu 8

See linux kernel source mm/slub.c for more information.

### Destroy a kmem cache

Destroy a kmem cache in slot 0

    ./cmd.sh d 0

### Allocate mem from a slab cache

Allocate from slot 0

    ./cmd.sh a 0

`dmesg` shows the allocated address.

### Free mem to a slab cache

Free 0xaabb to slot 0

    ./cmd.sh f 0 aabb

### Write by pointer

Write @data to @pointer as @size in bits

    ./cmd.sh w aabb 16 1

This command will write 1 as a 16bit integer to 0xaabb

### Write by memset

Write #data to @pointer as @size in Bytes

    ./cmd.sh m aabb 8 1

This command will memset(aabb, 1, 8)

### Read

    ./cmd.sh r aabb 64

This command will read a 64bit integer from 0xaabb


