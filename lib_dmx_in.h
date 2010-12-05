#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

/* #define  USE_DIP */

#define  F_OSC (16000UL)		  		//oscillator freq. in kHz (typical 8MHz or 16MHz)


volatile uint8_t	 DmxRxField[16]; 		//array of DMX vals (raw)
volatile uint16_t	 DmxAddress;			//start address

extern void    init_DMX_RX(void);
extern void    get_dips(void);
