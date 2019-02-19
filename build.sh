#!/bin/bash

#prefix=arm-linux-gnueabi-
prefix=arm-none-eabi-
arch=arm
config=jz2440_defconfig

if [ $1 == "clean" ]
then
	make CROSS_COMPILE=$prefix ARCH=$arch distclean
elif [ $1 == "config" ]
then
	make CROSS_COMPILE=$prefix ARCH=$arch $config
elif [ $1 == "menu" ]
then
	make CROSS_COMPILE=$prefix ARCH=$arch menuconfig
elif [ $1 == "dissemble" ]
then
	${prefix}objdump -d vmlinux > vmlinux.dis
else
	make CROSS_COMPILE=$prefix ARCH=$arch uImage
fi


