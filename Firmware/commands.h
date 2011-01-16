/*
 * commands.h
 *
 *  Created on: 07.01.2011
 *      Author: Mike Shatohin (brunql)
 *     Project: USBtinyThermometer
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

#ifndef COMMANDS_H_
#define COMMANDS_H_

enum CMD{
    CMD_NOP = 0x00,
    CMD_FIND_SENSORS,
    CMD_START_MEASUREMENT,
    CMD_READ_TEMPERATURE_FIRST,
    CMD_READ_TEMPERATURE_SECOND,
};


enum SENSORS{
    FIRST_SENSOR = 0,
    SECOND_SENSOR = 1,
};

#endif /* COMMANDS_H_ */
