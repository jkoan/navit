/* Copyright (c) 2017-2021, Olaf Brandt
 * nRF5 SDK Copyright (c) 2014 - 2017, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in sbhsbtce and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of sbhsbtce code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in a processor manufactured by Nordic
 *   Semiconductor ASA, or in a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef CUSTOMBOARD_H
#define CUSTOMBOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

//define for 24V version. Comment out for 12V
#define TWENTYFOUR_VOLT_VERSION
#define BISTABLE_ON_DELAY 110  // How long a bistable relais shall be triggered

//GPIO MAPPING
#define BHSONGPIO	00		//BHS ON
#define BHSOFFGPIO	30		//BHS OFF
#define AUXLGPIO	29		//AUXL
#define AUXRGPIO	28		//AUXR
#define AUX2GPIO	24		//AUX2
#define AUX3GPIO	23		//AUX3
#define AUX4GPIO	21		//AUX4
#define AUX5GPIO	22		//AUX5
#define SUPPLY_EN	12		//3.3V Supply enable
#define SUPPLY_NBIOT 11		//5V Supply enable

#define SPI_CS 10

#define TWI_INSTANCE_ID     0
#define SCL_PIN 		NRF_GPIO_PIN_MAP(0,4)
#define SDA_PIN 		NRF_GPIO_PIN_MAP(0,6)

#define SPI_INSTANCE_ID     1
#define SPI_SS_PIN 		NRF_GPIO_PIN_MAP(0,10)		// Slave Select P0.8
#define SPI_MISO_PIN 	NRF_GPIO_PIN_MAP(0,5)		// SDO PIN35 P0.5
#define SPI_MOSI_PIN 	NRF_GPIO_PIN_MAP(0,6)		// SDI == SDA P0.6
#define SPI_SCK_PIN 	NRF_GPIO_PIN_MAP(0,4)		// SCK
#define ADXL_INT1_PIN	7	//7
#define ADXL_INT2_PIN	8	//8

//#define LED_START      18
//#define LED_0          18
//#define LED_1          20
////#define LED_2          18
////#define LED_3          20
//#define LED_STOP       20
//
//#define LEDS_ACTIVE_STATE 1
//
//#define BSP_LED_0      LED_0
//#define BSP_LED_1      LED_1
////#define BSP_LED_2      LED_2
////#define BSP_LED_3      LED_3
//
//#define LEDS_INV_MASK  0x00
//
//#define BUTTON_START   15
//#define BUTTON_0       15
//#define BUTTON_1       17
//#define BUTTON_STOP    17
//#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP
//
//#define BUTTONS_ACTIVE_STATE 0
//
//#define BSP_BUTTON_0   BUTTON_0
//#define BSP_BUTTON_1   BUTTON_1
//
//#define BUTTONS_NUMBER 2
//#define LEDS_NUMBER    2
//
//#define BUTTONS_LIST { BUTTON_0, BUTTON_1 }
//#define LEDS_LIST { LED_0, LED_1 }
//
//#define RX_PIN_NUMBER  14
//#define TX_PIN_NUMBER  16
//#define CTS_PIN_NUMBER 0xFF
//#define RTS_PIN_NUMBER 0xFF
//
//#define GPIOGSMPWR 19	//PWR GSM
//#define GPIOGSMDTR 13	//SLEEP GSM

// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source        = NRF_CLOCK_LF_SRC_XTAL,            \
                                 .rc_ctiv       = 0,                                \
                                 .rc_temp_ctiv  = 0,                                \
                                 .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM}

#ifdef __cplusplus
}
#endif

#endif
