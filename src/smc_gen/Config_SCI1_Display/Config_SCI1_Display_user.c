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
* File Name        : Config_SCI1_Display_user.c
* Component Version: 1.11.0
* Device(s)        : R5F51308AxFP
* Description      : This file implements device driver for Config_SCI1_Display.
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
#include "Config_SCI1_Display.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint8_t * gp_sci1_tx_address;                /* SCI1 transmit buffer address */
extern volatile uint16_t  g_sci1_tx_count;                   /* SCI1 transmit data number */
extern volatile uint8_t * gp_sci1_rx_address;                /* SCI1 receive buffer address */
extern volatile uint16_t  g_sci1_rx_count;                   /* SCI1 receive data number */
extern volatile uint16_t  g_sci1_rx_length;                  /* SCI1 receive data length */
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_SCI1_Display_Create_UserInit
* Description  : This function adds user code after initializing the SCI1 channel
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_SCI1_Display_Create_UserInit(void)
{
    /* Start user code for user init. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI1_Display_transmit_interrupt
* Description  : This function is TXI1 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI1_TXI1
#if FAST_INTERRUPT_VECTOR == VECT_SCI1_TXI1
__fast_interrupt static void r_Config_SCI1_Display_transmit_interrupt(void)
#else
__interrupt static void r_Config_SCI1_Display_transmit_interrupt(void)
#endif
{
    if (0U < g_sci1_tx_count)
    {
        SCI1.TDR = *gp_sci1_tx_address;
        gp_sci1_tx_address++;
        g_sci1_tx_count--;
    }
    else
    {
        SCI1.SCR.BIT.TIE = 0U;
        SCI1.SCR.BIT.TEIE = 1U;
    }
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI1_Display_transmitend_interrupt
* Description  : This function is TEI1 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI1_TEI1
#if FAST_INTERRUPT_VECTOR == VECT_SCI1_TEI1
__fast_interrupt static void r_Config_SCI1_Display_transmitend_interrupt(void)
#else
__interrupt static void r_Config_SCI1_Display_transmitend_interrupt(void)
#endif
{
    /* Set TXD1 pin */
    PORT1.PMR.BYTE &= 0xBFU;

    SCI1.SCR.BIT.TIE = 0U;
    SCI1.SCR.BIT.TE = 0U;
    SCI1.SCR.BIT.TEIE = 0U;

    r_Config_SCI1_Display_callback_transmitend();
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI1_Display_receive_interrupt
* Description  : This function is RXI1 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI1_RXI1
#if FAST_INTERRUPT_VECTOR == VECT_SCI1_RXI1
__fast_interrupt static void r_Config_SCI1_Display_receive_interrupt(void)
#else
__interrupt static void r_Config_SCI1_Display_receive_interrupt(void)
#endif
{
    #pragma diag_suppress=Pa082
    if (g_sci1_rx_length > g_sci1_rx_count)
    {
        *gp_sci1_rx_address = SCI1.RDR;
        gp_sci1_rx_address++;
        g_sci1_rx_count++;
    }

    #pragma diag_suppress=Pa082
    if (g_sci1_rx_length <= g_sci1_rx_count)
    {
        /* All data received */
        SCI1.SCR.BIT.RIE = 0U;
        SCI1.SCR.BIT.RE = 0U;
        r_Config_SCI1_Display_callback_receiveend();
    }
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI1_Display_receiveerror_interrupt
* Description  : This function is ERI1 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI1_ERI1
#if FAST_INTERRUPT_VECTOR == VECT_SCI1_ERI1
__fast_interrupt static void r_Config_SCI1_Display_receiveerror_interrupt(void)
#else
__interrupt static void r_Config_SCI1_Display_receiveerror_interrupt(void)
#endif
{
    uint8_t err_type;

    r_Config_SCI1_Display_callback_receiveerror();

    /* Clear overrun, framing and parity error flags */
    err_type = SCI1.SSR.BYTE;
    err_type &= 0xC7U;
    err_type |= 0xC0U;
    SCI1.SSR.BYTE = err_type;
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI1_Display_callback_transmitend
* Description  : This function is a callback function when SCI1 finishes transmission
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI1_Display_callback_transmitend(void)
{
    /* Start user code for r_Config_SCI1_Display_callback_transmitend. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI1_Display_callback_receiveend
* Description  : This function is a callback function when SCI1 finishes reception
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI1_Display_callback_receiveend(void)
{
    /* Start user code for r_Config_SCI1_Display_callback_receiveend. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI1_Display_callback_receiveerror
* Description  : This function is a callback function when SCI1 reception encounters error
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI1_Display_callback_receiveerror(void)
{
    /* Start user code for r_Config_SCI1_Display_callback_receiveerror. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
