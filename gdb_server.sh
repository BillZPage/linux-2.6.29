#!/bin/bash

JLinkGDBServer -CPU ARM7_9 -device S3C2440A -endian little -if JTAG -speed auto
