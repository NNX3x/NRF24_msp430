/******************************************************************************
 * 
 * TODO: License and terms, NDA, etc...
 *
 * FILE_NAME_
 * FILE_FUNCTION / DESCRIPTION 
 *****************************************************************************/
 
#ifndef WIFI_LINK_H_
#define WIFI_LINK_H_

/******************************************************************************
 * System includes
 *****************************************************************************/
#include <msp430.h>
#include <stdint.h>

/******************************************************************************
 * Local includes
 *****************************************************************************/
#include "devio.h"
#include "NRF24L.h"
#include "uart.h"
#include "spi.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif
#ifndef NULL
    #define NULL ((void *)0)
#endif

#define FAST_CLOCK                          // Use 16MHz core clock instead of
                                            // the default 1MHz clock.

#endif /*WIFI_LINK_H_*/
