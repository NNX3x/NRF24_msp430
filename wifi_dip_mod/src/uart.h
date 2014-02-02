/******************************************************************************
 *
 * TODO: License and terms, NDA, etc...
 *
 * FILE_NAME_
 * FILE_FUNCTION / DESCRIPTION
 *****************************************************************************/

#ifndef UART_H_
#define UART_H_

/******************************************************************************
 * Global variables
 *****************************************************************************/

extern volatile uint8_t UART_DataReady;
extern volatile uint8_t UART_DataRX;


/******************************************************************************
 * Global functions
 *****************************************************************************/

void UART_Init(void);
void UART_SendByte(uint8_t data);
void UART_SendBuffer(uint8_t* buffer, uint8_t length);
void UART_SendString(const char* str);
void UART_SendHexByte(uint8_t data);

#endif /* UART_H_ */
