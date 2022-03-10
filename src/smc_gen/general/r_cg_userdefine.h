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
* File Name        : r_cg_userdefine.h
* Version          : 1.1.105
* Device(s)        : R5F51308AxFP
* Description      : User header file for code generation.
***********************************************************************************************************************/

#ifndef CG_USER_DEF_H
#define CG_USER_DEF_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/* Start user code for include. Do not edit comment generated here */
#include <stdbool.h>
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/
/* Start user code for register. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/* Start user code for macro define. Do not edit comment generated here */
#define RX_BUF_USB 50
#define TX_BUF_USB 150
#define RX_BUF_CELLMODULE 120
#define TX_BUF_CELLMODULE 20
#define RX_BUF_DISPLAY 50
#define TX_BUF_DISPLAY 50

#define MSG_START '!'
#define MSG_END '\n'

#define CELL_MODULE_CHAIN_1 1
#define CELL_MODULE_CHAIN_2 2
#define CELL_MODULE_CHAIN_3 3
#define CELL_MODULE_CHAIN_4 4
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/* Start user code for type define. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
/* Start user code for function. Do not edit comment generated here */
void Error_Handler(void)
{
    __disable_interrupt();
    LED_RE1_OFF
    LED_RE2_ON
    for(;;)
    {
        LED_RE1_TGL
        LED_RE2_TGL
        R_Config_IWDT_Restart();
        R_BSP_SoftwareDelay(200, BSP_DELAY_MILLISECS);
    }
}
/* End user code. Do not edit comment generated here */
#endif

