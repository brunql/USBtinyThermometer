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

#include "onewire.h"
#include "ds18x20.h"


volatile uint8_t cmd = CMD_NOP;
volatile uint16_t temperature = 0xFEEE; // near absolute zero
volatile uint8_t result = 0x00;


//
//  Software USB interface
//
uint8_t usb_setup( uint8_t data[8] )
{
	return 0xff; // call usb_in()
}

uint8_t usb_in( uint8_t* data, uint8_t len )
{
    data[0] = (temperature & 0xff00) >> 8;
    data[1] = (temperature & 0x00ff);

    data[2] = VERSION_OF_HARDWARE_MAJOR;
    data[3] = VERSION_OF_HARDWARE_MINOR;

    data[4] = VERSION_OF_FIRMWARE_MAJOR;
    data[5] = VERSION_OF_FIRMWARE_MINOR;

    data[6] = result; // result of the last operation

    return len;
}

void usb_out( uint8_t* data, uint8_t len )
{
    cmd = data[0];
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

		switch(cmd){
		case CMD_FIND_SENSOR:
		    SET(LED);
		    result = DS18X20_FindSensor();
		    CLR(LED);
		    cmd = CMD_NOP;
		    break;
		case CMD_START_MEASUREMENT:
		    SET(LED);
		    result = DS18X20_StartMeasurement();
		    CLR(LED);
		    cmd = CMD_NOP;
		    break;
		case CMD_READ_TEMPERATURE:
		    SET(LED);
		    temperature = DS18X20_ReadTemperature();
		    CLR(LED);
		    cmd = CMD_NOP;
		    break;
		default:
		    break;
		}
	}
}
