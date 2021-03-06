/*
File: nand_interface_header.h
Description: This file has the essential functions that are needed for interfacing
			.. ONFI compatible NAND devices
			.. Each of the functions declared here are defined in file nand_interface_header.c
*/
#ifndef nand_interface_header_h
#define nand_interface_header_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


// macro for debug functionality
// .. set to false for experimentation
#define DEBUG false
#define FORCE_INLINE __attribute__((always_inline))
// set the following variable to true if you want to following functions:
// .. timer_start()
// .. timer_end()
#define TIMER_PROFILE false
// following are the registers in NIOS computer
// .. the base address
#define TIMER_BASE ((uint32_t*) 0xff202000)
// .. the control register
#define TIMER_CONTROL ((uint32_t*) 0xff202004)
// .. timer counter low (only 16-bits)
#define TIMER_COUNTER_LOW ((uint32_t*) 0xff202008)
// .. timer counter high (only 16-bits)
#define TIMER_COUNTER_HIGH ((uint32_t*) 0xff20200C)
// .. timer counter snapshot low (only 16-bits)
#define TIMER_COUNTER_SNAP_LOW ((uint32_t*) 0xff202010)
// .. timer counter snapshot high (only 16-bits)
#define TIMER_COUNTER_SNAP_HIGH ((uint32_t*) 0xff202014)
// .. instead of making call to timer_diff(), use the following statement
#define PRINT_CC_TAKEN printf(".. the last operation took %lu cc.\n",timer_diff())

#define STOP_PRINT_CC_TAKEN {\
								uint32_t clock_count = timer_diff();\
								printf(".. the last operation took %lu cc.\n",clock_count);\
							}

// function timer_start()
// .. following function starts the timer in downwards direction
// .. that means we will have max values in the counter and count downwards
FORCE_INLINE inline void timer_start()
{
	*TIMER_COUNTER_LOW = 0xffff;
	*TIMER_COUNTER_HIGH = 0xffff;
	// .. bit 0: interupt enable, bit 1: continuous mode, bit 2:start counting
	*TIMER_CONTROL = 0x006; //0b0110
}

// function timer_end()
// .. following function takes the snapshot from the counter and puts them in snap register
// .. do not make a call to this function from user code, see timer_diff instead
FORCE_INLINE inline void timer_end()
{
	// grab the snap shot value
	// .. to catch the snapshot value, just write anything to the snap register
	*TIMER_COUNTER_SNAP_LOW = 1;
	// now we can stop the timer
	// .. bit 3 is STOP
	*TIMER_CONTROL = 0x08;
}

// function timer_diff
// .. this function gives out the time in nanoseconds as measured by the timer
FORCE_INLINE inline  uint32_t timer_diff()
{
	// first stop the timer
	timer_end();
	// the initial value that we started was 0xff ff ff ff ff
	uint32_t clock_count = 0xffffffff-((*TIMER_COUNTER_SNAP_HIGH)*65536+(*TIMER_COUNTER_SNAP_LOW)); // in cc
	return clock_count;
}

// The computer system used here is DE1_SoC that runs at 100Mhz (10 ns period)
//  connection to the NAND is made in parallel port 1 on JP1
// .. the address of which is at 0xff200060
// .. so here we will create masks for the data to be read/written
#define JUMPER_LOCATION ((uint32_t*) 0xff200060)

// value 1 in direction register means output
// .. 0 means input
// .. following is just the address of the register
#define JUMPER_DIRECTION ((uint32_t*) 0xff200064)

#define PUSH_KEY_LOCATION ((uint32_t*) 0xff200050)

#define DQ_mask (0x000000ff)	//connected at D7D6..D0

#define WP_shift 8
#define WP_mask (0x1<<WP_shift)	// connected at D8

#define CLE_shift 11
#define CLE_mask (0x1<<CLE_shift) // connected at D11

#define ALE_shift 10
#define ALE_mask (0x1<<ALE_shift) //connected at D10

#define RE_shift 13
#define RE_mask (0x1<<RE_shift) // connected at D13

#define WE_shift 9
#define WE_mask (0x1<<WE_shift) // connected at D9

#define CE_shift 12
#define CE_mask (0x1<<CE_shift) // connected at D12

#define RB_shift 14
#define RB_mask (0x1<<RB_shift) // connected to D14


#define SAMPLE_TIME asm("nop");asm("nop")
#define HOLD_TIME {asm("nop");}
// 100ns
#define tWW asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop")
#define tWB {asm("nop"); asm("nop");}
#define tRR {for(uint8_t i=0;i<4;i++) asm("nop");}
#define tRHW tWB
#define tCCS tWB
#define tADL tWB
#define tWHR {for(uint8_t i=0;i<12;i++) asm("nop");} // .. tWHR = 120ns


// put the user defined header codes here
// .. all the operations here are asynchronous
// .. we are using TSOP NAND flash which only supports asynchronous interface

// function to initialize the data and command lines all in inactive state
// .. here the data lines are output for MCU and all other lines as well
// ... set data lines as input right before when needed
// R/B signal will be set as input
void set_pin_direction_inactive();

//function that sets the data lines as input
//.. to be used when data is to be received from NAND
// ... please do not forget to reset them to output once done
void set_datalines_direction_input();

//function that sets the data lines as output/default
//.. to be used when sending data or sending command
// ... this function must be called once the datalines are set as input
void set_datalines_direction_default();

//function that resets the values of the outputs pin values
// .. for default state so that we do not do any inadvertent
// .. operations
void set_default_pin_values();

// function to send an arbitrary command signal to the NAND device
// .. the procedure is as follows ( in the sequence )
// .. .. Write Enable should go low WE => low
// .. .. Chip Enable should go low CE => low
// .. .. ALE should go low ALE => low
// .. .. CLE should go high CLE => high
// .. .. send the command signal in the DQ pins
void send_command(uint8_t command_to_send);

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
void send_addresses(uint8_t* address_to_send, uint8_t num_address_bytes);
void send_address(uint8_t address_to_send);

// function to send address from the host machine to the NAND flash
// .. Data is written from DQ[7:0] to the cache register of the selected die (LUN)
// .. .. on the rising edge of WE# when CE# is LOW, ALE is LOW, CLE is LOW, and RE# is HIGH
// .. CLE should be low
// .. ALE should be low
// .. CE should be low
// .. WE should be low
// .. put data on DQ and latch WE high for certain duration
// .. make WE low and repeat the procedure again for number of bytes required (int num_data)
void send_data(uint8_t* data_to_send,uint16_t num_data);

// function to receive data from the NAND device
// .. data can be received when on ready state (RDY signal)
// .. data can be received following READ operation
// .. the procedure should be as follows
// .. .. CE should be low
// .. .. ALE and CLE should be low
// .. .. ensure RDY is high
// .. .. WE should be high
// .. .. data is available at DQ pins on the falling edge of RE pin (RE is also input to NAND)
void get_data(uint8_t* data_received,uint16_t num_data);

// .. .. data is available at DQ pins on the falling edge of RE pin (RE is also input to NAND)
void get_data_fast(uint8_t* data_received,uint16_t num_data);

// function to disable Program and Erase operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void write_protect();

// function to disable Program operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void disable_program();
void enable_program();

// function to disable erase operation
// .. when WP is low, program and erase operation are disabled
// .. when WP is high, program and erase operation are enabled
void disable_erase();
void write_enable();
void enable_erase();

// function to initialize the NAND device
// .. following are the tasks to be performed for initialization of NAND device
// .. provide Vcc (ie ramp Vcc)
// .. host must wait for R/B to be valid (R/B should be low for certain duration and be high
// .. .. to indicate device is ready) R/B is output from the NAND device
// .. issue 0xFF command after R/B goes high (Reset is the first command to be issued)
// .. R/B should be monotired again after issuing 0XFF command
void device_initialization();

// function to reset the whole device
// .. issue a command 0xFF for reset operation
// .. following operations should be performed for this
// .. .. enable command cycle
// .. .. command to be sent is 0xFF
// .. .. check for R/B signal to be high after certain duration (should go low(busy) and go high (ready))
void reset_device();

// following function will reset just a particular LUN
// .. a LUN or logical unit is also called a NAND flash die (our NAND has 2 planes in a die)
// .. can be used to put the LUN into a known state or abort commadn sequences going on
// .. .. all the cache contents are invalid
// .. .. if any operation is going on, partial operations might take place
// .. .. the command issuing has to be followed by sequence of (3-byte) address of the LUN
void reset_LUN(uint8_t* address_LUN, uint8_t num_address_bytes);

// function to read the device ID
// when read from address 00h, it returns 8-byte ID of which first 5-bytes are manufacturer ID
// follow the following sequences
// .. set command cycle
// .. .. send 90h as command
// .. set address sending mode
// .. ..send 00 as address
// .. wait for tWHR duration
// .. read 8-bytes from the DQ pins
void read_manufacturer_id(uint8_t* device_id_array);

// function that reads the device ID and tries to detect the device name
// .. call the function device_id at address 00h
// .. lookup table based finding for device name
void detect_device();

// function to read the 4-byte ONFI code
// when read from address 00h, it returns 4-byte ONFI code
// follow the following sequences
// .. set command cycle
// .. .. send 90h as command
// .. set address sending mode
// .. ..send 20 as address
// .. wait for tWHR duration
// .. read 4-bytes from the DQ pins
void read_ONFI_id(uint8_t* device_id_array);

void read_JEDEC_id(uint8_t* device_id_array);

// function to read the unique identifier programmed into the target
// .. only accepted when device is not busy
// .. start a command cycle with 0x70 as command
// .. start a address cycle of 0x00 as address
// .. target is busy for tR , can be tested using R/B# signal begin high to indicate if the device is ready
// .. one data byte is output per RE# toggle at DQ pin
// .. .. 32 bytes of data is received
// .. .. .. first 16 bytes is the unique ID and next 16-bytes is complement of the data
// .. .. .. XOR should be done to ensure correctness
void read_unique_id(uint8_t* device_id_array, uint8_t num_data);

void print_array(uint8_t* my_array, uint16_t len);

void read_status(uint8_t* status_value);

// use this for multi-LUN device to avoid bus contention
// .. r1, r2 and r3 are the three-bytes for the row addresss
void read_status_enhanced(uint8_t* status_value, uint8_t* r1r2r3);

// write a function to perform an read operation
void read_page(uint8_t* address,uint8_t address_length);

void read_page_cache_sequential(uint8_t* address, uint8_t address_length,uint8_t* data_read,uint16_t* data_read_len,uint16_t num_pages);

void change_read_column(uint8_t* col_address);

void change_read_column_enhanced(uint8_t* address);

void change_write_column(uint8_t* col_address);

void change_row_address(uint8_t* address);

void read_mode();

void program_page(uint8_t* address,uint8_t* data,uint16_t num_data);

void program_page_cache(uint8_t* address,uint8_t* data,uint16_t num_data,uint8_t num_pages);

void erase_block(uint8_t* row_address);

void partial_erase_block(uint8_t* row_address, uint8_t lp_cnt);

void timing_test(uint8_t t_count);

void timing_test_0nop();
void timing_test_1nop();
void timing_test_2nop();
void timing_test_3nop();
void timing_test_4nop();
void timing_test_5nop();
void timing_test_6nop();
void timing_test_7nop();

#endif