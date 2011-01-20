/*
 * main.c
 *
 *   Created on: 6.01.2011
 *       Author: Mike Shatohin (brunql)
 *
 *      Project: USBtinyThermometer
 *          MCU: ATtiny44
 *        Clock: 12MHz
 *
 *  Command to fill flash and set fuses:

 $ avrdude  -pt44 -cusbasp -u
            -Uflash:w:USBtinyThermometer.hex:a
            -Ulfuse:w:0xfe:m
            -Uhfuse:w:0xdc:m
            -Uefuse:w:0xff:m
 *
 *  Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  USBtinyThermometer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  USBtinyThermometer is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <util/delay.h>
#include "usb.h"
#include "main.h"
#include "def.h"
#include "version.h"
#include "commands.h"

#include <util/atomic.h>

#include "onewire.h"
#include "ds18x20.h"


volatile uint8_t cmd = CMD_NOP;
volatile uint8_t cmd_new = CMD_NOP;


volatile uint16_t temperature = 0xABCD;
volatile uint8_t result = 0x00;

#define NUMBER_OF_SENSORS   5

uint8_t sensorIDs[NUMBER_OF_SENSORS][OW_ROMCODE_SIZE];
volatile uint8_t nSensors = 0xff; // number of sensors on 1-wire



//
//  Software USB interface
//
uint8_t usb_setup( uint8_t data[8] )
{
	return 0xff; // call usb_in()
}

uint8_t usb_in( uint8_t* data, uint8_t len )
{
    if(len == 8){
        data[0] = (temperature & 0xff00) >> 8;
        data[1] = (temperature & 0x00ff);

        data[2] = VERSION_OF_HARDWARE_MAJOR;
        data[3] = VERSION_OF_HARDWARE_MINOR;

        data[4] = VERSION_OF_FIRMWARE_MAJOR;
        data[5] = VERSION_OF_FIRMWARE_MINOR;

        data[6] = result; // result of the last operation

        data[7] = nSensors;
    }else{
        for(uint8_t i=1; i <= len; i++){
            data[i] = i;
        }
        CLR(LED);
    }
    return len;
}

void usb_out( uint8_t* data, uint8_t len )
{
    cmd = data[0];
}


uint8_t SearchSensors(void)
{
    uint8_t result = 0;

    uint8_t id[OW_ROMCODE_SIZE];
    uint8_t ow_res = OW_SEARCH_FIRST;

    ow_reset();

    nSensors = 0;

    ow_res = OW_SEARCH_FIRST;
    while ( ow_res != OW_LAST_DEVICE && nSensors < NUMBER_OF_SENSORS ) {
        DS18X20_FindSensor( &ow_res, &id[0] );

        if( ow_res == OW_PRESENCE_ERR ) {
            result = OW_PRESENCE_ERR;
            break;
        }

        if( ow_res == OW_DATA_ERR ) {
            result = OW_DATA_ERR;
            break;
        }

        for (uint8_t i=0; i < OW_ROMCODE_SIZE; i++ )
            sensorIDs[nSensors][i] = id[i];

        nSensors++;
    }

    return result;
}


//
//  Main
//
int	main(void)
{
    usb_init();

	for(;;)
	{
        usb_poll();

        if(cmd == CMD_NOP){
            usb_poll();

        }else if(cmd == CMD_FIND_SENSORS){
            result = SearchSensors();
            cmd = CMD_NOP;

        }else if(cmd == CMD_START_MEASUREMENT){
            result = DS18X20_StartMeasurement();
            cmd = CMD_NOP;

        }else if(cmd == CMD_READ_TEMPERATURE_FIRST){
            temperature = DS18X20_ReadTemperature(&sensorIDs[FIRST_SENSOR][0]);
            cmd = CMD_NOP;

        }else if(cmd == CMD_READ_TEMPERATURE_SECOND){
            temperature = DS18X20_ReadTemperature(&sensorIDs[SECOND_SENSOR][0]);
            cmd = CMD_NOP;

        }else{
            usb_poll();
        }
	}
}
