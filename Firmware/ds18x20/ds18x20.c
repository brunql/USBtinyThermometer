/*********************************************************************************
Title:    DS18X20-Functions via One-Wire-Bus
Author:   Martin Thomas <eversmith@heizung-thomas.de>   
          http://www.siwawi.arubi.uni-kl.de/avr-projects
Software: avr-gcc 4.3.3 / avr-libc 1.6.7 (WinAVR 3/2010) 
Hardware: any AVR - tested with ATmega16/ATmega32/ATmega324P and 3 DS18B20

Partly based on code from Peter Dannegger and others.

changelog:
20041124 - Extended measurements for DS18(S)20 contributed by Carsten Foss (CFO)
200502xx - function DS18X20_read_meas_single
20050310 - DS18x20 EEPROM functions (can be disabled to save flash-memory)
           (DS18X20_EEPROMSUPPORT in ds18x20.h)
20100625 - removed inner returns, added static function for read scratchpad
         . replaced full-celcius and fractbit method with decicelsius
           and maxres (degreeCelsius*10e-4) functions, renamed eeprom-functions,
           delay in recall_e2 replaced by timeout-handling
10100714 - ow_command_skip_last_recovery used for parasite-powerd devices so the
           strong pull-up can be enabled in time even with longer OW recovery times
**********************************************************************************/

#include <stdlib.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "ds18x20.h"
#include "onewire.h"
#include "crc8.h"

#define uart_puts_P_verbose(s__)


/* find DS18X20 Sensors on 1-Wire-Bus
   input/ouput: diff is the result of the last rom-search
                *diff = OW_SEARCH_FIRST for first call
   output: id is the rom-code of the sensor found */
uint8_t DS18X20_find_sensor( uint8_t *diff, uint8_t id[] )
{
	uint8_t go;
	uint8_t ret;

	ret = DS18X20_OK;
	go = 1;
	do {
		*diff = ow_rom_search( *diff, &id[0] );
		if ( *diff == OW_PRESENCE_ERR || *diff == OW_DATA_ERR ||
		     *diff == OW_LAST_DEVICE ) { 
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

///* get power status of DS18x20
//   input:   id = rom_code
//   returns: DS18X20_POWER_EXTERN or DS18X20_POWER_PARASITE */
//uint8_t DS18X20_get_power_status( uint8_t id[] )
//{
//	uint8_t pstat;
//
//	ow_reset();
//	ow_command( DS18X20_READ_POWER_SUPPLY, id );
//	pstat = ow_bit_io( 1 );
//	ow_reset();
//	return ( pstat ) ? DS18X20_POWER_EXTERN : DS18X20_POWER_PARASITE;
//}

/* start measurement (CONVERT_T) for all sensors if input id==NULL 
   or for single sensor where id is the rom-code */
uint8_t DS18X20_start_meas( /*uint8_t with_power_extern, uint8_t id[] */)
{
	uint8_t ret;

	ow_reset();
	if( ow_input_pin_state() ) { // only send if bus is "idle" = high
//		if ( with_power_extern != DS18X20_POWER_EXTERN ) {
//			ow_command_with_parasite_enable( DS18X20_CONVERT_T, id );
//			/* not longer needed: ow_parasite_enable(); */
//		} else {
			ow_command( DS18X20_CONVERT_T, NULL );
//		}
		ret = DS18X20_OK;
	} 
	else { 
		uart_puts_P_verbose( "DS18X20_start_meas: Short Circuit!\r" );
		ret = DS18X20_START_FAIL;
	}

	return ret;
}

//// returns 1 if conversion is in progress, 0 if finished
//// not available when parasite powered.
//uint8_t DS18X20_conversion_in_progress(void)
//{
//	return ow_bit_io( 1 ) ? DS18X20_CONVERSION_DONE : DS18X20_CONVERTING;
//}

uint8_t DS18X20_read_scratchpad( uint8_t id[], uint8_t sp[], uint8_t n )
{
	uint8_t i;
	uint8_t ret;

	ow_command( DS18X20_READ, id );
	for ( i = 0; i < n; i++ ) {
		sp[i] = ow_byte_rd();
	}
	if ( crc8( &sp[0], DS18X20_SP_SIZE ) ) {
		ret = DS18X20_ERROR_CRC;
	} else {
		ret = DS18X20_OK;
	}

	return ret;
}
