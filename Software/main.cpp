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
#include <math.h>
#include <usb.h>
#include "opendevice.h"

#include "../Firmware/usbtiny.h"    /* VID, PID */
#include "../Firmware/commands.h"
#include "version.h"


static void printUsage()
{
    fprintf(stderr, "Print to stdout temperature requested from USB device\n");
    fprintf(stderr, "Project: USBtinyThermometer\n");
    fprintf(stderr, "Version: %s\n", VERSION_STR);
    fprintf(stderr, "Author: Mike Shatohin (http://brunql.github.com)\n");
    fprintf(stderr, "Project on github: https://github.com/brunql/USBtinyThermometer\n");
    fprintf(stderr, "Copyleft (c) 2011\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Usage: USBtinyThermometer [-v] [-l] [-h] [-2] \n");
    fprintf(stderr, "Example: USBtinyThermometer -2 -v \n");
    fprintf(stderr, "  -h, --help       show this help\n");
    fprintf(stderr, "  -v, --verbose    vebose output\n");
    fprintf(stderr, "  -l, --loop       loop measurement\n");
    fprintf(stderr, "  -r, --round      rounded to one decimal\n");
    fprintf(stderr, "  -1, --first      measure first sensor (default)\n");
    fprintf(stderr, "  -2, --second     measure second sensor\n");
    fprintf(stderr, "\n");
    fflush(stderr);
}


// Size of the buffer using in usb data transfers
#define BUFF_SIZE           8
// Buffer indexes
#define BUFF_CMD_INDEX      0
#define BUFF_LAST_RESULT    6
#define BUFF_SENSORS_NUMBER_INDEX 7

#define DELAY_BETWEEN_USB_REQUESTS_us  100*1000

// Global variable verbose unsing in out(...) and outFail(...) functions
int verbose = 0;


// Output str to stdout if 'verbose' option set
static void out(const char *str)
{
    if(verbose){
        printf("%s", str);
        fflush(stdout);
    }
}

void outFail(const int usb_control_msg_res, const char buff[BUFF_SIZE], const char * usb_error_str)
{
    if(verbose){
        printf("  usb_control_msg_res = %d; \n", usb_control_msg_res);
        printf("  usb_error_str = %s; \n", usb_error_str);
        printf("  buffer = ");
        for(int i=0; i<BUFF_SIZE; i++){
            printf("0x%02X ", buff[i] & 0xff);
        }
        printf("\n");
        printf("  result_last_operation = 0x%02X; \n", (buff[BUFF_LAST_RESULT] & 0xff));
        fflush(stdout);
    }
}


int main(int argc, char **argv)
{
    char buffer[BUFF_SIZE];

    usb_dev_handle *handle = NULL;

    int result = 0;

    bool loop_measure = false;
    bool round_to_one_decimal = false;
    int measure_sensor_index = FIRST_SENSOR;

    if(argc > 3){
        printUsage();
        exit(1);
    }

    for(int i=1; i<argc; i++){
        if((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--verbose") == 0)){
            verbose = 1;
        }else if((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--loop") == 0)){
            loop_measure = true;
        }else if((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--round") == 0)){
            round_to_one_decimal = true;
        }else if((strcmp(argv[i], "-1") == 0) || (strcmp(argv[i], "--first") == 0)){
            measure_sensor_index = FIRST_SENSOR;
        }else if((strcmp(argv[i], "-2") == 0) || (strcmp(argv[i], "--second") == 0)){
            measure_sensor_index = SECOND_SENSOR;
        }else{
            printUsage();
            printf("'%s'\n", argv[i]);
            exit(1);
        }
    }

    usb_init();

    if(usbOpenDevice(&handle, USBTINY_VENDOR_ID, NULL, USBTINY_DEVICE_ID, NULL) != 0){
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", USBTINY_DEVICE_NAME, USBTINY_VENDOR_ID, USBTINY_DEVICE_ID);
        exit(2);
    }

    out("Device open success\n");


    do{

        out("Find temperature sensors...");

        usleep(DELAY_BETWEEN_USB_REQUESTS_us);

        buffer[BUFF_CMD_INDEX] = CMD_FIND_SENSORS;
        result = usb_control_msg(handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
        if(result == sizeof(buffer)){
            out("OK\n");

            // Check if we have only one sensor, but want to measure from second, tell this fact to user
            out("Get number of sensors..."); // just simple read buffer[BUFF_SENSORS_NUMBER_INDEX]
            usleep(DELAY_BETWEEN_USB_REQUESTS_us);
            buffer[BUFF_CMD_INDEX] = CMD_NOP;
            result = usb_control_msg(handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
            if(result == sizeof(buffer)){
                out("OK\n");
                if(buffer[BUFF_SENSORS_NUMBER_INDEX] == 0){
                    out("Error: haven't any sensors\n");
                    exit(5);
                }else if(measure_sensor_index == SECOND_SENSOR && buffer[BUFF_SENSORS_NUMBER_INDEX] < 2){
                    out("Error: have only one sensor\n");
                    exit(5);
                }
            }else{
                // Check number of sensors
                out("FAIL\n");
                outFail(result, buffer, usb_strerror());
                exit(3);
            }
        }else{
            // Find sensors
            out("FAIL\n");
            outFail(result, buffer, usb_strerror());
            exit(3);
        }

        usleep(DELAY_BETWEEN_USB_REQUESTS_us);

        out("Start measurement...");
        buffer[BUFF_CMD_INDEX] = CMD_START_MEASUREMENT;
        result = usb_control_msg(handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
        if(result == sizeof(buffer)){
            out("OK\n");
        }else{
            out("FAIL\n");
            outFail(result, buffer, usb_strerror());
            exit(3);
        }

        out("Sleep 1 second while measuring...\n");
        sleep(1);

        if(measure_sensor_index == FIRST_SENSOR){
            out("Read temperature from FIRST sensor...");
            buffer[BUFF_CMD_INDEX] = CMD_READ_TEMPERATURE_FIRST;
        }else if(measure_sensor_index == SECOND_SENSOR){
            out("Read temperature from SECOND sensor...");
            buffer[BUFF_CMD_INDEX] = CMD_READ_TEMPERATURE_SECOND;
        }else{
            fprintf(stderr, "Read temperature from sensor=%d; FAIL", measure_sensor_index);
            exit(4);
        }

        usleep(DELAY_BETWEEN_USB_REQUESTS_us);
        result = usb_control_msg(handle, USB_ENDPOINT_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
        if(result == sizeof(buffer)){
            out("OK\n");
        }else{
            out("FAIL\n");
            outFail(result, buffer, usb_strerror());
            exit(3);
        }

        usleep(DELAY_BETWEEN_USB_REQUESTS_us);

        out("Request temperature...");
        result = usb_control_msg(handle, USB_ENDPOINT_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE, 0, 0, 0, buffer, sizeof(buffer), 5000);
        if(result == sizeof(buffer)){
            out("OK\n");
        }else{
            out("FAIL\n");
            outFail(result, buffer, usb_strerror());
            exit(3);
        }

        if(verbose){
            printf("Buffer: ");
            for(unsigned i = 0; i<sizeof(buffer); i++){
                printf("0x%02x ", buffer[i] & 0xff);
            }
            printf("\n");
        }

        uint8_t temp_lo = buffer[1] & 0xff;
        uint8_t temp_hi = buffer[0] & 0xff;

        int16_t temp = (temp_hi << 8) | (temp_lo);

        float temp_f = temp / 16.0; // low 4 bits of temp are fractional part

        out("Temperature = ");

        round_to_one_decimal = true;
        if(round_to_one_decimal){
            temp_f *= 10;
            temp_f = round(temp_f);
            temp_f /= 10;
            printf("%+.1f\n", temp_f);
        }else{
            printf("%+.4f\n", temp_f);
        }

        out("\n");
    }while(loop_measure);

    usb_close(handle);

    return 0;
}











