/******************************************************************************
 * 
 * TODO: License and terms, NDA, etc...
 * 
 * FILE_NAME_
 * FILE_FUNCTION / DESCRIPTION
 *****************************************************************************/
 #include "wifi_link.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#define PAYLOAD_WIDTH 5                     // size of an RX/TX payload.

/******************************************************************************
 * Static variables
 *****************************************************************************/


/******************************************************************************
 * Static functions
 *****************************************************************************/

/*-----------------------------------------------------------------------------
 * Function name:   main
 * Description:     Program entry point. There is NO ESCAPE!! (diabolical laughter..)
 * Parameters:      NONE
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
int main()
{
	uint8_t stat = 0;
	uint8_t ret;
	uint8_t retBuf[PAYLOAD_WIDTH];

	/* Disable watchdog */
	//TODO: Implement the WDT, just not for debug mode.
	WDTCTL = WDTPW + WDTHOLD; 

	/* Initialize IO, set frequency, initialize other driver code, etc */
	DEVIO_Init();
	nrf24_init();
	RX_init();
	UART_SendString("Goliath Online!\n");
	/* Main loop and state machine*/
	while(1)
	{

	    //TODO: split out most of this into tasks in other files
	    //TODO: Implement LPM when there is nothing to do.
		if (!(DEVIO_P2IN_NRF_IRQ & P2IN)) {

			ret = SPI_SendCommand(CMD_REG_READ + REG_STATUS, DUMMY_BYTE); // IRQ flag set.
			if (ret & TX_DS) {
				SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, TX_DS);  // TX was acknowledged, clear interrupt flag, set back to PRX
				UART_SendString("TX-ACK'ed\n");

			}

			else if (ret & RX_DR) {
				// Data was received, read payload, clear interrupt flag, check for more data
				do {
					SPI_GetBuffer(CMD_RX_READ, retBuf, PAYLOAD_WIDTH);
					SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, RX_DR);
					UART_SendString("GET-RX:");
					UART_SendBuffer(retBuf, PAYLOAD_WIDTH);
					UART_SendString("\n");

					ret = SPI_SendCommand(CMD_REG_READ + REG_FIFO_STATUS,
							DUMMY_BYTE);
				} while (!(ret & RX_FIFO_EMPTY));
			}
		}


	    //TODO: Change the status LED to reflect what is actually happening,
	    // such as on when there is data on the UART or we are actively
	    // communicating with the wireless module.
		// Blink the status LED for debug.
		stat ^= 0x01;
		DEVIO_StatusLED_Set(stat);
		//__bis_SR_register(LPM2_bits);
	}
}
