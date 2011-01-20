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
// Changes: made it just for two temperature sensor
// ------------------------------------------------------------------------------ //

#include <stdlib.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "ds18x20.h"
#include "onewire.h"
#include "crc8.h"

#include "commands.h"


uint8_t scratchPad[DS18X20_SP_SIZE];


/*
 * Find two DS18X20 Sensors on 1-Wire-Bus
 */
uint8_t DS18X20_FindSensor(uint8_t *ow_res, uint8_t id[])
{
	uint8_t ret = DS18X20_OK;
    uint8_t go = 1;

    do {
        *ow_res = ow_rom_search( *ow_res, &id[0] );
        if ( *ow_res == OW_PRESENCE_ERR || *ow_res == OW_DATA_ERR ||
             *ow_res == OW_LAST_DEVICE ) {
            go  = 0;
            ret = DS18X20_ERROR;
        } else {
            if ( id[0] == DS18B20_FAMILY_CODE || id[0] == DS18S20_FAMILY_CODE ||
                 id[0] == DS1822_FAMILY_CODE ) {
                go = 0;
            }
        }
    } while (go);

    return ret;
}

/*
 * Start measurement for all sensors
 */
uint8_t DS18X20_StartMeasurement(void)
{
	uint8_t ret = DS18X20_START_FAIL;

	ow_reset();
	if( ow_input_pin_state() ) { // only send if bus is "idle" = high
	    ow_command( DS18X20_CONVERT_T, 0 );
		ret = DS18X20_OK;
	}

	return ret;
}

/*
 * Returns 1 if conversion is in progress, 0 if finished;
 * not available when parasite powered.
 */
uint8_t DS18X20_IsInProgress(void)
{
	return ow_bit_io( 1 ) ? DS18X20_CONVERSION_DONE : DS18X20_CONVERTING;
}

/*
 * Returns temperature of sensor specified by sensorIndex (mb: FIRST_SENSOR or SECOND_SENSOR)
 */
uint16_t DS18X20_ReadTemperature(uint8_t id[])
{
    uint16_t temperature = 0xf1f2;
	uint8_t ret = DS18X20_OK;

	ow_reset();

	ow_command( DS18X20_READ, &id[0] );

	for( uint8_t i = 0; i < DS18X20_SP_SIZE; i++ ) {
		scratchPad[i] = ow_byte_rd();
	}
	if( crc8( &scratchPad[0], DS18X20_SP_SIZE ) ) {
		ret = DS18X20_ERROR_CRC;
	}

	temperature = ((uint16_t)scratchPad[1] << 8) | scratchPad[0];

	return temperature;
}
