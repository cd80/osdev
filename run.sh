#!/bin/bash
qemu-system-x86_64 -L . -m 64 -fda Disk.img -M pc -curses -hda HDD.img -boot a
#qemu-system-x86_64 -L . -m 64 -M pc -curses -drive file=fat:rw:fat-type=12:Disk.img,id=fat32,format=raw,if=none
#qemu-system-x86_64 -L . -m 64 -fda ./Disk.img -M pc -curses -enable-kvm

stty sane
