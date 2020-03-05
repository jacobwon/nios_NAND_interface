#include <nand_interface_header.h>

// put the user defined header codes here
// .. all the operations here are asynchronous
// .. we are using TSOP NAND flash which only supports asynchronous interface

uint32_t* jumper_address =  JUMPER_LOCATION;

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
	*jumper_address |= (command_to_send & DQ_mask);

	__delay_cycles(4);
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
// .. .. CE goes low
// .. .. CLE goes low
// .. .. ALE goes high
// .. .. Put data on the DQ pin
// .. .. Address is loaded from DQ on rising edge of WE
// .. .. maintain WE high for certain duration and make it low
// .. .. put next address bits on DQ and cause rising edge of WE
// .. .. address expected is 5-bytes ColAdd1, ColAdd2, RowAdd1, RowAdd2, RowAdd3
void send_address(uint8_t* address_to_send, int num_address_bytes)
{

}

// function to send address from the host machine to the NAND flash
// .. Data is written from DQ[7:0] to the cache register of the selected die (LUN)
// .. .. on the rising edge of WE# when CE# is LOW, ALE is LOW, CLE is LOW, and RE# is HIGH
// .. CLE should be low
// .. ALE should be low
// .. CE should be low
// .. WE should be low
// .. put data on DQ and latch WE high for certain duration
// .. make WE low and repeat the procedure again for number of bytes required (int num_data)
void send_data(uint8_t* data_to_send,int num_data)
{

}

// function to receive data from the NAND device
// .. data can be received when on ready state (RDY signal)
// .. data can be received following READ operation
// .. the procedure should be as follows
// .. .. CE should be low
// .. .. ALE and CLE should be low
// .. .. ensure RDY is high
// .. .. WE should be high
// .. .. data is available at DQ pins on the falling edge of RE pin (RE is also input to NAND)
void get_data(uint8_t* data_received,int num_data)
{

}

// function to disable Program and Erase operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void write_protect()
{

}

// function to disable Program operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void disable_program()
{

}

// function to disable erase operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void disable_erase()
{

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

}

// function to reset the whole device
// .. issue a command 0xFF for reset operation
// .. following operations should be performed for this
// .. .. enable command cycle
// .. .. command to be sent is 0xFF
// .. .. check for R/B signal to be high after certain duration (should go low(busy) and go high (ready))
void reset_device()
{

}

// following function will reset just a particular LUN
// .. a LUN or logical unit is also called a NAND flash die (our NAND has 2 planes in a die)
// .. can be used to put the LUN into a known state or abort commadn sequences going on
// .. .. all the cache contents are invalid
// .. .. if any operation is going on, partial operations might take place
// .. .. the command issuing has to be followed by sequence of (3-byte) address of the LUN
void reset_LUN(uint8_t* address_LUN, int num_address_bytes)
{

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
void read_device_id_00(int* device_id_array, int num_data)
{

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
void read_device_id_20(int* device_id_array, int num_data)
{

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
void read_unique_id(int* device_id_array, int num_data)
{

}
#endif