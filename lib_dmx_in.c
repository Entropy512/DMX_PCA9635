/**** A P P L I C A T I O N   N O T E   ************************************
*
* Title			: DMX512 reception library
* Version		: v1.3
* Last updated	: 13.04.09
* Target		: Transceiver Rev.3.01 [ATmega8515]
* Clock			: 8MHz, 16MHz
*
* written by hendrik hoelscher, www.hoelscher-hi.de
***************************************************************************
 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either version2 of 
 the License, or (at your option) any later version. 

 This program is distributed in the hope that it will be useful, 
 but WITHOUT ANY WARRANTY; without even the implied warranty of 
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 General Public License for more details. 

 If you have no copy of the GNU General Public License, write to the 
 Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 

 For other license models, please contact the author.

;***************************************************************************/

#include "lib_dmx_in.h"

// ********************* local definitions *********************

enum {IDLE, BREAK, STARTB, STARTADR};			//DMX states

volatile uint8_t 	 gDmxState;


// *************** DMX Reception Initialisation ****************
void init_DMX_RX(void)
{
#ifdef USE_DIP
  DDRC   = 0;										//set up DIPs
  PORTC  = 0xFF;
  DDRE  &= ~((1<<PE2)|(1<<PE1));
  PORTE |=   (1<<PE2)|(1<<PE1);
#endif
  
  UBRR0H  = 0;										//enable reception
  UBRR0L  = 3;						//250kbaud, 8N2
  UCSR0C  = _BV(UCSZ01)|_BV(UCSZ00)|_BV(USBS0);
  UCSR0B  = _BV(RXEN0)|_BV(RXCIE0);
  gDmxState= IDLE;
}



// ************* get DMX start address **************
void get_dips(void)
{
#ifdef USE_DIP
uint16_t Temp= (0xFF-PINC);						//invert DIP state
if (!(PINE &(1<<2)))
	{
	Temp += 256;								//9th bit
	}
if (Temp != 0)
	{
	DmxAddress= Temp;
	if (!(UCSR0B &_BV(RXCIE0)))					//if receiver was disabled -> enable and wait for break
		{
		gDmxState= IDLE;
		UCSR0B |= _BV(RXCIE0);
		}
	}
else
	{
	  UCSR0B &= ~_BV(RXCIE0);						//disable Receiver if start address = 0
	uint8_t i;
	for (i=0; i<sizeof(DmxRxField); i++)
		{
		DmxRxField[i]= 0;
		}
	}
#endif 
}



// *************** DMX Reception ISR ****************
ISR(USART_RX_vect)
{
  static  uint16_t DmxCount;
  uint8_t USARTstate = UCSR0A;				//get state before data!
  uint8_t DmxByte = UDR0;				//get data
  uint8_t DmxState = gDmxState;			//just load once from SRAM to increase speed



  if (USARTstate &_BV(FE0))						//check for break
    {
      UCSR0A &= ~_BV(FE0);							//reset flag (necessary for simulation in AVR Studio)
      DmxCount =  DmxAddress;						//reset channel counter (count channels before start address)
      gDmxState= BREAK;
     }
  
  else if (DmxState == BREAK)
    {
      if(DmxByte == 0)
	
	{
	  gDmxState= STARTB;		//normal start code detected
	}
      else
	{
	  gDmxState= IDLE;

	}

    }
  
  else if (DmxState == STARTB)
    {
      if (--DmxCount == 0)						//start address reached?
	{
	  DmxCount= 1;							//set up counter for required channels
	  DmxRxField[0]= DmxByte;					//get 1st DMX channel of device
	  gDmxState= STARTADR;
	}

    }
  
  else if (DmxState == STARTADR)
    {
      DmxRxField[DmxCount++]= DmxByte;			//get channel
      if (DmxCount >= sizeof(DmxRxField)) 		//all ch received?
	{
	  gDmxState= IDLE;						//wait for next break
	}
    }							
}

