/******************************************************************************
 * 
 * TODO: License and terms, NDA, etc...
 *
 * FILE_NAME_
 * FILE_FUNCTION / DESCRIPTION 
 *****************************************************************************/
 
#ifndef DEVIO_H_
#define DEVIO_H_

#include "wifi_link.h"

/******************************************************************************
 * IO Ports
 *****************************************************************************/
/* UART Lines*/
//#define DEVIO_P1IN_RTS 			(0x01 << 0)
#define DEVIO_P1IN_RXD			(0x01 << 1)
#define DEVIO_P1OUT_TXD			(0x01 << 2)
#define DEVIO_P1OUT_CTS 		(0x01 << 3)

/* SPI Lines */
#define DEVIO_P1OUT_SCK			(0x01 << 5)
#define DEVIO_P1IN_MISO			(0x01 << 6)
#define DEVIO_P1OUT_MOSI		(0x01 << 7)

/* nRF24L SPI select line controls. */
#define NRF_SPI_ENABLE          (P2OUT &= ~DEVIO_P2OUT_NRF_CSN)
#define NRF_SPI_DISABLE         (P2OUT |= DEVIO_P2OUT_NRF_CSN)

/* nRF24L TX/RX power control lines. */
#define NRF_CE_HIGH             (P2OUT |= DEVIO_P2OUT_NRF_CE)
#define NRF_CE_LOW              (P2OUT &= ~DEVIO_P2OUT_NRF_CE)

/* nRF Module Control Lines */
#define DEVIO_P2IN_NRF_IRQ 		(0x01 << 2)
#define DEVIO_P2OUT_NRF_CSN 	(0x01 << 1)
#define DEVIO_P2OUT_NRF_CE 		(0x01 << 0)

/* Status LED */
#define DEVIO_P1OUT_STAT_LED 	(0x01 << 0)

/* FTDI USB Sleep status line. We should go low power when this drops low. */
#define DEVIO_P2IN_SLEEP 		(0x01 << 4)

/******************************************************************************
 * Global functions
 *****************************************************************************/
void DEVIO_Init(void);
void DEVIO_StatusLED_Set(uint8_t state);


#endif
