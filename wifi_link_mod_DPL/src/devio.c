/******************************************************************************
 * 
 * TODO: License and terms, NDA, etc...
 *
 * FILE_NAME_
 * FILE_FUNCTION / DESCRIPTION 
 *****************************************************************************/
 #include "wifi_link.h"


/******************************************************************************
 * Global functions
 *****************************************************************************/


/*-----------------------------------------------------------------------------
 * Function name:   DEVIO_Init
 * Description:     Master initialization routine for all ports and driver code
 * Parameters:      NONE
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void DEVIO_Init(void)
{
	/* Set DCO frequency*/
    #ifndef FAST_CLOCK
        BCSCTL1 = CALBC1_1MHZ;
        DCOCTL = CALDCO_1MHZ;
    #else
        BCSCTL1 = CALBC1_16MHZ;
        DCOCTL = CALDCO_16MHZ;
    #endif

	/* Port 1 Directions */
	P1SEL = 0x00; // For now, just set all pins to digital IO.
	P1DIR |= DEVIO_P1OUT_TXD;
	P1DIR |= DEVIO_P1OUT_CTS;
	P1DIR |= DEVIO_P1OUT_SCK;
	P1DIR |= DEVIO_P1OUT_MOSI;
	
	//P1DIR &= ~(DEVIO_P1IN_RTS);
	P1DIR &= ~(DEVIO_P1IN_RXD);
	P1DIR &= ~(DEVIO_P1IN_MISO);
	P1DIR &= ~(DEVIO_P1IN_NRF_IRQ);
	
	/* Port 2 Directions */
	P2SEL = 0x00; // For now, just set all pins to digital IO.
	P2DIR |= DEVIO_P2OUT_NRF_CSN;
	P2DIR |= DEVIO_P2OUT_NRF_CE;
	P2DIR |= DEVIO_P2OUT_STAT_LED;
	
	//P2DIR &= ~(DEVIO_P2IN_SLEEP);

	/* Now that port directions are set, init the other driver code. */
	UART_Init();
	SPI_Init();
	NRF_Init();

	/* Finally, enable interrupts. */
	__bis_SR_register(GIE);
}

/*-----------------------------------------------------------------------------
 * Function name:   DEVIO_StatusLED_Set
 * Description:     Turns the status LED on or off
 * Parameters:      state - '0' off, else on
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void DEVIO_StatusLED_Set(uint8_t state)
{
    if(state != 0)
    {
        P2OUT |= DEVIO_P2OUT_STAT_LED;
    } else
    {
        P2OUT &= ~(DEVIO_P2OUT_STAT_LED);
    }
}

