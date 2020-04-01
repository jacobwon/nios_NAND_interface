#include "nand_interface_header.h"

// put the user defined header codes here
// .. all the operations here are asynchronous
// .. we are using TSOP NAND flash which only supports asynchronous interface

volatile uint32_t* jumper_address =  JUMPER_LOCATION;
volatile uint32_t* jumper_direction =  JUMPER_DIRECTION;
volatile uint32_t* push_button = PUSH_KEY_LOCATION;

void check_status()
{	
	send_command(0x70);
	uint8_t status;
	get_data(&status,1);

	if(status&0x01)
	{
		printf("Last Operation failed\n");
	}
}

// function to send an arbitrary command signal to the NAND device
// .. the procedure is as follows ( in the sequence )
void send_command(uint8_t command_to_send)
{
	// set the required pins as output from the angle of NIOS machine
	*jumper_direction |= 0x03fff;
	*jumper_direction &= ~0x4000;

	// let us first reset the DQ pints
	*jumper_address &= ~(DQ_mask);

	// .. Write Enable should go low WE => low
	// .. reset the bit that is connected to WE
	*jumper_address &= ~(WE_mask);
	// .. .. Chip Enable should go low CE => low
	*jumper_address &= ~(CE_mask);
	// .. .. ALE should go low ALE => low
	*jumper_address &= ~(ALE_mask);
	// .. RE goes high
	*jumper_address |= RE_mask;
	// .. .. CLE should go high CLE => high
	*jumper_address |= (CLE_mask);

	// .. .. send the command signal in the DQ pins	
	// .. .. the idea is clear the least 8-bits
	// .. .. copy the values to be sent
	*jumper_address = (*jumper_address&(~DQ_mask))|(command_to_send & DQ_mask);

	//insert delay here
	// .. tDS = 40 ns
	SAMPLE_TIME;

	// disable write enable again
	*jumper_address |= (WE_mask);

	//insert delay here
	// .. because the command is written on the rising edge of WE
	// tDH = 20 ns
	HOLD_TIME;

	// disable CLE
	*jumper_address &= ~(CLE_mask);
	
	
	// reset all the data on DQ pins
	*jumper_address &= ~(DQ_mask);
}

// function to send address to the NAND device 
// .. the procedure is as follows (in the sequence)
void send_addresses(uint8_t* address_to_send, uint8_t num_address_bytes)
{
	// set the required pins as output
	*jumper_direction |= 0x03fff;
	*jumper_direction &= ~0x4000;

	// .. .. CE goes low
	*jumper_address &= ~CE_mask;
	// .. CLE goes low
	*jumper_address &= ~CLE_mask;
	// .. ALE goes high
	*jumper_address |= ALE_mask;
	// .. RE goes high
	*jumper_address |= RE_mask;
	
	for(uint8_t i=0;i<num_address_bytes;i++)
	{
		//insert delay here
		HOLD_TIME;	// tDH

		*jumper_address &= ~(WE_mask);

		// .. Put data on the DQ pin
		// .. .. the idea is clear the least 8-bits
		// .. .. copy the values to be sent
		*jumper_address = (*jumper_address&(~DQ_mask))|(address_to_send[i] & DQ_mask);
		//.. a simple delay
		SAMPLE_TIME; //tDS

		// .. Address is loaded from DQ on rising edge of WE
		*jumper_address |= WE_mask;
		// .. maintain WE high for certain duration and make it low
		
		// .. put next address bits on DQ and cause rising edge of WE
		// .. address expected is 5-bytes ColAdd1, ColAdd2, RowAdd1, RowAdd2, RowAdd3
	}
	// .. ALE goes low
	*jumper_address &=  ~(ALE_mask);
}

// function to send address to the NAND device 
// .. the procedure is as follows (in the sequence)
void send_address(uint8_t address_to_send)
{
	// set the required pins as output
	*jumper_direction |= 0x03fff;
	*jumper_direction &= ~0x4000;

	// .. .. CE goes low
	*jumper_address &= ~CE_mask;
	// .. CLE goes low
	*jumper_address &= ~CLE_mask;
	// .. ALE goes high
	*jumper_address |= ALE_mask;
	// .. RE goes high
	*jumper_address |= RE_mask;
	
	for(uint8_t i=0;i<1;i++)
	{
		//insert delay here
		HOLD_TIME;	// tDH

		*jumper_address &= ~(WE_mask);

		// .. Put data on the DQ pin
		// .. .. the idea is clear the least 8-bits
		// .. .. copy the values to be sent
		*jumper_address = (*jumper_address&(~DQ_mask))|(address_to_send & DQ_mask);
		//.. a simple delay
		SAMPLE_TIME; //tDS

		// .. Address is loaded from DQ on rising edge of WE
		*jumper_address |= WE_mask;
		// .. maintain WE high for certain duration and make it low
		
		// .. put next address bits on DQ and cause rising edge of WE
		// .. address expected is 5-bytes ColAdd1, ColAdd2, RowAdd1, RowAdd2, RowAdd3
	}
	// .. ALE goes low
	*jumper_address &=  ~(ALE_mask);
}

// function to send address from the host machine to the NAND flash
// .. Data is written from DQ[7:0] to the cache register of the selected die (LUN)
// .. .. on the rising edge of WE# when CE# is LOW, ALE is LOW, CLE is LOW, and RE# is HIGH
void send_data(uint8_t* data_to_send,uint16_t num_data)
{
	// .. CE should be low
	*jumper_address &= ~CE_mask;
	// .. CLE should be low
	*jumper_address &= ~CLE_mask;
	// .. ALE should be low
	*jumper_address &= ~ALE_mask;
	// .. RE should be high
	*jumper_address |= RE_mask;

	for(uint16_t i=0;i<num_data;i++)
	{
		//insert delay here
		HOLD_TIME;	//tDH

		// .. make WE low and repeat the procedure again for number of bytes required (int num_data)
		*jumper_address &= ~WE_mask;

		// .. put data on DQ and latch WE high for certain duration
		// .. .. the idea is clear the least 8-bits
		// .. .. copy the values to be sent
		*jumper_address = (*jumper_address&(~DQ_mask))|(data_to_send[i] & DQ_mask);
		//.. a simple delay
		SAMPLE_TIME;	// tDS

		*jumper_address |= WE_mask;
		
	}
}

// function to receive data from the NAND device
// .. data is output from the cache regsiter of selected die
// .. it is supported following a read operation of NAND array
void get_data(uint8_t* data_received,uint16_t num_data)
{
	*jumper_direction &= ~0x40ff;

	// .. data can be received when on ready state (RDY signal)
	// .. ensure RDY is high
	// .. .. just keep spinning here checking for ready signal
	while((*jumper_address & RB_mask)== 0x00);

	// .. data can be received following READ operation
	// .. the procedure should be as follows
	// .. .. CE should be low
	*jumper_address &= ~CE_mask;
	// .. make WE high
	*jumper_address |= WE_mask;
	// .. .. ALE and CLE should be low
	*jumper_address &= ~ALE_mask;
	*jumper_address &= ~CLE_mask;

	for(uint16_t i=0;i<num_data;i++)
	{			
		// set the RE to high for next cycle
		*jumper_address &= ~RE_mask;

		// tRP = 50ns
		SAMPLE_TIME;
		asm("nop");

		// .. data is available at DQ pins on the falling edge of RE pin (RE is also input to NAND)
		*jumper_address |= RE_mask;
		
		//insert delay here
		// .. tREH = 30 ns
		asm("nop");asm("nop");

		// read the data
		data_received[i] = *jumper_address & DQ_mask;
	}

	// set the pins as output
	*jumper_direction |= 0x03fff;	
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
	for(uint8_t i=0;i<4;i++);
}

void write_enable()
{
	// check to see if the device is busy
	// .. wait if busy
	while((*jumper_address&RB_mask)==0x00);

	// wp to high
	*jumper_address |= WP_mask;
	
	//insert delay here
	tWW;
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
	*jumper_direction &= ~0x4000;
	*jumper_direction |= 0x3fff;

	//insert delay here
	for(uint16_t i=0;i<5000;i++);	//50 us max

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
	// .. not needed because the next polling statement will take care
	// .. polling the Ready/BUSY signal
	// .. but we should wait for tWB = 200ns before the RB signal is valid
	tWB;	// tWB = 200ns

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
	tWB;//tWB
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
	// .. tWHR = 120ns
	tWHR;

	get_data(device_id_array,8);
}

// function that reads the device ID and tries to detect the device name
// .. call the function device_id at address 00h
// .. lookup table based finding for device name
// .. this function is not decisive
void detect_decive()
{
	// create a 8-byte variable
	// .. static array
	uint8_t my_device_id[8];

	// following call should return the device ID to the array
	read_device_id_00(my_device_id);
	char device_name[20] = "?";
	// now check each bytes
	if(my_device_id[0]==0x2c)
	{
		if(my_device_id[1]==0x88)
		{
			if(my_device_id[2]==0x04)
			{
				if(my_device_id[3]==0x4b)
				{					
					if(my_device_id[4]==0xa9)
					{
						strcpy(device_name,"MT29FxxxG08Cxxxx\0");	
					}
				}
			}
		}else if(my_device_id[1]==0xA8)
		{
			if(my_device_id[2]==0x05)
			{
				if(my_device_id[3]==0x5b)
				{
					if(my_device_id[4]==0xa9)
					{
						strcpy(device_name,"MT29FxxxG08Cxxxx\0");	
					}					
				}				
			}
		}
	}

	if(device_name[0]=='?')
	{
		printf("Device ID not detected\n");
	}else
	{
		printf("Detected Device ID is %s\n", my_device_id);
	}
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
	
	// .. tWHR = 120ns
	tWHR;

	get_data(device_id_array,4);
#if DEBUG
	printf("Before reading from the device\n");
	print_array(device_id_array,4);
#endif
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
	// ..tWB = 200ns
	tWB;

	// make sure none of the LUNs are busy
	while((*jumper_address & RB_mask)==0);

	uint8_t* data_temp = (uint8_t*)malloc(32*sizeof(uint8_t));

#if DEBUG
	printf("Before reading from the device\n");
	print_array(data_temp,32);
#endif

	get_data(data_temp,32);


#if DEBUG
	printf("After reading from the device\n");
	print_array(data_temp,32);
#endif

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
	// .. tWHR= 120ns
	tWHR;

	get_data(status_value,1);
}

// use this for multi-LUN device to avoid bus contention
// .. r1, r2 and r3 are the three-bytes for the row addresss
void read_status_enhanced(uint8_t* status_value, uint8_t* r1r2r3)
{
	send_command(0x78);
	send_addresses(r1r2r3,3);
	
	//insert delay here
	//insert delay here
	// .. tWHR= 120ns
	tWHR;

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

// follow the following function call by get_data() function call
// .. please change this if the device has multiple dies
void change_read_column(uint8_t* col_address)
{
	tRHW;	// tRHW = 200ns

	send_command(0x05);
	send_addresses(col_address,2);

	send_command(0xe0);

	tCCS;	//tCCS = 200ns
}

// follow the following function call by get_data() function call
void change_read_column_enhanced(uint8_t* address)
{
	tRHW;	// tRHW = 200ns

	send_command(0x06);
	send_addresses(address,5);

	send_command(0xe0);

	tCCS;	//tCCS = 200ns
}

// following function call should be followed by send_data() function calls
// .. this just changes the address in the selected cache register
void change_write_column(uint8_t* col_address)
{
	send_command(0x85);
	send_addresses(col_address,2);

	tCCS;	//tCCS = 200ns
}


// change teh row address where the cache register contents will be programmed in NAND flash
// .. row address means block and page address
// .. data input is optional after the address cycles
// .. .. data input begins at the column address specified
void change_row_address(uint8_t* address)
{
	send_command(0x85);

	send_addresses(address,5);

	// .. wait before inputting the data
	tCCS;	//tCCS = 200ns
}

// write a function to perform an read operation from NAND flash to cache register
// .. reads one page from the NAND to the cache register
// .. during the read, you can use change_read_column and change_row_address
void read_page(uint8_t* address,uint8_t address_length)
{
	// make sure none of the LUNs are busy
	while((*jumper_address & RB_mask)==0);

	send_command(0x00);
	send_addresses(address,address_length);
	send_command(0x30);

	// just a delay
	tWB;
	// check for RDY signal
	while((*jumper_address & RB_mask)==0);
	// tRR = 40ns
	tRR;
}

// following is the faster read operation
// .. essentially calls the read_page function above
// .. and reads the output data from the cache and at the same time copies next page to the data regsiter
void read_page_cache_sequential(uint8_t* address, uint8_t address_length,uint8_t* data_read,uint16_t* data_read_len,uint16_t num_pages)
{
	// make sure none of the LUNs are busy
	while((*jumper_address & RB_mask)==0);

	send_command(0x00);
	send_addresses(address,address_length);
	send_command(0x30);

	// just a delay
	tWB;

	// check if it is out of Busy cycle
	while((*jumper_address & RB_mask)==0);
	// lets wait again
	tRR;

	for(uint16_t page_num = 0;page_num<num_pages-1;page_num++)
	{
		send_command(0x31);
		// just a delay
		tWB;

		// check if it is out of Busy cycle
		while((*jumper_address & RB_mask)==0);
		// lets wait again
		tRR;

		*data_read_len = 8192;
		get_data(data_read+(page_num*(*data_read_len)),*data_read_len);
	}	

	// read page cache last
	send_command(0x3f);
	// just a delay
	tWB;

	// check if it is out of Busy cycle
	while((*jumper_address & RB_mask)==0);
	// lets wait again
	tRR;

	*data_read_len = 8192;
	get_data(data_read+((num_pages-1)*(*data_read_len)),*data_read_len);
}

// enables data output for the last selected die and cache register
// .. after a READ operation has been monitored
void read_mode()
{
	send_command(0x00);
}


// program operation is used to move data from cache or data regsiter to NAND array
// .. during program operation, the contents of cache or data registers are modified by internal control logic
// .. pages in a block should be programmed from the least significant page to the most sign addres
// .. programming pages out of order in a block is not allowed
// .. address: should be 5 bytes and data should be 
void program_page(uint8_t* address,uint8_t* data,uint16_t num_data)
{
	// 
	send_command(0x80);
	send_addresses(address,5);

	// tADL
	tADL;

	send_data(data,num_data);
	send_command(0x10);

	tWB;

	// check if it is out of Busy cycle
	while((*jumper_address & RB_mask)==0);

	uint8_t status_value;
	// .. use  the commended code for multi-plane die
	// read_status_enhanced(&status_value,(address+2));
	read_status(&status_value);
	if(status_value&0x01)
	{
		printf("Failed Program Operation\n");
	}else
	{
#if DEBUG
printf("Program Operation Successful\n");
#endif
	}
}

// function used to program multiple pages
// the data is copied to the cache, and then to the NAND memory
// .. there is no need to wait for the previous operation to complete
// .. and can move on with next program operation of page
// address is an array with multiple of 5, each representing the address of each page
// data is same programmed to each page
// num_data is the number of bytes of data in data array
void program_page_cache(uint8_t* address,uint8_t* data,uint16_t num_data,uint8_t num_pages)
{
	for(uint8_t page_num = 0;page_num<num_pages-1;page_num++)	
	{
		send_command(0x80);
		send_addresses(address+(5*page_num),5);

		// tADL
		tADL;

		send_data(data,num_data);
		send_command(0x15);

		tWB;

		// check if it is out of Busy cycle
		while((*jumper_address & RB_mask)==0);
	}
	send_command(0x80);
	send_addresses((address+5*(num_pages-1)),5);

	// tADL
	tADL;

	send_data(data,num_data);
	send_command(0x10);

	tWB;

	// check if it is out of Busy cycle
	while((*jumper_address & RB_mask)==0);

	uint8_t status_value;
	// .. use  the commended code for multi-plane die
	// read_status_enhanced(&status_value,(address+2));
	read_status(&status_value);
	if(status_value&0x01)
	{
		printf("Failed Program Operation\n");
	}else
	{
#if DEBUG
printf("Program Operation Successful\n");
#endif
	}
}

void erase_block(uint8_t* row_address)
{	
	// check if it is out of Busy cycle
	while((*jumper_address & RB_mask)==0);

	send_command(0x60);
	send_addresses(row_address,3);
	send_command(0xd0);

	tWB;
	
	// check if it is out of Busy cycle
	while((*jumper_address & RB_mask)==0);

	uint8_t status;
	read_status(&status);	
	if(status&0x01)
	{
		printf("Failed Erase Operation\n");
	}else
	{
#if DEBUG
printf("Erase Operation Successful\n");
#endif
	}
}
