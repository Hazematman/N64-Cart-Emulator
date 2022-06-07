# N64 Cartridge Bus Emulator
This is an attempt to emulate the N64 cartridge bus using a fast modern SBC.
This project currently uses the "Allwinner Nezha" development board but will switch
to the "Sispeed Lichee RV" in the future (assuming everything works) as the Sispeed board is much cheaper
($30 vs $100) and it exposes enough IO on the board without needing soldering.

# How it works
This project works by booting a baremetal program on the board that using interrupts to respond to the
ALE_L, ALE_H, and /READ signals from the N64 bus, it then loads the appropriate data directly from RAM on
the SBC and makes it available on the address bus.

# How to use it
The makefile assumes that you have a toolchain called `riscv64-unknown-elf` in your path 
(I.E. `riscv64-unknown-elf-gcc` and friends exist in your path). Assuming you do you should be able to just
run `make` to have it generate a binary file called `n64_cart.bin` that represents the program. This program
assumes that it will be loaded into memory on the board at address `0x45000000`. To load the program on hardware
I am currently using uboot with the `loadx`  command to load the binary over serial. Using the `screen` program
as your serial terminal you can do the following to load the program in uboot
1. run `screen` pointed to the right `ttyUSB` device
2. once uboot starts up, press any button to get a terminal in uboot
3. run `loadx 45000000` in uboot
4. in screen preform the key combination of `CTRL-A + :` and then type in `exec !! sx out.bin`. This
   will output the binary file in the `XMODEM` format for uboot to load.
5. Once the program is done loading run `go  45000000` to run the application

# TODO
Currently this only implements the framework to get this idea working. There is still a lot of work to do to
test if the Allwinner D1 chip is fast enough to emulate the N64 cartridge bus.

