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
	// the first thing to do is to make start-up the NAND set
	device_initialization();

	// let us wait on key press to start any operation
	volatile uint32_t* push_button = PUSH_KEY_LOCATION;
	while((*push_button&0xf)==0);

#if DEBUG
	printf("Starting the NAND interface program..\n");
	fflush(stdout);
#endif
	printf("Device ID 00:\t");
	uint8_t device_id_array_0[8];
	// let us read the device ID
	read_device_id_00(device_id_array_0);
	print_array(device_id_array_0,8);

	printf("Device ID 20:\t");
	uint8_t device_id_array[4];
	// let us read the device ID
	read_device_id_20(device_id_array);
	print_array(device_id_array,4);

	printf("Detecting Device:\t");
	// let us predict the name of device
	detect_device();

	printf("Unique Device ID:\t");
	uint8_t unique_device_id_array[16];
	read_unique_id(unique_device_id_array, 16);
	print_array(unique_device_id_array,16);

	// let us just read a random page for now
	// .. 5-byte page address
	// .. c1,c2,r1,r2,r3
	// uint8_t my_page_address[5] = {}; 

	return 0;
}