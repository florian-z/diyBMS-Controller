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
* File Name        : Config_SCI9_CellModule_user.c
* Component Version: 1.11.0
* Device(s)        : R5F51308AxFP
* Description      : This file implements device driver for Config_SCI9_CellModule.
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
#include "Config_SCI9_CellModule.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint8_t * gp_sci9_tx_address;                /* SCI9 transmit buffer address */
extern volatile uint16_t  g_sci9_tx_count;                   /* SCI9 transmit data number */
extern volatile uint8_t * gp_sci9_rx_address;                /* SCI9 receive buffer address */
extern volatile uint16_t  g_sci9_rx_count;                   /* SCI9 receive data number */
extern volatile uint16_t  g_sci9_rx_length;                  /* SCI9 receive data length */
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_SCI9_CellModule_Create_UserInit
* Description  : This function adds user code after initializing the SCI9 channel
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_SCI9_CellModule_Create_UserInit(void)
{
    /* Start user code for user init. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI9_CellModule_transmit_interrupt
* Description  : This function is TXI9 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI9_TXI9
#if FAST_INTERRUPT_VECTOR == VECT_SCI9_TXI9
__fast_interrupt static void r_Config_SCI9_CellModule_transmit_interrupt(void)
#else
__interrupt static void r_Config_SCI9_CellModule_transmit_interrupt(void)
#endif
{
    if (0U < g_sci9_tx_count)
    {
        SCI9.TDR = *gp_sci9_tx_address;
        gp_sci9_tx_address++;
        g_sci9_tx_count--;
    }
    else
    {
        SCI9.SCR.BIT.TIE = 0U;
        SCI9.SCR.BIT.TEIE = 1U;
    }
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI9_CellModule_transmitend_interrupt
* Description  : This function is TEI9 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI9_TEI9
#if FAST_INTERRUPT_VECTOR == VECT_SCI9_TEI9
__fast_interrupt static void r_Config_SCI9_CellModule_transmitend_interrupt(void)
#else
__interrupt static void r_Config_SCI9_CellModule_transmitend_interrupt(void)
#endif
{
    /* Set TXD9 pin */
    PORTB.PMR.BYTE &= 0x7FU;

    SCI9.SCR.BIT.TIE = 0U;
    SCI9.SCR.BIT.TE = 0U;
    SCI9.SCR.BIT.TEIE = 0U;

    r_Config_SCI9_CellModule_callback_transmitend();
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI9_CellModule_receive_interrupt
* Description  : This function is RXI9 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI9_RXI9
#if FAST_INTERRUPT_VECTOR == VECT_SCI9_RXI9
__fast_interrupt static void r_Config_SCI9_CellModule_receive_interrupt(void)
#else
__interrupt static void r_Config_SCI9_CellModule_receive_interrupt(void)
#endif
{
    #pragma diag_suppress=Pa082
    if (g_sci9_rx_length > g_sci9_rx_count)
    {
        *gp_sci9_rx_address = SCI9.RDR;
        gp_sci9_rx_address++;
        g_sci9_rx_count++;
    }

    #pragma diag_suppress=Pa082
    if (g_sci9_rx_length <= g_sci9_rx_count)
    {
        /* All data received */
        SCI9.SCR.BIT.RIE = 0U;
        SCI9.SCR.BIT.RE = 0U;
        r_Config_SCI9_CellModule_callback_receiveend();
    }
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI9_CellModule_receiveerror_interrupt
* Description  : This function is ERI9 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI9_ERI9
#if FAST_INTERRUPT_VECTOR == VECT_SCI9_ERI9
__fast_interrupt static void r_Config_SCI9_CellModule_receiveerror_interrupt(void)
#else
__interrupt static void r_Config_SCI9_CellModule_receiveerror_interrupt(void)
#endif
{
    uint8_t err_type;

    r_Config_SCI9_CellModule_callback_receiveerror();

    /* Clear overrun, framing and parity error flags */
    err_type = SCI9.SSR.BYTE;
    err_type &= 0xC7U;
    err_type |= 0xC0U;
    SCI9.SSR.BYTE = err_type;
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI9_CellModule_callback_transmitend
* Description  : This function is a callback function when SCI9 finishes transmission
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI9_CellModule_callback_transmitend(void)
{
    /* Start user code for r_Config_SCI9_CellModule_callback_transmitend. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI9_CellModule_callback_receiveend
* Description  : This function is a callback function when SCI9 finishes reception
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI9_CellModule_callback_receiveend(void)
{
    /* Start user code for r_Config_SCI9_CellModule_callback_receiveend. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI9_CellModule_callback_receiveerror
* Description  : This function is a callback function when SCI9 reception encounters error
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI9_CellModule_callback_receiveerror(void)
{
    /* Start user code for r_Config_SCI9_CellModule_callback_receiveerror. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
