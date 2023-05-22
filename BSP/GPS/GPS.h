/*
 * GY-NEO6MV2.h
 *
 *  Created on: 5 avr. 2023
 *      Author: Yassine HERMI
 */

#ifndef GPS_NEO6MV2_H_
#define GPS_NEO6MV2_H_

#include "stdint.h"


#ifdef GPS_NEO6M

typedef struct __attribute__((__packed__)) {
    uint32_t epoch_time;           //4 octets
    float latitude;               //4  octets
    float longitude;                //4 octets
    char speed;                    //1 octet
}Data_from_GPS;

void Get_Data(uint8_t buffer[13],uint8_t buff[13]); // receive data from GPS module
void GPS_Init(void);                                // Initialise the module's UART (UART4)

#endif

#endif /* GPS_NEO6MV2_H_ */
