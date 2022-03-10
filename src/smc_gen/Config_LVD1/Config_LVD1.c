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
* File Name        : Config_LVD1.c
* Component Version: 1.10.0
* Device(s)        : R5F51308AxFP
* Description      : This file implements device driver for Config_LVD1.
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
#include "Config_LVD1.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_LVD1_Create
* Description  : This function initializes the LVD1
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_LVD1_Create(void)
{
    /* Disable LVD1 */
    SYSTEM.LVCMPCR.BIT.LVD1E = 0U;

    /* Set control registers */
    SYSTEM.LVDLVLR.BYTE &= 0xF0U;
    SYSTEM.LVDLVLR.BYTE |= _00_LVD_LVD1LVL_4_29;
    SYSTEM.LVD1CR0.BYTE = _40_LVD_LVD1RI_1 | _00_LVD_LVD1RN_0;

    R_Config_LVD1_Create_UserInit();
}

/***********************************************************************************************************************
* Function Name: R_Config_LVD1_Start
* Description  : This function starts the LVD1 operation
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_LVD1_Start(void)
{
    uint16_t protect_dummy = (uint16_t)(SYSTEM.PRCR.WORD & 0x000FU);
    uint16_t w_count;

    /* Disable protect bit */
    SYSTEM.PRCR.WORD = 0xA508U | protect_dummy;

    /* Voltage monitoring 1 reset enabled */
    SYSTEM.LVD1CR0.BYTE = (SYSTEM.LVD1CR0.BYTE & _F7_LVD_CR_BIT_MASK) | _01_LVD_LVD1RIE_ENABLE;

    /* Voltage detection 1 circuit enabled */
    SYSTEM.LVCMPCR.BIT.LVD1E = 1U;

    /* Wait for LVD voltage detection to start */
    for (w_count = 0U; w_count <= _0781_LVD1_STABLE_WAIT_TIME; w_count++)
    {
        nop();
    }

    /* Voltage monitoring 1 circuit comparison results output enabled */
    SYSTEM.LVD1CR0.BYTE = (SYSTEM.LVD1CR0.BYTE & _F7_LVD_CR_BIT_MASK) | _04_LVD_LVD1CMPE_ENABLE;

    /* Restore the previous state of the protect register */
    SYSTEM.PRCR.WORD = (uint16_t)(0xA500U | protect_dummy);
}

/***********************************************************************************************************************
* Function Name: R_Config_LVD1_Stop
* Description  : This function stops the LVD1 operation
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_LVD1_Stop(void)
{
    uint16_t protect_dummy = (uint16_t)(SYSTEM.PRCR.WORD & 0x000FU);

    /* Disable protect bit */
    SYSTEM.PRCR.WORD = 0xA508U | protect_dummy;

    /* Voltage monitoring 1 circuit comparison results output disabled */
    SYSTEM.LVD1CR0.BYTE = SYSTEM.LVD1CR0.BYTE & _F7_LVD_CR_BIT_MASK & (~_04_LVD_LVD1CMPE_ENABLE);

    /* The voltage detection 1 circuit disabled */
    SYSTEM.LVCMPCR.BIT.LVD1E = 0U;

    /* Voltage monitoring 1 reset disabled */
    SYSTEM.LVD1CR0.BYTE = SYSTEM.LVD1CR0.BYTE & _F7_LVD_CR_BIT_MASK & (~_01_LVD_LVD1RIE_ENABLE);

    /* Restore the previous state of the protect register */
    SYSTEM.PRCR.WORD = (uint16_t)(0xA500U | protect_dummy);
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
