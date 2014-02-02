#include "wifi_link.h"

#define PAYLOAD_WIDTH 5                     // size of an RX/TX payload.

void nrf24_init(void) {
	NRF_CE_LOW;
	// Write TX address to TX address register
	SPI_SendBuffer(CMD_REG_WRITE + REG_TX_ADDR, NRF_TX_ADDR, NRF_TX_ADDR_WIDTH,
			NULL );
	// Write TX address to data pipe 0 address register, ignore return data
	SPI_SendBuffer(CMD_REG_WRITE + REG_RX_ADDR_P0, NRF_TX_ADDR,
			NRF_TX_ADDR_WIDTH, NULL );
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
}

void RX_init(void) {
	NRF_CE_LOW;
	SPI_SendCommand(CMD_REG_WRITE + REG_CONFIG,
			PRIM_RX + PWR_UP + EN_CRC + CRCO);
	NRF_CE_HIGH;
}
