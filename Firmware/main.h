/*
 * main.c
 *
 *   Created on: 6.01.2011
 *       Author: Mike Shatohin (brunql)
 *
 *      Project: USBtinyThermometer
 *          MCU: ATtiny2313
 *        Clock: 12MHz
 *
 *  Command to fill flash and set fuses:
 *  $ avrdude  -pt2313 -cusbasp -u -Uflash:w:USBtinyThermometer.hex:a -Ulfuse:w:0xfe:m -Uhfuse:w:0xd9:m -Uefuse:w:0xff:m
 *
 *  Copyright (c) 2010, 2011 Mike Shatohin, mikeshatohin [at] gmail.com
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
 

#ifndef MAIN_H_
#define MAIN_H_

#define LED      (D,6)

#endif /* MAIN_H_ */
