/******************************************************************************
 *
 * TODO: License and terms, NDA, etc...
 *
 * FILE_NAME_
 * FILE_FUNCTION / DESCRIPTION
 *****************************************************************************/

#ifndef SPI_H_
#define SPI_H_

/******************************************************************************
 * Global functions
 *****************************************************************************/

void SPI_Init(void);

uint8_t SPI_SendRawByte(uint8_t data);
uint8_t SPI_SendByte(uint8_t data);
uint8_t SPI_SendCommand(uint8_t command, uint8_t data);

void SPI_SendBuffer(uint8_t command, uint8_t* buffer, uint8_t length, uint8_t* ret);
void SPI_GetBuffer(uint8_t command, uint8_t* ret, uint8_t num_bytes);

#endif /*SPI_H_*/
