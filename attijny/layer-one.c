/*
	SELFPRGEN = [ ]
	DWEN = [ ]
	EESAVE = [X]
	SPIEN = [X]
	WDTON = [ ]
	BODLEVEL = DISABLED
	RSTDISBL = [ ]
	CKDIV8 = [ ]
	CKOUT = [ ]
	SUT_CKSEL = INTRCOSC_8MHZ_14CK_0MS

	EXTENDED = 0xFF (valid)
	HIGH = 0x9F (valid)
	LOW = 0xC4 (valid)
*/

#define F_CPU					(8000000UL)

#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>
#include <stdlib.h>

#undef RGB
#define RGB(b,r,g)				((uint32_t)(((uint8_t)(r)|((uint16_t)((uint8_t)(g))<<8))|(((uint32_t)(uint8_t)(b))<<16)))

#define bit_get(p,m)			((p) & (m))
#define bit_set(p,m)			((p) |= (m))
#define bit_clear(p,m)			((p) &= ~(m))
#define bit_flip(p,m)			((p) ^= (m))

// PORTD
#define RED_LED_0				(2)
#define RED_LED_1				(3)
#define RED_LED_2				(4)
#define RED_LED_3				(5)
#define RED_LED_4				(6)

// PORTB
#define RED_LED_5				(0)

#define NUM_RGB_LEDS			(14)
#define	StripLights_MAX_X		(14)

#define shortdelay(); 			asm("nop\n\tnop\n\t");

#define TIMER1_PRESCALE_1 		(1)
#define TIMER1_PRESCALE_8 		(2)
#define TIMER1_PRESCALE_64 		(3)
#define TIMER1_PRESCALE_256		(4)
#define TIMER1_PRESCALE_1024	(5)

#define MIN_OF(x,y)				((x) < (y) ? (x) : (y))
#define MAX_OF(x,y)				((x) > (y) ? (x) : (y))

#define GET_BIT(a,n)			((a >> n) & 1)
#define SET_BIT(a,n)			(a|=(1<<n))
#define CLR_BIT(a,n)			(a&=~(1<<n))
#define SWITCH_BIT(a,n)			(a^=(1<<n))
#define IS_BIT_SET(a,n)			(a & (1<<n))
#define IS_BIT_CLR(a,n)			(~(a & (1<<n)))

#define Red(c)					((uint8_t)((c >> 16) & 0xFF))
#define Green(c)				((uint8_t)((c >> 8)  & 0xFF))
#define Blue(c)					((uint8_t)( c        & 0xFF))

// 		PORTB,DDRB,PIN
#define HEX__(n)				0x##n##LU

/* 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0)	\
	+((x&0x000000F0LU)?2:0)		\
	+((x&0x00000F00LU)?4:0)		\
	+((x&0x0000F000LU)?8:0)		\
	+((x&0x000F0000LU)?16:0)	\
	+((x&0x00F00000LU)?32:0)	\
	+((x&0x0F000000LU)?64:0)	\
	+((x&0xF0000000LU)?128:0)

/* *** user macros ***/

/* for upto 8-bit binary constants */
#define B8(d)					((unsigned char)B8__(HEX__(d)))

/* for upto 16-bit binary constants, MSB first */
#define B16(dmsb,dlsb)			(((unsigned short)B8(dmsb)<<	+ B8(dlsb)))

/* for upto 32-bit binary constants, MSB first */
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24)	 \
	+ ((unsigned long)B8(db2)<<16) \
	+ ((unsigned long)B8(db3)<<	 \
	+ B8(dlsb))

#define D6_ON					( 1 << 0 )
#define DD_ON					( 1 << 1 )
#define BO_ON					( D6_ON | DD_ON )

void send ( const void *values, uint16_t array_size, uint8_t bit );

volatile uint8_t counter = 0;// interrupt needs volatile variable
volatile uint8_t counter2 = 0;// interrupt needs volatile variable , counter2 for effects

static uint8_t  rotate = 1;

unsigned char data[NUM_RGB_LEDS * 3] ;

// length of rainbow effect
uint8_t effectLength = 1;

//rotate left, barrel shift
uint8_t inline rotl32a ( uint8_t x, uint8_t n )
{
    return ( x << n ) | ( x >> ( 8 - n ) );
}

////////////////////////////////////////////////////////////////////////////////////////////

void StripLights_Pixel ( uint8_t x, uint32_t color )
{	
    if ( x >= 0 && x < StripLights_MAX_X  ) {
        data[ ( x * 3 )     ] = (color       & 0xFF) >> 3 ;
        data[ ( x * 3 ) + 1 ] = (color >>  8 & 0xFF) >> 3 ;
        data[ ( x * 3 ) + 2 ] = (color >> 16 & 0xFF) >> 3 ;
    }
}

void inline StripLights_Trigger ( uint8_t t )
{
    send ( &data,  NUM_RGB_LEDS, 1 );
}

void inline StripLights_MemClear ( uint32_t d )
{
    memset ( &data[0], d, sizeof ( data ) );
}

////////////////////////////////////////////////////////////////////////////////////////////

static  uint8_t mode = 0;

ISR ( TIMER0_OVF_vect )
{
    if ( ++counter > 30 ) {

        // second counter to control rgb rainbow effect
        if ( ++counter2 > 60 ) {

            effectLength++;

            counter2 = 0;
            SWITCH_BIT ( PORTB , 3 );

            mode = rand() % 3;
        }

        // rotate around
        rotate = rotl32a ( rotate , 1 );

        // reset rotate when we hit this bit position.
        if ( rotate == B8 ( 1000000 ) ) { rotate  = 1;}

        if ( bit_get ( rotate, _BV ( 0 ) ) ) {
            bit_set ( PORTD, _BV ( RED_LED_0 ) );
        } else {
            bit_clear ( PORTD, _BV ( RED_LED_0 ) );
        }

        if ( bit_get ( rotate, _BV ( 1 ) ) ) {
            bit_set ( PORTD, _BV ( RED_LED_1 ) );
        } else {
            bit_clear ( PORTD, _BV ( RED_LED_1 ) );
        }

        if ( bit_get ( rotate, _BV ( 2 ) ) ) {
            bit_set ( PORTD, _BV ( RED_LED_2 ) );
        } else {
            bit_clear ( PORTD, _BV ( RED_LED_2 ) );
        }

        if ( bit_get ( rotate, _BV ( 3 ) ) ) {
            bit_set ( PORTD, _BV ( RED_LED_3 ) );
        } else {
            bit_clear ( PORTD, _BV ( RED_LED_3 ) );
        }

        if ( bit_get ( rotate, _BV ( 4 ) ) ) {
            bit_set ( PORTD, _BV ( RED_LED_4 ) );
        } else {
            bit_clear ( PORTD, _BV ( RED_LED_4 ) );
        }

        if ( bit_get ( rotate, _BV ( 5 ) ) ) {
            bit_set ( PORTB, _BV ( RED_LED_5 ) );
        } else {
            bit_clear ( PORTB, _BV ( RED_LED_5 ) );
        }

        SWITCH_BIT ( PORTB , 2 );
 
		//reset the counter
        counter = 0;
    }
}

uint32_t inline newColor ( uint8_t r, uint8_t g, uint8_t b )
{
    return ( ( ( uint32_t ) ( r ) << 16 ) | ( ( uint32_t ) ( g ) <<  8 ) | ( b ) );
}

uint32_t Wheel ( uint8_t WheelPos )
{
    WheelPos = 255 - WheelPos;

    if ( WheelPos < 85 ) {
        return newColor ( 255 - WheelPos * 3, 0, WheelPos * 3 );
    }

    if ( WheelPos < 170 ) {
        WheelPos -= 85;
        return newColor ( 0, WheelPos * 3, 255 - WheelPos * 3 );
    }

    WheelPos -= 170;
    return newColor ( WheelPos * 3, 255 - WheelPos * 3, 0 );
}

void Rainbow ( uint32_t length , uint32_t effectLengthR )
{
    static uint16_t  x = 0;

    x += 1;

    if ( x == ( 256 * 5 ) ) {
        x = 0;
    }

    for ( uint8_t i = 0 ; i < length ; i++ ) {

        uint32_t color = Wheel ( ( ( i * 256 ) / effectLengthR + x ) & 0xFF );

        StripLights_Pixel ( i, color );
    }
}

static int8_t pos = 0, dir = 1; // Position, direction of "eye"

void Larson ( uint8_t count  )
{
    for ( uint8_t j = 0; j < count; j++ ) {

        // Draw 5 pixels centered on pos.  setPixelColor() will clip any
        // pixels off the ends of the strip, we don't need to watch for that.
        StripLights_Pixel ( pos - 2, 	RGB ( 0x10, 0x00, 0x00 ) ); // Dark red
        StripLights_Pixel ( pos - 1, 	RGB ( 0x80, 0x00, 0x00 ) ); // Medium red
        StripLights_Pixel ( pos, 		RGB ( 0xFF, 0x30, 0x00 ) ); // Center pixel is brightest
        StripLights_Pixel ( pos + 1, 	RGB ( 0x80, 0x00, 0x00 ) ); // Medium red
        StripLights_Pixel ( pos + 2, 	RGB ( 0x10, 0x00, 0x00 ) ); // Dark red

        StripLights_Trigger ( 1 );

        _delay_ms ( 60 );

        // Rather than being sneaky and erasing just the tail pixel,
        // it's easier to erase it all and draw a new one next time.
        //StripLights_MemClear ( 0 );

        // Using this instead of the memclear would leave a trail behind it.
        //FadeStrip(StripLights_MIN_X, StripLights_MAX_X, 6);

        // Bounce off ends of strip
        pos += dir;

        if ( pos < 0 ) {
            pos = 1;
            dir = -dir;

        } else
            if ( pos >= StripLights_MAX_X ) {
                pos = StripLights_MAX_X - 2;
                dir = -dir;
            }
    }
}

int main ( void )
{
    ACSR = 0;

    PORTD = 0;
    PORTB = 0;

    // setup leds and rgb leds
    DDRD = _BV ( 1 ) | _BV ( 2  ) | _BV ( 3  ) | _BV ( 4  ) | _BV ( 5  ) | _BV ( 6  );
    DDRB = _BV ( 0 ) | _BV ( 1 ) | _BV ( 2  ) | _BV ( 3  ) ;

    cli();

    // set prescale timer
    TCCR0B |= ( 1 << CS01 ) | ( 1 << CS00 );

    // enable timer overflow interrupt
    TIMSK |= 1 << TOIE0; // left shift 1 to TOIE0 and OR with TIMSK

    sei(); //start timer

    while ( 1 ) {
		
        switch ( mode ) {

            case 0:
                send ( &data,  NUM_RGB_LEDS, 1 );
                Rainbow ( NUM_RGB_LEDS, effectLength );
                break;

            case 1:
                Larson ( 100 );
                _delay_ms ( 100 );
                break;

            case 2:
                send ( &data,  NUM_RGB_LEDS, 1 );
                Rainbow ( NUM_RGB_LEDS, 1 );
                break;
        }
    }
}

// this is very dependent on xtal clock speed, see fuse settings above

//
// Copyright (c) 2013 Danny Havenith
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

/**
 * Library for bit-banging data to WS2811 led controllers.
 * This file contains a definition of the send() function for 8 Mhz controllers.
 */

#define WS2811_PORT PORTD

typedef struct rgb_t {
    uint8_t r, g, b;
} rgb;

/**
* This function sends the RGB - data in an array of rgb structs through
* the given io - pin.
* The port is determined by the macro WS2811_PORT, but the actual pin to
* be used is an argument to this function. This allows a single instance of this function
* to control up to 8 separate channels.
*/
void send ( const void *values, uint16_t array_size, uint8_t bit )
{
    const uint8_t mask = _BV ( bit );
    uint8_t low_val = WS2811_PORT & ( ~mask );
    uint8_t high_val = WS2811_PORT | mask;
    uint16_t size = array_size * sizeof ( rgb ); // size in bytes


// reset the controllers by pulling the data line low
    uint8_t bitcount = 7;
    WS2811_PORT = low_val;
    _delay_loop_1 ( 107 ); // at 3 clocks per iteration, this is 320 ticks or 40us at 8Mhz

// The labels in this piece of assembly code aren't very explanatory. The real documentation
// of this code can be found in the spreadsheet ws2811@8Mhz.ods
// A hint if you still want to follow the code below:
// The code will send bits from most significant to least significant (bits 7 to 0) and consists
// of two variants of the main loop:
// 1) The code for a regular bit (i.e. bits 7-1) starts at label s00 with the current bit value
// already in the carry flag and it jumps halfway back to label cont06.
// 2) For a part of bit 1 and all of bit 0, the code falls through (after the skip03 label)
// where the code needs to fork in a "transmit 0" and "transmit 1" branch. This is because
// there's extra work to be done (loading the next byte), which needs to be carefully placed
// in the time between toggling the output pins.
//
// The two-digit suffix of labels shows the "phase" of the signal at the time
// of the execution, 00 being the first clock tick of the bit and 09 being the last.
    asm volatile (
        "start:  LDI %[bits], 7                          \n" // start code, load bit count
        "        LD __tmp_reg__, %a[dataptr]+            \n" // fetch first byte
        "cont06: NOP                                     \n"
        "cont07: NOP                                     \n"
        "        OUT %[portout], %[downreg]              \n" // Force line down, even if it already was down
        "cont09: LSL __tmp_reg__                         \n" // Load next bit into carry flag.
        "s00:    OUT %[portout], %[upreg]                \n" // Start of bit, bit value is in carry flag
        "        BRCS skip03                             \n" // only lower the line if the bit...
        "        OUT %[portout], %[downreg]              \n" // ...in the carry flag was zero.
        "skip03: SUBI %[bits], 1                         \n" // Decrease bit count...
        "        BRNE cont06                             \n" // ...and loop if not zero
        "        LSL __tmp_reg__                         \n" // Load the last bit into the carry flag
        "        BRCC Lx008                              \n" // Jump if last bit is zero
        "        LDI %[bits], 7                          \n" // Reset bit counter to 7
        "        OUT %[portout], %[downreg]              \n" // Force line down, even if it already was down
        "        NOP                                     \n"
        "        OUT %[portout], %[upreg]                \n" // Start of last bit of byte, which is 1
        "        SBIW %[bytes], 1                        \n" // Decrease byte count
        "        LD __tmp_reg__, %a[dataptr]+            \n" // Load next byte
        "        BRNE cont07                             \n" // Loop if byte count is not zero
        "        RJMP brk18                              \n" // Byte count is zero, jump to the end
        "Lx008:  OUT %[portout], %[downreg]              \n" // Last bit is zero
        "        LDI %[bits], 7                          \n" // Reset bit counter to 7
        "        OUT %[portout], %[upreg]                \n" // Start of last bit of byte, which is 0
        "        NOP                                     \n"
        "        OUT %[portout], %[downreg]              \n" // We know we're transmitting a 0
        "        SBIW %[bytes], 1                        \n" // Decrease byte count
        "        LD __tmp_reg__, %a[dataptr]+            \n"
        "        BRNE cont09                             \n" // Loop if byte count is not zero
        "brk18:  OUT %[portout], %[downreg]              \n"
        "                                                \n" // used to be a NOP here, but returning from the function takes long enough
        "                                                \n" // We're done.
        : /* no output */
        : /* inputs */
        [dataptr] "e" ( values ), 	// pointer to grb values
        [upreg]   "r" ( high_val ),	// register that contains the "up" value for the output port (constant)
        [downreg] "r" ( low_val ),	// register that contains the "down" value for the output port (constant)
        [bytes]   "w" ( size ),		// number of bytes to send
        [bits]    "d" ( bitcount ),     // number of bits/2
        [portout] "I" ( _SFR_IO_ADDR ( WS2811_PORT ) ) // The port to use
    );
}
