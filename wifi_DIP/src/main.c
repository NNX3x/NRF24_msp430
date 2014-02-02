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

/* Main function finite state machine. */
typedef enum
{
    IDLE,
    INIT,
    TX_SEND,
    INIT_RX,
    FIFO_STATUS,
    RX_STATUS,
    RX_READ,
    STATUS,
    CLEAR
} MainState_E;


/******************************************************************************
 * Static variables
 *****************************************************************************/
static uint8_t Payload[PAYLOAD_WIDTH];


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
    MainState_E State = INIT;
	uint8_t stat = 0;
	uint8_t ret;
	uint8_t retBuf[PAYLOAD_WIDTH];
	uint8_t index;

	/* Disable watchdog */
	//TODO: Implement the WDT, just not for debug mode.
	WDTCTL = WDTPW + WDTHOLD; 

	/* Initialize IO, set frequency, initialize other driver code, etc */
	DEVIO_Init();

	/* Main loop and state machine*/
	while(1)
	{
	    //TODO: split out most of this into tasks in other files
	    //TODO: Implement LPM when ther is nothing to do.
	    switch(State)
	    {
	    case IDLE:
	        if(UART_DataReady == TRUE)
	        {
	            UART_DataReady = FALSE;

	            switch(UART_DataRX)
	            {
	            case 'i':
	                State = INIT;
	                break;
	            case 't':
	                State = TX_SEND;
	                break;
	            case 'r':
	                State = INIT_RX;
	                break;
	            case 'f':
	                State = FIFO_STATUS;
	                break;
	            case 'R':
	                State = RX_STATUS;
	                break;
	            case 'g':
	                State = RX_READ;
	                break;
	            case 's':
	                State = STATUS;
	                break;
	            case 'c':
	                State = CLEAR;
	                break;
	            }
	        } else if (!(DEVIO_P2IN_NRF_IRQ & P2IN))
	        {
	            // IRQ flag set.
	            ret = SPI_SendCommand(CMD_REG_READ + REG_STATUS, DUMMY_BYTE);
	            if(ret & TX_DS)
	            {
	                // TX was acknowledged, clear interrupt flag, set back to PRX
	                SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, TX_DS);
	                UART_SendString("TX-ACK'ed\n\r");
	                State = INIT_RX;
	            }
	            else if (ret & MAX_RT)
	            {
	                // Maximum number of TX retries reached, clear flags, set back to PRX
	                UART_SendString("MAX_RT reached, reseting.\n\r");
	                NRF_CE_LOW;
	                SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, MAX_RT);
                    SPI_SendByte(CMD_TX_FLUSH);
                    State = INIT_RX;
	            }
	            else if (ret & RX_DR)
	            {
	                // Data was received, read payload, clear interrupt flag, check for more data
	                do
	                {
	                    SPI_GetBuffer(CMD_RX_READ, retBuf, PAYLOAD_WIDTH);
                        SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, RX_DR);
                        UART_SendString("GET-RX:");
                        UART_SendBuffer(retBuf,PAYLOAD_WIDTH);
                        UART_SendString("\n\r");

	                    ret = SPI_SendCommand(CMD_REG_READ + REG_FIFO_STATUS, DUMMY_BYTE);
	                } while (!(ret & RX_FIFO_EMPTY));
	            }
	        }
	        break;
	    case INIT:
	        NRF_CE_LOW;
	        // Write TX address to TX address register
            SPI_SendBuffer( CMD_REG_WRITE + REG_TX_ADDR, NRF_TX_ADDR,
                            NRF_TX_ADDR_WIDTH, NULL);
            // Write TX address to data pipe 0 address register, ignore return data
            SPI_SendBuffer( CMD_REG_WRITE + REG_RX_ADDR_P0, NRF_TX_ADDR,
                            NRF_TX_ADDR_WIDTH, NULL);
            // Enable auto ack on data pipe 0
            SPI_SendCommand(CMD_REG_WRITE + REG_ENAA, ENAA_P0);
            // Enable data pipe 0
            SPI_SendCommand(CMD_REG_WRITE + REG_EN_RXADDR, ERX_P0);
            // Set the retry rate, Wait 500+86uS (0x10) and 10 retries (0x0A)
            SPI_SendCommand(CMD_REG_WRITE + REG_SETUP_RETR, 0x10 + 0x0A);
            // Set the RF channel
            SPI_SendCommand(CMD_REG_WRITE + REG_RF_CHAN, NRF_RF_CHANNEL);
            // Set the payload width for receiving
            SPI_SendCommand(CMD_REG_WRITE + REG_RX_PW_P0, PAYLOAD_WIDTH);
            // Set power and speed
            SPI_SendCommand(CMD_REG_WRITE + REG_RF_SETUP,
                            RF_PWR_0dBm + RF_DR_2Mbps + LNA_HCURR);
            // Clear out status flags, clear buffers
            SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, MAX_RT + RX_DR + TX_DS);
            SPI_SendByte(CMD_TX_FLUSH);
            SPI_SendByte(CMD_RX_FLUSH);
            // Power up, primary receiver, CRC encoding - 2 bytes
            SPI_SendCommand(CMD_REG_WRITE + REG_CONFIG,
                            PRIM_RX + PWR_UP + EN_CRC + CRCO);
            NRF_CE_HIGH;
            UART_SendString("Init.\n\r");
	        State = IDLE;
	        break;
	    case TX_SEND:
	        NRF_CE_LOW;
	        index = 0;
	        UART_SendString("Send:");
	        do
	        {
	            if(UART_DataReady == TRUE)
	            {
	                UART_DataReady = FALSE;
	                UART_SendByte(UART_DataRX); // Echo data back.
	                if (UART_DataRX == '\r')
	                {
	                    break;
	                }
	                Payload[index] = UART_DataRX;
	                index ++;
	            }
	        } while (index < PAYLOAD_WIDTH);

	        // Null the rest of the buffer
	        for(; index < PAYLOAD_WIDTH; index++)
	        {
	            Payload[index] = '\0';
	        }

            SPI_SendBuffer( CMD_TX_WRITE, Payload, PAYLOAD_WIDTH, NULL);
            // Power up, primary transmitter, CRC encoding - 2 bytes
            SPI_SendCommand(CMD_REG_WRITE + REG_CONFIG,
                            PRIM_TX + PWR_UP + EN_CRC + CRCO);
            UART_SendString("\tConf:PTX\n\r");
	        NRF_CE_HIGH;
	        State = IDLE;
	        break;
	    case INIT_RX:
	        NRF_CE_LOW;
	        SPI_SendCommand(CMD_REG_WRITE + REG_CONFIG,
	                        PRIM_RX + PWR_UP + EN_CRC + CRCO);
	        UART_SendString("Conf:PRX\n\r");
	        NRF_CE_HIGH;
	        State = IDLE;
	        break;
	    case FIFO_STATUS:
	        ret = SPI_SendCommand(CMD_REG_READ + REG_FIFO_STATUS, DUMMY_BYTE);
	        UART_SendString("FIFO:");
	        UART_SendHexByte(ret);
	        UART_SendString("\n\r");
	        State = IDLE;
	        break;
	    case RX_STATUS:
	        ret = SPI_SendCommand(CMD_REG_READ + REG_RX_PW_P0, DUMMY_BYTE);
	        UART_SendString("RX-STAT:");
	        UART_SendHexByte(ret);
	        UART_SendString("\n\r");
	        State = IDLE;
	        break;
	    case RX_READ:
	        SPI_GetBuffer(CMD_RX_READ, retBuf, PAYLOAD_WIDTH);  // Get the data.
	        SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, RX_DR); // Clear the interrupt for for RX
	        UART_SendString("GET-RX:");
	        UART_SendBuffer(retBuf,PAYLOAD_WIDTH);
	        UART_SendString("\n\r");
	        State = IDLE;
	        break;
	    case STATUS:            // Get the status register
	        ret = SPI_SendCommand(CMD_REG_READ + REG_STATUS, DUMMY_BYTE);
	        UART_SendString("STAT:");
	        UART_SendHexByte(ret);
	        UART_SendString("\n\r");
	        State = IDLE;
	        break;
	    case CLEAR:
	        NRF_CE_LOW;
	        SPI_SendCommand(CMD_REG_WRITE + REG_STATUS, MAX_RT + RX_DR + TX_DS); // Clear out status stuff
	        SPI_SendByte(CMD_TX_FLUSH);
	        SPI_SendByte(CMD_RX_FLUSH);
	        UART_SendString("FLUSH\n\r");
	        State = IDLE;
	        break;
	    }

	    //TODO: Change the status LED to reflect what is actually happening,
	    // such as on when there is data on the UART or we are actively
	    // communicating with the wireless module.
		// Blink the status LED for debug.
		stat ^= 0x01;
		DEVIO_StatusLED_Set(stat);
	}

}
