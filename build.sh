#!/bin/bash

riscv64-unknown-elf-gcc -Wall -march=rv64g --std=gnu99 -msmall-data-limit=0 -c main.c
riscv64-unknown-elf-gcc -Wall -march=rv64g --std=gnu99 -msmall-data-limit=0 -c printf.c
riscv64-unknown-elf-gcc -Wall -march=rv64g --std=gnu99 -msmall-data-limit=0 -c start.S
riscv64-unknown-elf-gcc -Wall -march=rv64g -ffreestanding -nostdlib -msmall-data-limit=0 -T linker.ld start.o main.o printf.o
riscv64-unknown-elf-objcopy -O binary a.out out.bin
