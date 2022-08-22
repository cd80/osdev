#!/bin/bash
qemu-system-x86_64 -L . -m 4G -fda Disk.img -M pc -curses
#qemu-system-x86_64 -L . -m 64 -M pc -curses -drive file=fat:rw:fat-type=12:Disk.img,id=fat32,format=raw,if=none
#qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -M pc -curses -enable-kvm

stty sane
