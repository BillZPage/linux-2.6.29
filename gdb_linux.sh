#!/bin/bash

GDB=arm-none-linux-gnueabi-gdb
#GDB=arm-none-eabi-gdb

$GDB -x target.cmd  -ex "connect_jei localhost 2331"
