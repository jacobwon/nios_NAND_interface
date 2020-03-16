/*
File: my_main.c
Description: This file is created inside the project 'nios_nand_interface'
.. The goal of this project is to create a computer system that interfaces with a ONFI2.2 standard
.. .. memory system
.. The computer system used here is DE1_SoC that runs at 100Mhz (10 ns period)
.. connection to the NAND is made in parallel port 1 on JP1
.. .. the address of which is at 0xff200060

Author: Prawar Poudel (pp0030@uah.edu)
		4 March 2020
*/


// user defined header file
#include "nand_interface_header.h"

int main()
{
#if DEBUG
	printf("Starting the NAND interface program..\n");
	fflush(stdout);
#endif

	// let us wait on key press to start any operation
	volatile uint32_t* push_button = PUSH_KEY_LOCATION;
	while((*push_button&0xf)==0);

	// the first thing to do is to make start-up the NAND set
	device_initialization();

	uint8_t my_device_id[4];
	enable_program();
	read_device_id_20(my_device_id);
	print_array(my_device_id,4);

	read_status(my_device_id);
	print_array(my_device_id,1);

	return 0;
}