/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Config_PORT.h
* Component Version: 2.3.0
* Device(s)        : R5F51308AxFP
* Description      : This file implements device driver for Config_PORT.
***********************************************************************************************************************/

#ifndef CFG_Config_PORT_H
#define CFG_Config_PORT_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_port.h"

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_Config_PORT_Create(void);
void R_Config_PORT_Create_UserInit(void);
/* Start user code for function. Do not edit comment generated here */
#define LED_GN1_ON      PORT4.PODR.BYTE |= (1<<0);
#define LED_GN1_OFF     PORT4.PODR.BYTE &= ~(1<<0);
#define LED_GN1_TGL     PORT4.PODR.BYTE ^= (1<<0);
#define LED_GE1_ON      PORT4.PODR.BYTE |= (1<<1);
#define LED_GE1_OFF     PORT4.PODR.BYTE &= ~(1<<1);
#define LED_GE1_TGL     PORT4.PODR.BYTE ^= (1<<1);
#define LED_RT1_ON      PORT4.PODR.BYTE |= (1<<2);
#define LED_RT1_OFF     PORT4.PODR.BYTE &= ~(1<<2);
#define LED_RT1_TGL     PORT4.PODR.BYTE ^= (1<<2);
#define LED_BL1_ON      PORT4.PODR.BYTE |= (1<<3);
#define LED_BL1_OFF     PORT4.PODR.BYTE &= ~(1<<3);
#define LED_BL1_TGL     PORT4.PODR.BYTE ^= (1<<3);

#define LED_GN2_ON      PORT4.PODR.BYTE |= (1<<4);
#define LED_GN2_OFF     PORT4.PODR.BYTE &= ~(1<<4);
#define LED_GN2_TGL     PORT4.PODR.BYTE ^= (1<<4);
#define LED_GE2_ON      PORT4.PODR.BYTE |= (1<<5);
#define LED_GE2_OFF     PORT4.PODR.BYTE &= ~(1<<5);
#define LED_GE2_TGL     PORT4.PODR.BYTE ^= (1<<5);
#define LED_RT2_ON      PORT4.PODR.BYTE |= (1<<6);
#define LED_RT2_OFF     PORT4.PODR.BYTE &= ~(1<<6);
#define LED_RT2_TGL     PORT4.PODR.BYTE ^= (1<<6);
#define LED_BL2_ON      PORT4.PODR.BYTE |= (1<<7);
#define LED_BL2_OFF     PORT4.PODR.BYTE &= ~(1<<7);
#define LED_BL2_TGL     PORT4.PODR.BYTE ^= (1<<7);

#define OUT_CHARGER_LOAD_ON     PORTD.PODR.BYTE |= (1<<4)
#define OUT_CHARGER_LOAD_OFF    PORTD.PODR.BYTE &= ~(1<<4);
#define OUT_CHARGER_DOOR_ON     PORTD.PODR.BYTE |= (1<<5);
#define OUT_CHARGER_DOOR_OFF    PORTD.PODR.BYTE &= ~(1<<5);

#define IN_SIGNAL_LINE_PWR      (PORT3.PIDR.B1)
#define IN_SIGNAL_KL15_PWR      (PORT3.PIDR.B2)

#define OUT_SPI_nMSS_ON         PORTC.PODR.BYTE |= (1<<4);
#define OUT_SPI_nMSS_OFF        PORTC.PODR.BYTE &= ~(1<<4);
/* End user code. Do not edit comment generated here */
#endif
