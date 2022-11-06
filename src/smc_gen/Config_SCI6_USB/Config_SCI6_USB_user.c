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
* File Name        : Config_SCI6_USB_user.c
* Component Version: 1.11.0
* Device(s)        : R5F51308AxFP
* Description      : This file implements device driver for Config_SCI6_USB.
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
#include "Config_SCI6_USB.h"
/* Start user code for include. Do not edit comment generated here */
#include "uart_usb.h"
#include "string.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint8_t * gp_sci6_tx_address;                /* SCI6 transmit buffer address */
extern volatile uint16_t  g_sci6_tx_count;                   /* SCI6 transmit data number */
extern volatile uint8_t * gp_sci6_rx_address;                /* SCI6 receive buffer address */
extern volatile uint16_t  g_sci6_rx_count;                   /* SCI6 receive data number */
extern volatile uint16_t  g_sci6_rx_length;                  /* SCI6 receive data length */
/* Start user code for global. Do not edit comment generated here */
extern volatile uint8_t   g_sci6_mode;
static volatile uint8_t g_sci6_rx_buf[RX_BUF_USB] = {0};     /* SCI6 internal receive buffer */
static void r_Config_SCI6_USB_restart_receiver(void);
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_SCI6_USB_Create_UserInit
* Description  : This function adds user code after initializing the SCI6 channel
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

void R_Config_SCI6_USB_Create_UserInit(void)
{
    /* Start user code for user init. Do not edit comment generated here */
    R_Config_SCI6_USB_Serial_Receive((uint8_t*)g_sci6_rx_buf, RX_BUF_USB);
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI6_USB_transmit_interrupt
* Description  : This function is TXI6 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI6_TXI6
#if FAST_INTERRUPT_VECTOR == VECT_SCI6_TXI6
__fast_interrupt static void r_Config_SCI6_USB_transmit_interrupt(void)
#else
__interrupt static void r_Config_SCI6_USB_transmit_interrupt(void)
#endif
{
    if (0U < g_sci6_tx_count)
    {
        SCI6.TDR = *gp_sci6_tx_address;
        if (SCI_TX_ZERO == g_sci6_mode)
        {
            *gp_sci6_tx_address = '\0'; // mark as done / free
        }
        gp_sci6_tx_address++;
        g_sci6_tx_count--;
    }
    else
    {
        SCI6.SCR.BIT.TIE = 0U;
        SCI6.SCR.BIT.TEIE = 1U;
    }
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI6_USB_transmitend_interrupt
* Description  : This function is TEI6 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI6_TEI6
#if FAST_INTERRUPT_VECTOR == VECT_SCI6_TEI6
__fast_interrupt static void r_Config_SCI6_USB_transmitend_interrupt(void)
#else
__interrupt static void r_Config_SCI6_USB_transmitend_interrupt(void)
#endif
{
    /* Set TXD6 pin */
    PORTD.PMR.BYTE &= 0xFEU;

    SCI6.SCR.BIT.TIE = 0U;
    SCI6.SCR.BIT.TE = 0U;
    SCI6.SCR.BIT.TEIE = 0U;

    r_Config_SCI6_USB_callback_transmitend();
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI6_USB_receive_interrupt
* Description  : This function is RXI6 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI6_RXI6
#if FAST_INTERRUPT_VECTOR == VECT_SCI6_RXI6
__fast_interrupt static void r_Config_SCI6_USB_receive_interrupt(void)
#else
__interrupt static void r_Config_SCI6_USB_receive_interrupt(void)
#endif
{
    #pragma diag_suppress=Pa082
    if (g_sci6_rx_length > g_sci6_rx_count)
    {
        /* rx buffer has space */
        uint8_t buf = SCI6.RDR;
        if (buf == MSG_START)
        {
//            /* message start detected -> reset incoming buffer */
//            memset((uint8_t*)g_sci6_rx_buf, '\0', RX_BUF_USB);
//            g_sci6_rx_count = 0U;
//            g_sci6_rx_length = RX_BUF_USB;
//            gp_sci6_rx_address = g_sci6_rx_buf;
        }
        /* append received byte to buffer */
        *gp_sci6_rx_address = buf;
        gp_sci6_rx_address++;
        g_sci6_rx_count++;
        if(buf == MSG_END)
        {
            /* message end detected -> forward received message */
            r_Config_SCI6_USB_callback_receiveend();
        }
    }
    else
    {
        /* rx buffer full, but no end character -> restart receiver */
        SCI6.SCR.BIT.RIE = 0U;
        SCI6.SCR.BIT.RE = 0U;
        r_Config_SCI6_USB_restart_receiver();
    }
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI6_USB_receiveerror_interrupt
* Description  : This function is ERI6 interrupt service routine
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

#pragma vector = VECT_SCI6_ERI6
#if FAST_INTERRUPT_VECTOR == VECT_SCI6_ERI6
__fast_interrupt static void r_Config_SCI6_USB_receiveerror_interrupt(void)
#else
__interrupt static void r_Config_SCI6_USB_receiveerror_interrupt(void)
#endif
{
    uint8_t err_type;

    r_Config_SCI6_USB_callback_receiveerror();

    /* Clear overrun, framing and parity error flags */
    err_type = SCI6.SSR.BYTE;
    err_type &= 0xC7U;
    err_type |= 0xC0U;
    SCI6.SSR.BYTE = err_type;

    r_Config_SCI6_USB_restart_receiver();
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI6_USB_callback_transmitend
* Description  : This function is a callback function when SCI6 finishes transmission
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI6_USB_callback_transmitend(void)
{
    /* Start user code for r_Config_SCI6_USB_callback_transmitend. Do not edit comment generated here */
    send_message_usb_done();
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI6_USB_callback_receiveend
* Description  : This function is a callback function when SCI6 finishes reception
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/

static void r_Config_SCI6_USB_callback_receiveend(void)
{
    /* Start user code for r_Config_SCI6_USB_callback_receiveend. Do not edit comment generated here */
    pass_message_usb((uint8_t*)g_sci6_rx_buf, g_sci6_rx_count);
    r_Config_SCI6_USB_restart_receiver();
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_Config_SCI6_USB_callback_receiveerror
* Description  : This function is a callback function when SCI6 reception encounters error
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
#pragma diag_suppress=Pe177
static void r_Config_SCI6_USB_callback_receiveerror(void)
{
    /* Start user code for r_Config_SCI6_USB_callback_receiveerror. Do not edit comment generated here */
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
void r_Config_SCI6_USB_restart_receiver(void)
{
    memset((uint8_t*)g_sci6_rx_buf, '\0', RX_BUF_USB);
    R_Config_SCI6_USB_Serial_Receive((uint8_t*)g_sci6_rx_buf, RX_BUF_USB);
}
/* End user code. Do not edit comment generated here */
