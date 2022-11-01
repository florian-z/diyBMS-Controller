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
* File Name        : Config_RSPI0_Shunt.h
* Component Version: 1.11.0
* Device(s)        : R5F51308AxFP
* Description      : This file implements device driver for Config_RSPI0_Shunt.
***********************************************************************************************************************/

#ifndef CFG_Config_RSPI0_Shunt_H
#define CFG_Config_RSPI0_Shunt_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_rspi.h"

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define _07_RSPI0_DIVISOR               (0x07U) /* SPBR(RSPI bit rate) register value */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_Config_RSPI0_Shunt_Create(void);
void R_Config_RSPI0_Shunt_Create_UserInit(void);
void R_Config_RSPI0_Shunt_Start(void);
void R_Config_RSPI0_Shunt_Stop(void);
MD_STATUS R_Config_RSPI0_Shunt_Send_Receive(uint16_t * const tx_buf, uint16_t tx_num, uint16_t * const rx_buf);
static void r_Config_RSPI0_Shunt_callback_transmitend(void);
static void r_Config_RSPI0_Shunt_callback_receiveend(void);
static void r_Config_RSPI0_Shunt_callback_error(uint8_t err_type);
/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif
