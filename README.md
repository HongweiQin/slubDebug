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


