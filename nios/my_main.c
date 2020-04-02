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
	printf(" * Device ID 00:\t");
	uint8_t device_id_array_0[8];
	// let us read the device ID
	read_device_id_00(device_id_array_0);
	print_array(device_id_array_0,8);

	printf(" * Device ID 20:\t");
	uint8_t device_id_array[4];
	// let us read the device ID
	read_device_id_20(device_id_array);
	print_array(device_id_array,4);

	printf(" * Detecting Device:\t");
	// let us predict the name of device
	detect_device();

	// let us just read a random page for now
	// .. 5-byte page address
	// .. c1,c2,r1,r2,r3
	uint8_t my_page_address[5] = {0x00,0x00,0x0,0xf7,0x06}; 
	uint8_t my_page_address2[5] = {0x00,0x00,0x01,0xf7,0x06}; 
	uint8_t new_col[2] = {50,0};
	uint8_t data_received[8192];

	printf(" * Printing array value:");
	memset(data_received,0x0,sizeof(data_received));
	print_array(data_received,100);

	printf(" * Reading address (address in reverse order:)");
	print_array(my_page_address,5);
	read_page(my_page_address,5);
	change_read_column(new_col);
	get_data(data_received,100);
	print_array(data_received,100);

	printf(" * Reading address (address in reverse order:)");
	print_array(my_page_address2,5);
	read_page(my_page_address2,5);
	change_read_column(new_col);
	get_data(data_received,100);
	print_array(data_received,100);

	// let us erase the block
	printf(" * Erasing address (address in reverse order:)");
	print_array(my_page_address+2,3);
	enable_erase();
	erase_block(my_page_address+2);
	disable_erase();
	// now read the erased page, should be 0xffs
	read_page(my_page_address,5);
	change_read_column(new_col);
	get_data(data_received,100);
	print_array(data_received,100);

	// let us erase the block
	printf(" * Erasing address (address in reverse order:)");
	print_array(my_page_address2+2,3);
	enable_erase();
	erase_block(my_page_address2+2);
	disable_erase();
	// now read the erased page, should be 0xffs
	read_page(my_page_address2,5);
	change_read_column(new_col);
	get_data(data_received,100);
	print_array(data_received,100);

	// let us clear the buffer
	// should be 00s
	printf("Clearing the buffer: \n");
	memset(data_received,0x0,sizeof(data_received));
	print_array(data_received,100);


	// let us program the page
	printf(" * Programming address (address in reverse order:)");
	print_array(my_page_address,5);
	enable_erase();
	program_page(my_page_address,data_received,8192);
	disable_erase();
	read_page(my_page_address,5);
	change_read_column(new_col);
	get_data(data_received,100);
	print_array(data_received,100);

	printf(" * Programming address (address in reverse order:)");
	print_array(my_page_address2,5);
	enable_erase();
	program_page(my_page_address2,data_received,8192);
	disable_erase();
	read_page(my_page_address2,5);
	change_read_column(new_col);
	get_data(data_received,100);
	print_array(data_received,100);

	return 0;
}