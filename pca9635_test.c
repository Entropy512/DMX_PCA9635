/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <joerg@FreeBSD.ORG> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.        Joerg Wunsch
 * ----------------------------------------------------------------------------
 *
 * Simple AVR demonstration.  Controls a LED that can be directly
 * connected from OC1/OC1A to GND.  The brightness of the LED is
 * controlled with the PWM.  After each period of the PWM, the PWM
 * value is either incremented or decremented, that's all.
 *
 * $Id: demo.c,v 1.9 2006/01/05 21:30:10 joerg_wunsch Exp $
 */

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "i2cmaster.h"
#include "lib_dmx_in.h"

#define F_CPU 16000000UL


ISR (TIMER0_COMPA_vect)
{
  /*
    This ISR no longer does anything.  Going to keep it around - likely to be used for a program mode state machine.
  */

}

void
ioinit (void)			/* Note [6] */
{
  cli();

  /* set up timer 0 to generate 1 ms interrupts */
  /* clear timer on compare mode for timer 0 */
  TCCR0A = _BV(WGM01);

  /* Divide by 64 prescaler.  With F_CPU at 16 MHz, this timer will ht 250 after 1 ms */
  TCCR0B = _BV(CS01) |_BV(CS00);

  /* Divide by 256 prescaler */
  //  TCCR0B = _BV(CS02) |_BV(CS00);

  /* Set output compare register to 250, so a compare interrupt will trigger every ms */
  //  OCR0A = 250;
  OCR0A = 250;

  /* Set timer 1 compare interrupt to enabled */

  /*
  TIMSK0 = _BV(OCIE0A);
  */

  DmxAddress = 10;

  init_DMX_RX();

  i2c_init();

  sei ();
}

void
init9635 (void)			/* Note [6] */
{
  i2c_start((0x60<<1)+I2C_WRITE);
  
  //Autoincrement ALL registers, start at reg 0 - we're initting
  i2c_write(0x80);
  
  //Reg 0x00 = MODE1 - set to 0x80 - autoincrement enabled (this is readonly?), do not respond to subaddresses or allcall
  i2c_write(0x80);
  
  //Reg 0x01 = MODE2 - set to 0x02 - noninverted logic state, open-drain
  i2c_write(0x02);
  
  //Reg 0x02-0x11 - PWM0-15 - LED brightnesses, start at low brightness (0x20)
  
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  i2c_write(0x20);
  
  //Reg 0x12 - Group PWM - should not matter
  i2c_write(0xff);

  //Reg 0x13 - Group Freq - should not matter
  i2c_write(0x00);

  //Reg 0x14-0x17 - LED Output State Control - all 0xAA (output state for each LED = 0x2)
  i2c_write(0xaa);
  i2c_write(0xaa);
  i2c_write(0xaa);
  i2c_write(0xaa);

  //Reg 0x18-0x1b - Subaddressing stuff, doesn't matter, just stop the xfer
  i2c_stop();

  
}

int
main (void)
{
  uint8_t mychannel;

  /*
    Hardcode for now, change this to programmable
  */

  static i2c_addr = 0x60;

  ioinit ();
  
  init9635();


  /* loop forever, refresh I2C data as fast as possible */
  
  for (;;)			/* Note [7] */
    {  

      i2c_start((i2c_addr<<1)+I2C_WRITE);
      //0xA0 = autoincrement brightness, 0x02 = PWM00 reg
      i2c_write(0xA0 |(0x3));
      for(mychannel = 0; mychannel < 15; mychannel++)
	{
	  i2c_write(DmxRxField[mychannel]);
	}
      i2c_stop();
      
      
      
    }
  
  return (0);
}
