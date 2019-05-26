#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>


#define IS_BIT(a,p)    (a&(1<<p))    // test the a variable p bit
#define SET_BIT(a,p) a |=  (1<<p)    // set in a the p bit
#define CLR_BIT(a,p) a &= ~(1<<p)    // clear in a the p bit

#define LED0 		( 4 )
#define LED1 		( 2 )//2
#define LED2 		( 1 )//3
#define LED3 		( 0  )//4
#define LED4 		( 5 )//5

// byte two
#define LED5 		( 3 )
#define LED6 		( 2 )
#define LED7 		( 1 )
#define LED8 		( 0 )

#define shortdelay(); 			asm("nop\n\t" \
"nop\n\t");

#define TIMER1_PRESCALE_1 		(1)
#define TIMER1_PRESCALE_8 		(2)
#define TIMER1_PRESCALE_64 		(3)
#define TIMER1_PRESCALE_256		(4)
#define TIMER1_PRESCALE_1024	(5)



// NSL Cylon
#define B9__(x) ((x&0x0000000FLU)?LED0:0) \
+((x&0x000000F0LU)?LED1:0) \
+((x&0x00000F00LU)?LED2:0) \
+((x&0x0000F000LU)?LED3:0) \
+((x&0x000F0000LU)?LED4:0) \
+((x&0x00F00000LU)?LED5:0) \
+((x&0x0F000000LU)?LED6:0) \
+((x&0xF0000000LU)?LED7:0) \
+((x&0xF00000000LU)?LED8:0)

#define _B9(d) ((uint16_t)B9__(HEX__(d)))

#define B9(d) 0b##d

#define NUM_ELEM(x) (sizeof (x) / sizeof (*(x)))

uint16_t eepromWord __attribute__ ( ( section ( ".eeprom" ) ) );

static const uint16_t pattern2[] = {
	//0b1111111111111111,
	0b0001000000000000,
};

static const uint16_t pattern1[] = {
	//0b1111111111111111,
	0b0001000000000000,
	0b0000100000000000,
	0b0000010000000000,
	0b0000001000000000,
	0b0000000100000000,
	0b0000000010000000,
	0b0000000001000000,
	0b0000000000100000,
	0b0000000000010000,
	0b0000000000001000,
	0b0000000000000100,
	0b0000000000000010,
	0b0000000000000001,
	0b0000000000000010,
	0b0000000000000100,
	0b0000000000001000,
	0b0000000000010000,
	0b0000000000100000,
	0b0000000001000000,
	0b0000000010000000,
	0b0000000100000000,
	0b0000001000000000,
	0b0000010000000000,
	0b0000100000000000,
	0b0001000000000000,
	0b0010000000000000,
};

int main ( void )
{
	unsigned char index=0, mode = 0;

	//Initialization routine: Clear watchdog timer-- this can prevent several things from going wrong.
	MCUSR &= 0xF7;		//Clear WDRF Flag
	WDTCSR	= 0x18;		//Set stupid bits so we can clear timer...
	WDTCSR	= 0x00;

	//Data direction register: DDR's
	//Port A:outputs
	//Port B:  B3 is an input for switch rest are outputs.
	//Port D: outputs

	// setup mask for i/o direction
	DDRA = 255U;
	DDRB = 247;
	DDRD = 255U;

	// pullup for button
	PORTA = 0;	// Pull-up resistors enabled, PA0, PA1
	PORTB = (1<<3);	// Pull-up resistor enabled, PB3
	PORTD = 0;

	uint16_t tmp =1;

	// test all leds on
	while ( ( PINB & (1<<3)  ) == 0 ) {	// Check if Jumper 1, at location PA1 is shorted
		PORTB = 0xff;
		PORTD = 0xff;
		PORTA =  0xff;
	}

	// binary counter
	for( ;; ) {

		if ( IS_BIT ( tmp, 0 ) ) { SET_BIT ( PORTB, 4 ); }

		if ( IS_BIT ( tmp, 1 ) ) { SET_BIT ( PORTB, 2 ); }

		if ( IS_BIT ( tmp, 2 ) ) { SET_BIT ( PORTB, 1 ); }

		if ( IS_BIT ( tmp, 3 ) ) { SET_BIT ( PORTB, 0 ); }

		if ( IS_BIT ( tmp, 4 ) ) { SET_BIT ( PORTD, 6 ); }

		if ( IS_BIT ( tmp, 5 ) ) { SET_BIT ( PORTD, 5 ); }

		/////////////////////////////////////////////////

		if ( IS_BIT ( tmp, 6 ) ) { SET_BIT ( PORTD, 4 ); }

		if ( IS_BIT ( tmp, 7 ) ) { SET_BIT ( PORTD, 1 ); }

		if ( IS_BIT ( tmp, 8 ) ) { SET_BIT ( PORTA, 1 ); }

		if ( IS_BIT ( tmp, 9 ) ) { SET_BIT ( PORTA, 0 ); }

		if ( IS_BIT ( tmp, 10 ) ) { SET_BIT ( PORTD, 2 ); }

		if ( IS_BIT ( tmp, 11 ) ) { SET_BIT ( PORTD, 3 ); }
		

		while ( ( PINB & (1<<3)  ) == 0 ) {	// Check if Jumper 1, at location PA1 is shorted
			// Optional place to do something.  :)
			mode++;
		}
		


		switch ( mode ) {

			case 0:
			
				tmp = pattern1[index];
				//tmp+=1;
				index ++;
				if ( index == ( sizeof( pattern1)/2) -1 ) {
					index =0;
				}
				//_delay_ms(1500);
				break;

			case 1:
				tmp += 1;
				// clear all leds
				break;
			default :
				mode = 0;
				break;
		}
		
		_delay_ms(50);

		// reset port b back to an input
		PORTB = (1<<3);
		PORTD = 0;
		PORTA =  0;

	}

	// setup timer callback, add more code here
	TCCR1B = ( 1 << WGM12 ) | TIMER1_PRESCALE_1;
	OCR1A = ( uint16_t ) 800;

	TIMSK |= 1 << OCIE1A;   // Output Compare Interrupt Enable (timer 1, OCR1A)

	
	return 0;
}


void delay_ms ( uint16_t milliseconds )
{
	for ( ; milliseconds > 0; milliseconds-- ) {
		_delay_ms ( 1 );
	}
}

// this function is called when timer1 compare matches OCR1A

SIGNAL ( TIMER1_COMPA_vect  )
{

}
