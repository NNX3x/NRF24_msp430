/******************************************************************************
 *
 * TODO: License and terms, NDA, etc...
 *
 * FILE_NAME_
 * FILE_FUNCTION / DESCRIPTION
 *****************************************************************************/
#include "wifi_link.h"

/******************************************************************************
 * Static Functions
 *****************************************************************************/
static inline uint8_t nibbleToHex(uint8_t nib);

/******************************************************************************
 * Global variables
 *****************************************************************************/
volatile uint8_t UART_DataReady;
volatile uint8_t UART_DataRX;


/******************************************************************************
 * Global functions
 *****************************************************************************/


/*-----------------------------------------------------------------------------
 * Function name:   UART_Init
 * Description:     Initializes the UART routines on USCIA0
 * Parameters:      NONE
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void UART_Init(void)
{
	UART_DataReady = FALSE;
	UART_DataRX = 0;

	// Enable UART on UCA0
  	P1SEL = BIT1 + BIT2;                    // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;                   // P1.1 = RXD, P1.2=TXD

    UCA0CTL1 |= UCSSEL_2; // SMCLK
    #ifndef FAST_CLOCK
        UCA0BR0 = 104;                      // 1MHz 9600
        UCA0BR1 = 0;
        UCA0MCTL = UCBRS0;                  // Modulation UCBRSx = 1
    #else
        UCA0BR0 = 104;                      // 16MHz - 9600
        UCA0BR1 = 0;
        UCA0MCTL = UCBRF3 + UCOS16;         // Modulation USOS16 = 1, UCBRF = 3
    #endif

  	UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
  	IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt
}


/*-----------------------------------------------------------------------------
 * Function name:   UART_SendByte
 * Description:     Sends out a single byte on UART
 * Parameters:      data - Byte to send out.
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void UART_SendByte(uint8_t data)
{
	while (!(IFG2 & UCA0TXIFG)) {} // Make sure it isn't already tx'ing
	UCA0TXBUF = data;
}


/*-----------------------------------------------------------------------------
 * Function name:   UART_SendBuffer
 * Description:     Sends an entire buffer out on the UART
 * Parameters:      buffer - Buffer to send
 *                  length - Number of bytes in the buffer to send
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void UART_SendBuffer(uint8_t* buffer, uint8_t length)
{
    uint8_t i;

    for(i = 0; i < length; i++)
    {
        UART_SendByte(buffer[i]);
    }
}


/*-----------------------------------------------------------------------------
 * Function name:   UART_SendString
 * Description:     Sends out a NULL terminated string on the UART
 * Parameters:      str - NULL terminated string pointer
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void UART_SendString(const char* str)
{
    while(*str != '\0')
    {
        UART_SendByte((uint8_t)(*str));
        str++;
    }
}


/*-----------------------------------------------------------------------------
 * Function name:   UART_SendHexByte
 * Description:     Sends a byte out on the UART in a human readable (ASCII)
 *                  format. (1 byte of data in, 2 bytes of ASCII out)
 * Parameters:      data - byte to send as human readable hex
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void UART_SendHexByte(uint8_t data)
{
    UART_SendByte(nibbleToHex(data >> 0x04));
    UART_SendByte(nibbleToHex(data & 0x0F));
}


/*-----------------------------------------------------------------------------
 * Function name:   nibbleToHex
 * Description:     Converts the lower four bits (nibble) of the given byte
 *                  into its ASCII equivalent hex character.
 * Parameters:      nib - nibble to convert
 * Returns:         Character representation
 *---------------------------------------------------------------------------*/
static inline uint8_t nibbleToHex(uint8_t nib)
{
    if(nib > 9)
    {
        return (nib + 'A' - 10);
    }
    return (nib + '0');
}


/*-----------------------------------------------------------------------------
 * Function name:   USCI0RX_ISR
 * Description:     ISR for USCIA0RX and USCIB0RX. Only designed to handle the
 *                  flag for USCIA0RX, a call caused by USCIB0RX with most
 *                  likely cause an infinite loop of DOOM!!!
 *---------------------------------------------------------------------------*/
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
	// Check that we were called by the correct interrupt flag....
    if(IFG2 & UCA0RXIFG)
	{
	    UART_DataRX = UCA0RXBUF;            // Read the received data out,
	                                        // this clears the interrupt flag.
	    UART_DataReady = TRUE;              // Set software flag to indicate
	                                        // new data is available.
	}
}

