/*********************************************************************************
Title:    DS18X20-Functions via One-Wire-Bus
Author:   Martin Thomas <eversmith@heizung-thomas.de>   
          http://www.siwawi.arubi.uni-kl.de/avr-projects
Software: avr-gcc 4.3.3 / avr-libc 1.6.7 (WinAVR 3/2010) 
Hardware: any AVR - tested with ATmega16/ATmega32/ATmega324P and 3 DS18B20

Partly based on code from Peter Dannegger and others.

**********************************************************************************/

// ------------------------------------------------------------------------------ //
// Date: 7.01.11
// Edited by Mike Shatohin (mikeshatohin@gmail.com)
// Project: USBtinyThermometer
// Changes: made it just for one temperature sensor
// ------------------------------------------------------------------------------ //

#include <stdlib.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "ds18x20.h"
#include "onewire.h"
#include "crc8.h"


uint8_t sensorID[OW_ROMCODE_SIZE];
uint8_t scratchPad[DS18X20_SP_SIZE];


/* find DS18X20 Sensors on 1-Wire-Bus
   output: id is the rom-code of the sensor found */
uint8_t DS18X20_FindSensor(void)
{
	uint8_t ret = DS18X20_OK;
	uint8_t ow_res = 0;
	ow_res = ow_rom_search( OW_SEARCH_FIRST, sensorID );
	if (ow_res == OW_PRESENCE_ERR || ow_res == OW_DATA_ERR) {
	    ret = DS18X20_ERROR;
	}
	return ret;
}

/* start measurement (CONVERT_T) for all sensors if input id==NULL 
   or for single sensor where id is the rom-code */
uint8_t DS18X20_StartMeasurement(void)
{
	uint8_t ret = DS18X20_START_FAIL;

	ow_reset();
	if( ow_input_pin_state() ) { // only send if bus is "idle" = high
	    ow_command( DS18X20_CONVERT_T, NULL );
		ret = DS18X20_OK;
	}

	return ret;
}

// returns 1 if conversion is in progress, 0 if finished
// not available when parasite powered.
uint8_t DS18X20_IsInProgress(void)
{
	return ow_bit_io( 1 ) ? DS18X20_CONVERSION_DONE : DS18X20_CONVERTING;
}

uint16_t DS18X20_ReadTemperature(void)
{
	uint16_t ret = 0x0000;

	ow_command( DS18X20_READ, sensorID );
	for( uint8_t i = 0; i < DS18X20_SP_SIZE; i++ ) {
		scratchPad[i] = ow_byte_rd();
	}
	if( crc8( scratchPad, DS18X20_SP_SIZE ) ) {
		ret = 0xffff;
	}
	ret = ((uint16_t)scratchPad[1] << 8) | scratchPad[0];

	return ret;
}
