#include "nand_interface_header.h"

// put the user defined header codes here
// .. all the operations here are asynchronous
// .. we are using TSOP NAND flash which only supports asynchronous interface

volatile uint32_t* jumper_address =  JUMPER_LOCATION;

// function to send an arbitrary command signal to the NAND device
// .. the procedure is as follows ( in the sequence )
void send_command(uint8_t command_to_send)
{
	// let us first reset the DQ pints
	*jumper_address &= ~(DQ_mask);

	// .. Write Enable should go low WE => low
	// .. reset the bit that is connected to WE
	*jumper_address &= ~(WE_mask);
	// .. .. Chip Enable should go low CE => low
	*jumper_address &= ~(CE_mask);
	// .. .. ALE should go low ALE => low
	*jumper_address &= ~(ALE_mask);
	// .. .. CLE should go high CLE => high
	*jumper_address |= (CLE_mask);

	// .. .. send the command signal in the DQ pins	
	// .. .. .. set the pins to be set
	*jumper_address |= (command_to_send & DQ_mask);
	// .. .. .. reset the pins to be reset
	*jumper_address &= (command_to_send & DQ_mask);

	//insert delay here
	for(uint8_t i=0;i<2;i++);

	// disable write enable again
	*jumper_address |= (WE_mask);
	// disable CLE
	*jumper_address |= (CLE_mask);
	// disable CE again
	*jumper_address |= (CE_mask);
	// reset all the data on DQ pins
	*jumper_address &= ~(DQ_mask);
}

// function to send address to the NAND device 
// .. the procedure is as follows (in the sequence)
void send_addresses(uint8_t* address_to_send, uint8_t num_address_bytes)
{
	// .. .. CE goes low
	*jumper_address &= ~CE_mask;
	// .. CLE goes low
	*jumper_address &= ~CLE_mask;
	// .. ALE goes high
	*jumper_address |= ALE_mask;
	
	// .. set the WE to low to create a rising edge later on
	*jumper_address &= ~WE_mask;

	for(uint8_t i=0;i<num_address_bytes;i++)
	{
		// .. Put data on the DQ pin
		// .. .. set the pins to be set
		*jumper_address |= (address_to_send[i] & DQ_mask);
		// .. .. reset the pins to be reset
		*jumper_address &= (address_to_send[i] & DQ_mask);

		// .. Address is loaded from DQ on rising edge of WE
		*jumper_address |= WE_mask;
		// .. maintain WE high for certain duration and make it low
		
		//insert delay here
		for(uint8_t i=0;i<2;i++);

		*jumper_address &= ~WE_mask;

		// .. put next address bits on DQ and cause rising edge of WE
		// .. address expected is 5-bytes ColAdd1, ColAdd2, RowAdd1, RowAdd2, RowAdd3
	}
}

void send_address(uint8_t address_to_send)
{
	// .. .. CE goes low
	*jumper_address &= ~CE_mask;
	// .. CLE goes low
	*jumper_address &= ~CLE_mask;
	// .. ALE goes high
	*jumper_address |= ALE_mask;
	
	// .. set the WE to low to create a rising edge later on
	*jumper_address &= ~WE_mask;

	for(uint8_t i=0;i<1;i++)
	{
		// .. Put data on the DQ pin
		// .. .. set the pins to be set
		*jumper_address |= (address_to_send & DQ_mask);
		// .. .. reset the pins to be reset
		*jumper_address &= (address_to_send & DQ_mask);

		// .. Address is loaded from DQ on rising edge of WE
		*jumper_address |= WE_mask;
		// .. maintain WE high for certain duration and make it low
		
		//insert delay here
		for(uint8_t i=0;i<2;i++);

		*jumper_address &= ~WE_mask;

		// .. put next address bits on DQ and cause rising edge of WE
		// .. address expected is 5-bytes ColAdd1, ColAdd2, RowAdd1, RowAdd2, RowAdd3
	}
}
// function to send address from the host machine to the NAND flash
// .. Data is written from DQ[7:0] to the cache register of the selected die (LUN)
// .. .. on the rising edge of WE# when CE# is LOW, ALE is LOW, CLE is LOW, and RE# is HIGH
void send_data(uint8_t* data_to_send,uint8_t num_data)
{
	// .. CLE should be low
	*jumper_address &= ~CLE_mask;
	// .. ALE should be low
	*jumper_address &= ~ALE_mask;
	// .. CE should be low
	*jumper_address &= ~CE_mask;
	// .. WE should be low
	*jumper_address &= ~WE_mask;

	for(uint8_t i=0;i<num_data;i++)
	{
		// .. put data on DQ and latch WE high for certain duration
		// .. .. set the pins to be set
		*jumper_address |= 	(data_to_send[i] & DQ_mask);
		// .. .. reset the pins to be reset
		*jumper_address &= 	(data_to_send[i] & DQ_mask);

		*jumper_address |= WE_mask;
		
		//insert delay here
		for(uint8_t i=0;i<2;i++);

		// .. make WE low and repeat the procedure again for number of bytes required (int num_data)
		*jumper_address &= ~WE_mask;
	}
}

// function to receive data from the NAND device
void get_data(uint8_t* data_received,uint8_t num_data)
{
	// .. data can be received when on ready state (RDY signal)
	// .. data can be received following READ operation
	// .. the procedure should be as follows
	// .. .. CE should be low
	*jumper_address &= ~CE_mask;
	// .. .. ALE and CLE should be low
	*jumper_address &= ~ALE_mask;
	*jumper_address &= ~CLE_mask;

	// .. make WE high
	*jumper_address |= WE_mask;

	// .. ensure RDY is high
	// .. .. just keep spinning here checking for ready signal
	while((*jumper_address & RB_mask)== 0x00);

	for(uint8_t i=0;i<num_data;i++)
	{			
			
		//insert delay here
		for(uint8_t i=0;i<2;i++);

		// .. data is available at DQ pins on the falling edge of RE pin (RE is also input to NAND)
		*jumper_address &= ~RE_mask;
		
		//insert delay here
		for(uint8_t i=0;i<2;i++);

		// read the data
		data_received[i] = *jumper_address & DQ_mask;
		// set the RE to high for next cycle
		*jumper_address |= RE_mask;
	}	
}

// function to disable Program and Erase operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void write_protect()
{
	// check to see if the device is busy
	// .. wait if busy
	while((*jumper_address&RB_mask)==0x00);

	// wp to low
	*jumper_address &= ~WP_mask;	
	
	//insert delay here
	for(uint8_t i=0;i<2;i++);
}

void write_enable()
{
	// check to see if the device is busy
	// .. wait if busy
	while((*jumper_address&RB_mask)==0x00);

	// wp to high
	*jumper_address |= WP_mask;
	
	//insert delay here
	for(uint8_t i=0;i<2;i++);
}

// function to disable Program operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void disable_program()
{
	write_protect();
}

void enable_program()
{
	write_enable();
}

// function to disable erase operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void disable_erase()
{
	write_protect();
}
void enable_erase()
{
	write_enable();
}

// function to initialize the NAND device
// .. following are the tasks to be performed for initialization of NAND device
// .. provide Vcc (ie ramp Vcc)
// .. host must wait for R/B to be valid (R/B should be low for certain duration and be high
// .. .. to indicate device is ready) R/B is output from the NAND device
// .. issue 0xFF command after R/B goes high (Reset is the first command to be issued)
// .. R/B should be monotired again after issuing 0XFF command
void device_initialization()
{
	//insert delay here
	for(uint8_t i=0;i<2;i++);	//10 us max
	// wait for R/B signal to go high
	while((*jumper_address & RB_mask)==0);
	// now issue RESET command
	reset_device();
}

// function to reset the whole device
// .. issue a command 0xFF for reset operation
// .. following operations should be performed for this
// .. .. enable command cycle
// .. .. command to be sent is 0xFF
// .. .. check for R/B signal to be high after certain duration (should go low(busy) and go high (ready))
void reset_device()
{
	// oxff is reset command
	send_command(0xff);
	// no address is expected for reset command
	// wait for busy signal again
	
	//insert delay here
	for(uint8_t i=0;i<2;i++);	// tPOR

	while((*jumper_address & RB_mask)==0);
}

// following function will reset just a particular LUN
// .. a LUN or logical unit is also called a NAND flash die (our NAND has 2 planes in a die)
// .. can be used to put the LUN into a known state or abort commadn sequences going on
// .. .. all the cache contents are invalid
// .. .. if any operation is going on, partial operations might take place
// .. .. the command issuing has to be followed by sequence of (3-byte) address of the LUN
void reset_LUN(uint8_t* address_LUN, uint8_t num_address_bytes)
{
	// 0xfa is the reset LUN command
	send_command(0xfa);
	// send the address of the LUN to reset
	send_addresses(address_LUN,num_address_bytes);
	
	//insert delay here
	for(uint8_t i=0;i<2;i++);//tWB
	while((*jumper_address & RB_mask)==0);	
}

// function to read the device ID
// when read from address 00h, it returns 8-byte ID of which first 5-bytes are manufacturer ID
// follow the following sequences
// .. set command cycle
// .. .. send 90h as command
// .. set address sending mode
// .. ..send 00 as address
// .. wait for tWHR duration
// .. read 8-bytes from the DQ pins
void read_device_id_00(uint8_t* device_id_array)
{
	// make sure none of the LUNs are busy
	while((*jumper_address & RB_mask)==0);
	// read ID command
	send_command(0x90);
	// send address 00
	send_address(0x00);
	// wait for tWHR duration
	
	//insert delay here
	for(uint8_t i=0;i<2;i++);

	get_data(device_id_array,8);
}

// function to read the 4-byte ONFI code
// when read from address 00h, it returns 4-byte ONFI code
// follow the following sequences
// .. set command cycle
// .. .. send 90h as command
// .. set address sending mode
// .. ..send 20 as address
// .. wait for tWHR duration
// .. read 4-bytes from the DQ pins
void read_device_id_20(uint8_t* device_id_array)
{
	// make sure none of the LUNs are busy
	while((*jumper_address & RB_mask)==0);
	// read ID command
	send_command(0x90);
	// send address 00
	send_address(0x20);
	// wait for tWHR duration
	
	//insert delay here
	// 1 for loop is ~10cc
	// .. 1cc is 10ns
	// .. 10 cc is 100 ns
	for(uint8_t i=0;i<4;i++);

	get_data(device_id_array,4);
}

// function to read the unique identifier programmed into the target
// .. only accepted when device is not busy
// .. start a command cycle with 0x70 as command
// .. start a address cycle of 0x00 as address
// .. target is busy for tR , can be tested using R/B# signal begin high to indicate if the device is ready
// .. one data byte is output per RE# toggle at DQ pin
// .. .. 32 bytes of data is received
// .. .. .. first 16 bytes is the unique ID and next 16-bytes is complement of the data
// .. .. .. XOR should be done to ensure correctness
void read_unique_id(uint8_t* device_id_array, uint8_t num_data)
{
	// make sure none of the LUNs are busy
	while((*jumper_address & RB_mask)==0);

	// command for read unique ID
	send_command(0xed);	
	// address, lets send 00
	send_address(0x00);

	
	//insert delay here
	for(uint8_t i=0;i<2;i++);// tWB+tR+tRR

	uint8_t* data_temp = (uint8_t*)malloc(32*sizeof(uint8_t));
	get_data(data_temp,32);

	// now check the validity of the data
	for(uint8_t i=0;i<16;i++)
	{
		if((data_temp[i]^data_temp[16+i]) != 0xff)
		{
			printf("Error in reading the unique device ID\n");
			break;
		}
	}
	for(uint8_t i=0;i<num_data;i++)
	{
		device_id_array[i] = data_temp[i];
	}
	free(data_temp);
}

// following function can be used to read the status following any command
void read_status(uint8_t* status_value)
{
	send_command(0x70);	
	//insert delay here
	for(uint8_t i=0;i<2;i++);	// tWHR
	get_data(status_value,1);
}

// just a normal function to print an array to terminal
void print_array(uint8_t* my_array, uint8_t len)
{
	for(uint8_t i=0;i<len;i++)
	{
		printf("0x%x ", my_array[i]);
	}
	printf("\n");
}

// write a function to perform an erase operation
