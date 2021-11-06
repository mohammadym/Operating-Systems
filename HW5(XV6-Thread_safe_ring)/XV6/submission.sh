#!/bin/sh

a=$(ls)


tar -cvf "../xv6.tar" --exclude=*.o --exclude=*.d --exclude=*.img --exclude=*.asm --exclude=kernel --exclude=_* $a
