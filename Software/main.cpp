/*
 * main.cpp
 *
 *   Created on: 7.01.2011
 *       Author: Mike Shatohin (brunql)
 *
 *      Project: USBtinyThermometer
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <usb.h>
#include "opendevice.h"

#include "../Firmware/usbtiny.h"    /* VID, PID */
#include "../Firmware/commands.h"
#include "version.h"


int verbose = 0;


static void printUsage()
{
    fprintf(stderr, "Print to stdout temperature requested from USB device\n");
    fprintf(stderr, "Project: USBtinyThermometer\n");
    fprintf(stderr, "Version: %s\n", VERSION_STR);
    fprintf(stderr, "Author: Mike Shatohin (http://brunql.github.com)\n");
    fprintf(stderr, "Project on github: https://github.com/brunql/USBtinyThermometer\n");
    fprintf(stderr, "Copyleft (c) 2011\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: USBtinyThermometer [-v] [-h]\n");
    fprintf(stderr, "  -h, --help       show this help\n");
    fprintf(stderr, "  -v, --verbose    vebose output\n");
    fprintf(stderr, "\n");
    fflush(stderr);
}

// Output str to stdout if 'verbose' option
static void out(const char *str)
{
    if(verbose){
        fprintf(stdout, str);
        fflush(stdout);
    }
}


int main(int argc, char **argv)
{
    char buffer[8];

    usb_dev_handle *handle = NULL;
    int result = 0;

    if(argc > 2){
        printUsage();
        exit(1);
    }else if(argc == 2){
        if((strcmp(argv[1], "-v") == 0) ||
           (strcmp(argv[1], "--verbose") == 0)){
            verbose = 1;
        }else{
            printUsage();
            exit(1);
        }
    }

    usb_init();

    if(usbOpenDevice(&handle, USBTINY_VENDOR_ID, NULL, USBTINY_DEVICE_ID, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", USBTINY_DEVICE_NAME, USBTINY_VENDOR_ID, USBTINY_DEVICE_ID);
        exit(2);
    }

    out("Device open success\n");

    out("Find temperature sensor...");

    buffer[0] = CMD_FIND_SENSOR;
    result = usb_control_msg(handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
    if(result == 8){
        out("OK\n");
    }else{
        out("FAIL\n");
        exit(3);
    }

    usleep(100000);

    out("Start measurement...");
    buffer[0] = CMD_START_MEASUREMENT;
    result = usb_control_msg(handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
    if(result == 8){
        out("OK\n");
    }else{
        out("FAIL\n");
        exit(3);
    }

    out("Sleep 1 second while measuring...\n");
    sleep(1);

    out("Read temperature...");
    buffer[0] = CMD_READ_TEMPERATURE;
    result = usb_control_msg(handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
    if(result == 8){
        out("OK\n");
    }else{
        out("FAIL\n");
        exit(3);
    }

    usleep(100000);

    out("Request temperature...");
    result = usb_control_msg(handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
    if(result == 8){
        out("OK\n");
    }else{
        out("FAIL\n");
        exit(3);
    }

    if(verbose){
        fprintf(stdout, "Buffer: ");
        for(int i = 0; i<sizeof(buffer); i++){
            fprintf(stdout, "0x%02x ", buffer[i] & 0xff);
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    uint8_t temp_lo = buffer[1] & 0xff;
    uint8_t temp_hi = buffer[0] & 0xff;

    int16_t temp = (temp_hi << 8) | (temp_lo);

    float temp_f = temp / 16.0; // low 4 bits of temp are fractional part

    out("Temperature = ");
    fprintf(stdout, "%+.4f", temp_f);
    out("\n");

    usb_close(handle);

    return 0;
}











