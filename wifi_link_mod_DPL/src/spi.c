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
/* Hardware of software SPI routine select. The software implementation is
 * only for debugging, as the hardware routine is working fine now. */
//#define SOFT_SPI

#ifdef SOFT_SPI
    #define SCK_LOW         (P1OUT &= ~DEVIO_P1OUT_SCK)
    #define SCK_HIGH        (P1OUT |= DEVIO_P1OUT_SCK)
    #define DELAY_LONG      2
    #define DELAY_SHORT     0

    void SendDelay(uint8_t lenght)
    {
        static volatile uint16_t delay;
        for(delay = 0; delay < lenght; delay++)
        {
        }
    }
#endif


/******************************************************************************
 * Global functions
 *****************************************************************************/


/*-----------------------------------------------------------------------------
 * Function name:   SPI_Init
 * Description:     Initializes the SPI routines on USCIB0
 * Parameters:      NONE
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void SPI_Init(void)
{
    #ifndef SOFT_SPI
        P1SEL |= BIT5 + BIT6 + BIT7;
        P1SEL2 |= BIT5 + BIT6 + BIT7;

        UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master

        // UCMSB Most significant bit first
        // UCCKPH Accept data on a rising edge.
        // UCMST Master Mode
        // UCSYNC Synchronous mode

        UCB0CTL1 |= UCSSEL_2;           // SMCLK
        #ifndef FAST_CLOCK
                UCB0BR0 |= 0x0A;                 // 1MHZ / 10 = 100khz SCLK
                UCB0BR1 = 0x00;                    // 1MHZ / 10 = 100khz SCLK
        #else
                UCB0BR0 |= 0x04;                 // 4 MHZ
                UCB0BR1 = 0x00;                    //
        #endif
        UCB0CTL1 &= ~UCSWRST;           // **Initialize USCI state machine**

        NRF_SPI_DISABLE;
    #else
        NRF_SPI_DISABLE;
        SCK_LOW;
    #endif
}


/*-----------------------------------------------------------------------------
 * Function name:   SPI_SendRawByte
 * Description:     Sends a byte out on the SPI lines. This function does not
 *                  modify the SPI select line
 * Parameters:      data - data byte to send
 * Returns:         byte received while sending out data
 *---------------------------------------------------------------------------*/
uint8_t SPI_SendRawByte(uint8_t data)
{
    #ifndef SOFT_SPI
        while (!(IFG2 & UCA0TXIFG));
        UCB0TXBUF = data;               // Send Data

        while (!(IFG2 & UCB0RXIFG));    // Wait to receive a byte
        return UCB0RXBUF;
    #else
        uint8_t i = 0;
        uint8_t bit = 0;
        uint8_t ret = 0;

        // Start with the clock low
        SCK_LOW;
        SendDelay(DELAY_SHORT);

        // Send bits
        // LSByte to MSByte, MSBit in each byte first
        for(i = 0; i < 8; i++)
        {
            bit = (data >> (7 - i)) & ( 0x01 );
            if(bit != 0)
            {
                P1OUT |= DEVIO_P1OUT_MOSI;
            } else
            {
                P1OUT &= ~DEVIO_P1OUT_MOSI;
            }

            SendDelay(DELAY_LONG);
            SCK_HIGH;

            SendDelay(DELAY_LONG);
            ret |= (((P1IN & DEVIO_P1IN_MISO)  << 1) >>  i);
            SCK_LOW;

            SendDelay(DELAY_SHORT);
        }

        P1OUT &= ~DEVIO_P1OUT_MOSI;
        SendDelay(DELAY_LONG);
        return ret;
    #endif
}


/*-----------------------------------------------------------------------------
 * Function name:   SPI_SendByte
 * Description:     Sends a byte out on the SPI lines. This function does
 *                  modify the SPI select line.
 * Parameters:      data - data byte to send
 * Returns:         byte received while sending out data
 *---------------------------------------------------------------------------*/
uint8_t SPI_SendByte(uint8_t data)
{
    uint8_t ret;

    NRF_SPI_ENABLE;
    ret = SPI_SendRawByte(data);
    NRF_SPI_DISABLE;

    return ret;
}


/*-----------------------------------------------------------------------------
 * Function name:   SPI_SendCommand
 * Description:     Sends out a command byte, followed by a data byte. This
 *                  function does modify the SPI select line.
 * Parameters:      data - data byte to send
 * Returns:         byte received while sending out the data byte
 *---------------------------------------------------------------------------*/
uint8_t SPI_SendCommand(uint8_t command, uint8_t data)
{
    uint8_t ret;

    NRF_SPI_ENABLE;
    (void) SPI_SendRawByte(command);
    ret = SPI_SendRawByte(data);
    NRF_SPI_DISABLE;

    return ret;
}


/*-----------------------------------------------------------------------------
 * Function name:   SPI_SendBuffer
 * Description:     Shifts out a buffer on the SPI lines. Command byte first,
 *                  followed by buffer[0], then buffer[1], ....
 *                  The first byte returned is the received byte from the
 *                  command, followed by the results of the buffer sending.
 * Parameters:      command - First byte to send, the command byte.
 *                  buffer - Pointer to the buffer to send out.
 *                  length - Number of bytes in the buffer to send outl
 *                  ret - Return buffer pointer. This can either be NULL, or it
 *                      needs to be length +1.
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void SPI_SendBuffer(uint8_t command, uint8_t* buffer, uint8_t length, uint8_t* ret)
{
    uint8_t i;
    uint8_t retByte;

    NRF_SPI_ENABLE;
    retByte = SPI_SendRawByte(command); // Send first byte, the command
    if(ret != NULL)
    {
        ret[0] = retByte;
    }

    for(i = 0; i < length; i++)     // Send buffer
    {
        retByte = SPI_SendRawByte(buffer[i]);
        if(ret != NULL)
        {
            ret[i +1] = retByte;
        }
    }

    NRF_SPI_DISABLE;
}


/*-----------------------------------------------------------------------------
 * Function name:   SPI_GetBuffer
 * Description:     Sends out a command on the SPI lines, followed by num_bytes
 *                  dummy bytes, and returns the received bytes. This is useful
 *                  for reading large blocks of memory on the target device.
 *                  Returned result does not include the byte shifted in from
 *                  the command byte.
 * Parameters:      command - First byte to send, the command byte.
 *                  ret - Return buffer pointer. This is where the results are
 *                      stored. It must be at least num_bytes large.
 *                  num_bytes - Number of bytes to shift in.
 * Returns:         NONE
 *---------------------------------------------------------------------------*/
void SPI_GetBuffer(uint8_t command, uint8_t* ret, uint8_t num_bytes)
{
    uint8_t i;

    NRF_SPI_ENABLE;
    (void) SPI_SendRawByte(command);

    for(i = 0; i < num_bytes; i++)
    {
       ret[i] = SPI_SendRawByte(DUMMY_BYTE);
    }
    NRF_SPI_DISABLE;
}
