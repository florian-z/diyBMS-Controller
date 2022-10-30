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
* Copyright (C) 2018 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Pin.c
* Version          : 1.0.2
* Device(s)        : R5F51308AxFP
* Description      : This file implements SMC pin code generation.
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Pins_Create
* Description  : This function initializes Smart Configurator pins
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Pins_Create(void)
{
    R_BSP_RegisterProtectDisable(BSP_REG_PROTECT_MPC);

    /* Set IRQ1 pin */
    MPC.P31PFS.BYTE = 0x40U;
    PORT3.PMR.BYTE &= 0xFDU;
    PORT3.PDR.BYTE &= 0xFDU;

    /* Set IRQ2 pin */
    MPC.P32PFS.BYTE = 0x40U;
    PORT3.PMR.BYTE &= 0xFBU;
    PORT3.PDR.BYTE &= 0xFBU;

    /* Set MISOA pin */
    MPC.PA7PFS.BYTE = 0x0DU;
    PORTA.PMR.BYTE |= 0x80U;

    /* Set MOSIA pin */
    MPC.PA6PFS.BYTE = 0x0DU;
    PORTA.PMR.BYTE |= 0x40U;

    /* Set RSPCKA pin */
    MPC.PA5PFS.BYTE = 0x0DU;
    PORTA.PMR.BYTE |= 0x20U;

    /* Set RXD0 pin */
    MPC.P21PFS.BYTE = 0x0AU;
    PORT2.PMR.BYTE |= 0x02U;

    /* Set RXD1 pin */
    MPC.P15PFS.BYTE = 0x0AU;
    PORT1.PMR.BYTE |= 0x20U;

    /* Set RXD5 pin */
    MPC.PC2PFS.BYTE = 0x0AU;
    PORTC.PMR.BYTE |= 0x04U;

    /* Set RXD6 pin */
    MPC.PD1PFS.BYTE = 0x0BU;
    PORTD.PMR.BYTE |= 0x02U;

    /* Set RXD12 pin */
    MPC.PE2PFS.BYTE = 0x0CU;
    PORTE.PMR.BYTE |= 0x04U;

    /* Set SCL pin */
    MPC.P12PFS.BYTE = 0x0FU;
    PORT1.PMR.BYTE |= 0x04U;

    /* Set SDA pin */
    MPC.P13PFS.BYTE = 0x0FU;
    PORT1.PMR.BYTE |= 0x08U;

    /* Set TXD0 pin */
    PORT2.PODR.BYTE |= 0x01U;
    MPC.P20PFS.BYTE = 0x0AU;
    PORT2.PDR.BYTE |= 0x01U;
    // PORT2.PMR.BIT.B0 = 1U; // Please set the PMR bit after TE bit is set to 1.

    /* Set TXD1 pin */
    PORT1.PODR.BYTE |= 0x40U;
    MPC.P16PFS.BYTE = 0x0AU;
    PORT1.PDR.BYTE |= 0x40U;
    // PORT1.PMR.BIT.B6 = 1U; // Please set the PMR bit after TE bit is set to 1.

    /* Set TXD5 pin */
    PORTC.PODR.BYTE |= 0x08U;
    MPC.PC3PFS.BYTE = 0x0AU;
    PORTC.PDR.BYTE |= 0x08U;
    // PORTC.PMR.BIT.B3 = 1U; // Please set the PMR bit after TE bit is set to 1.

    /* Set TXD6 pin */
    PORTD.PODR.BYTE |= 0x01U;
    MPC.PD0PFS.BYTE = 0x0BU;
    PORTD.PDR.BYTE |= 0x01U;
    // PORTD.PMR.BIT.B0 = 1U; // Please set the PMR bit after TE bit is set to 1.

    /* Set TXD12 pin */
    PORTE.PODR.BYTE |= 0x02U;
    MPC.PE1PFS.BYTE = 0x0CU;
    PORTE.PDR.BYTE |= 0x02U;
    // PORTE.PMR.BIT.B1 = 1U; // Please set the PMR bit after TE bit is set to 1.

    R_BSP_RegisterProtectEnable(BSP_REG_PROTECT_MPC);
}

