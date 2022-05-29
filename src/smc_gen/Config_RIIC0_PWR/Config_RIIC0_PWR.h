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
* File Name        : Config_RIIC0_PWR.h
* Component Version: 1.11.0
* Device(s)        : R5F51308AxFP
* Description      : This file implements device driver for Config_RIIC0_PWR.
***********************************************************************************************************************/

#ifndef CFG_Config_RIIC0_PWR_H
#define CFG_Config_RIIC0_PWR_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_riic.h"

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define _F0_IIC0_SCL_LOW_LEVEL_PERIOD                          (0xF0U) /* SCL clock low-level period setting */
#define _F1_IIC0_SCL_HIGH_LEVEL_PERIOD                         (0xF1U) /* SCL clock high-level period setting */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_Config_RIIC0_PWR_Create(void);
void R_Config_RIIC0_PWR_Create_UserInit(void);
void R_Config_RIIC0_PWR_Start(void);
void R_Config_RIIC0_PWR_Stop(void);
MD_STATUS R_Config_RIIC0_PWR_Master_Send(uint16_t adr, uint8_t * const tx_buf, uint16_t tx_num);
MD_STATUS R_Config_RIIC0_PWR_Master_Send_Without_Stop(uint16_t adr, uint8_t * const tx_buf, uint16_t tx_num);
MD_STATUS R_Config_RIIC0_PWR_Master_Receive(uint16_t adr, uint8_t * const rx_buf, uint16_t rx_num);
void R_Config_RIIC0_PWR_IIC_StartCondition(void);
void R_Config_RIIC0_PWR_IIC_StopCondition(void);
static void r_Config_RIIC0_PWR_callback_transmitend(void);
static void r_Config_RIIC0_PWR_callback_receiveend(void);
static void r_Config_RIIC0_PWR_callback_receiveerror(MD_STATUS status);
static void r_Config_RIIC0_PWR_callback_transmiterror(MD_STATUS status);
/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif
