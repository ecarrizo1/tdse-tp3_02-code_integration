//=====[Libraries]=============================================================
//#include "mbed.h"
//#include "arm_book_lib.h"

#include "display.h"
#include "main.h"
#include <stdbool.h>
#include "board.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"
#include "systick.h"

/********************** arm_book Defines *******************************/
//#include "arm_book_lib.h"
// Functional states
#ifndef OFF
#define OFF    0
#endif
#ifndef ON
#define ON     ( !OFF )
#endif

// Electrical states
#ifndef LOW
#define LOW    0
#endif
#ifndef HIGH
#define HIGH   ( !LOW )
#endif

//=====[Declaration of private defines]========================================
#define DISPLAY_IR_CLEAR_DISPLAY   0b00000001
#define DISPLAY_IR_ENTRY_MODE_SET  0b00000100
#define DISPLAY_IR_DISPLAY_CONTROL 0b00001000
#define DISPLAY_IR_FUNCTION_SET    0b00100000
#define DISPLAY_IR_SET_DDRAM_ADDR  0b10000000

#define DISPLAY_IR_ENTRY_MODE_SET_INCREMENT 0b00000010
#define DISPLAY_IR_ENTRY_MODE_SET_DECREMENT 0b00000000
#define DISPLAY_IR_ENTRY_MODE_SET_SHIFT     0b00000001
#define DISPLAY_IR_ENTRY_MODE_SET_NO_SHIFT  0b00000000

#define DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_ON  0b00000100
#define DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_OFF 0b00000000
#define DISPLAY_IR_DISPLAY_CONTROL_CURSOR_ON   0b00000010
#define DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF  0b00000000
#define DISPLAY_IR_DISPLAY_CONTROL_BLINK_ON    0b00000001
#define DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF   0b00000000

#define DISPLAY_IR_FUNCTION_SET_8BITS    0b00010000
#define DISPLAY_IR_FUNCTION_SET_4BITS    0b00000000
#define DISPLAY_IR_FUNCTION_SET_2LINES   0b00001000
#define DISPLAY_IR_FUNCTION_SET_1LINE    0b00000000
#define DISPLAY_IR_FUNCTION_SET_5x10DOTS 0b00000100
#define DISPLAY_IR_FUNCTION_SET_5x8DOTS  0b00000000


#define DISPLAY_20x4_LINE1_FIRST_CHARACTER_ADDRESS 0
#define DISPLAY_20x4_LINE2_FIRST_CHARACTER_ADDRESS 64
#define DISPLAY_20x4_LINE3_FIRST_CHARACTER_ADDRESS 20
#define DISPLAY_20x4_LINE4_FIRST_CHARACTER_ADDRESS 84

#define DISPLAY_16x2_LINE1_FIRST_CHARACTER_ADDRESS 0x00
#define DISPLAY_16x2_LINE2_FIRST_CHARACTER_ADDRESS 0x40

#define DISPLAY_RS_INSTRUCTION 0
#define DISPLAY_RS_DATA        1

#define DISPLAY_RW_WRITE 0
#define DISPLAY_RW_READ  1

#define DISPLAY_PIN_RS  4
#define DISPLAY_PIN_RW  5
#define DISPLAY_PIN_EN  6
#define DISPLAY_PIN_D0  7
#define DISPLAY_PIN_D1  8
#define DISPLAY_PIN_D2  9
#define DISPLAY_PIN_D3 10
#define DISPLAY_PIN_D4 11
#define DISPLAY_PIN_D5 12
#define DISPLAY_PIN_D6 13
#define DISPLAY_PIN_D7 14

#define DISPLAY_DEL_37US	37ul
#define DISPLAY_DEL_01US	01ul

//=====[Declaration of private data types]=====================================

//=====[Declaration and initialization of public global objects]===============

/*
DigitalOut displayD0( D0 );
DigitalOut displayD1( D1 );
DigitalOut displayD2( D2 );
DigitalOut displayD3( D3 );
DigitalOut displayD4( D4 );
DigitalOut displayD5( D5 );
DigitalOut displayD6( D6 );
DigitalOut displayD7( D7 );
DigitalOut displayRs( D8 );
DigitalOut displayEn( D9 );
*/

//=====[Declaration and initialization of private global variables]============
static display_t display;
static bool initial8BitCommunicationIsCompleted;

//=====[Declarations (prototypes) of private functions]========================
static void displayPinWrite( uint8_t pinName, int value );
static void displayDataBusWrite( uint8_t dataByte );
static void displayCodeWrite( bool type, uint8_t dataBus );

//=====[Implementations of public functions]===================================
void displayInit( displayConnection_t connection )
{
    display.connection = connection;
    initial8BitCommunicationIsCompleted = false;

    HAL_Delay(50);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_FUNCTION_SET |
                      DISPLAY_IR_FUNCTION_SET_8BITS );
    HAL_Delay(5);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_FUNCTION_SET |
                      DISPLAY_IR_FUNCTION_SET_8BITS );

    systick_delay_us(DISPLAY_DEL_37US);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_FUNCTION_SET |
                      DISPLAY_IR_FUNCTION_SET_8BITS );

    systick_delay_us(DISPLAY_DEL_37US);

    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_FUNCTION_SET |
                              DISPLAY_IR_FUNCTION_SET_8BITS |
                              DISPLAY_IR_FUNCTION_SET_2LINES |
                              DISPLAY_IR_FUNCTION_SET_5x8DOTS );
            systick_delay_us(DISPLAY_DEL_37US);
        break;

        case DISPLAY_CONNECTION_GPIO_4BITS:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_FUNCTION_SET |
                              DISPLAY_IR_FUNCTION_SET_4BITS );
            systick_delay_us(DISPLAY_DEL_37US);
            initial8BitCommunicationIsCompleted = true;

            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_FUNCTION_SET |
                              DISPLAY_IR_FUNCTION_SET_4BITS |
                              DISPLAY_IR_FUNCTION_SET_2LINES |
                              DISPLAY_IR_FUNCTION_SET_5x8DOTS );
            systick_delay_us(DISPLAY_DEL_37US);
        break;
    }

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_DISPLAY_CONTROL |
                      DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_OFF |
                      DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF |
                      DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF );
    systick_delay_us(DISPLAY_DEL_37US);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_CLEAR_DISPLAY );
    systick_delay_us(DISPLAY_DEL_37US);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_ENTRY_MODE_SET |
                      DISPLAY_IR_ENTRY_MODE_SET_INCREMENT |
                      DISPLAY_IR_ENTRY_MODE_SET_NO_SHIFT );
    systick_delay_us(DISPLAY_DEL_37US);

    displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                      DISPLAY_IR_DISPLAY_CONTROL |
                      DISPLAY_IR_DISPLAY_CONTROL_DISPLAY_ON |
                      DISPLAY_IR_DISPLAY_CONTROL_CURSOR_OFF |
                      DISPLAY_IR_DISPLAY_CONTROL_BLINK_OFF );
    systick_delay_us(DISPLAY_DEL_37US);
}


void displayCharPositionWrite( uint8_t charPositionX, uint8_t charPositionY )
{
    switch( charPositionY ) {
        case 0:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE1_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            systick_delay_us(DISPLAY_DEL_37US);
        break;

        case 1:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE2_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            systick_delay_us(DISPLAY_DEL_37US);
        break;

        case 2:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE3_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            systick_delay_us(DISPLAY_DEL_37US);
        break;

        case 3:
            displayCodeWrite( DISPLAY_RS_INSTRUCTION,
                              DISPLAY_IR_SET_DDRAM_ADDR |
                              ( DISPLAY_20x4_LINE4_FIRST_CHARACTER_ADDRESS +
                                charPositionX ) );
            systick_delay_us(DISPLAY_DEL_37US);
        break;
    }
}

void displayStringWrite( const char * str )
{
    while (*str) {
        displayCodeWrite(DISPLAY_RS_DATA, *str++);
    }
}


//=====[Implementations of private functions]==================================
static void displayCodeWrite( bool type, uint8_t dataBus )
{
    if ( type == DISPLAY_RS_INSTRUCTION )
        displayPinWrite( DISPLAY_PIN_RS, DISPLAY_RS_INSTRUCTION );
	else
        displayPinWrite( DISPLAY_PIN_RS, DISPLAY_RS_DATA );
    displayPinWrite( DISPLAY_PIN_RW, DISPLAY_RW_WRITE );
    displayDataBusWrite( dataBus );
}

static void displayPinWrite( uint8_t pinName, int value )
{
	GPIO_PinState pinState = (value == OFF) ? GPIO_PIN_RESET : GPIO_PIN_SET;
	switch( display.connection ) {
	        case DISPLAY_CONNECTION_GPIO_8BITS:
	        	switch( pinName ) {
	        		case DISPLAY_PIN_D0:
						HAL_GPIO_WritePin(LCD_D0_PORT, LCD_D0_PIN, pinState);   break;
					case DISPLAY_PIN_D1:
						HAL_GPIO_WritePin(LCD_D1_PORT, LCD_D1_PIN, pinState);   break;
					case DISPLAY_PIN_D2:
						HAL_GPIO_WritePin(LCD_D2_PORT, LCD_D2_PIN, pinState);   break;
					case DISPLAY_PIN_D3:
						HAL_GPIO_WritePin(LCD_D3_PORT, LCD_D3_PIN, pinState);   break;
					case DISPLAY_PIN_D4:
						HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, pinState);   break;
					case DISPLAY_PIN_D5:
						HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, pinState);   break;
					case DISPLAY_PIN_D6:
						HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, pinState);   break;
					case DISPLAY_PIN_D7:
						HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN, pinState);   break;
					case DISPLAY_PIN_RS:
						HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, pinState);   break;
					case DISPLAY_PIN_EN:
						HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, pinState);   break;
					case DISPLAY_PIN_RW: break;
					default: break;
				}
				break;

	        case DISPLAY_CONNECTION_GPIO_4BITS:
	            switch( pinName ) {
	                case DISPLAY_PIN_D4:
	                	HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, pinState);   break;
	                case DISPLAY_PIN_D5:
						HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, pinState);   break;
	                case DISPLAY_PIN_D6:
						HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, pinState);   break;
	                case DISPLAY_PIN_D7:
						HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN, pinState);   break;
	                case DISPLAY_PIN_RS:
	                	HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, pinState);   break;
	                case DISPLAY_PIN_EN:
	                	HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, pinState);   break;
	                case DISPLAY_PIN_RW: break;
	                default: break;
	            }
	            break;
	    }


}
/*
static void displayPinWrite( uint8_t pinName, int value )
{
    GPIO_PinState pinState = (value == ON) ? GPIO_PIN_SET : GPIO_PIN_RESET;



    switch( pinName ) {
        case DISPLAY_PIN_D4:
            HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN, pinState);
            break;
        case DISPLAY_PIN_D5:
            HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN, pinState);
            break;
        case DISPLAY_PIN_D6:
            HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN, pinState);
            break;
        case DISPLAY_PIN_D7:
            HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN, pinState);
            break;
        case DISPLAY_PIN_RS:
            HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN, pinState);
            break;
        case DISPLAY_PIN_EN:
            HAL_GPIO_WritePin(LCD_EN_PORT, LCD_EN_PIN, pinState);
            break;
        default:
            break;
    }
}
*/
/*
// En display.c

static void displayDataBusWrite( uint8_t dataBus )
{
    displayPinWrite( DISPLAY_PIN_EN, OFF );

    // Escribir siempre el nibble superior (bits 7-4) en los pines D7-D4
    displayPinWrite( DISPLAY_PIN_D7, (dataBus & 0b10000000) ? 1 : 0 );
    displayPinWrite( DISPLAY_PIN_D6, (dataBus & 0b01000000) ? 1 : 0 );
    displayPinWrite( DISPLAY_PIN_D5, (dataBus & 0b00100000) ? 1 : 0 );
    displayPinWrite( DISPLAY_PIN_D4, (dataBus & 0b00010000) ? 1 : 0 );

    if (display.connection == DISPLAY_CONNECTION_GPIO_8BITS)
    {
        // Si es 8-bits, escribir también el nibble inferior (bits 3-0)
        displayPinWrite( DISPLAY_PIN_D3, (dataBus & 0b00001000) ? 1 : 0 );
        displayPinWrite( DISPLAY_PIN_D2, (dataBus & 0b00000100) ? 1 : 0 );
        displayPinWrite( DISPLAY_PIN_D1, (dataBus & 0b00000010) ? 1 : 0 );
        displayPinWrite( DISPLAY_PIN_D0, (dataBus & 0b00000001) ? 1 : 0 );
    }

    // Si estamos en modo 4 bits, PERO AÚN NO terminamos la inicialización
    // (que empieza en 8-bits), debemos hacer el pulso EN aquí.
    if ( (display.connection == DISPLAY_CONNECTION_GPIO_4BITS) &&
         (initial8BitCommunicationIsCompleted == false) )
    {
        displayPinWrite( DISPLAY_PIN_EN, ON );
        systick_delay_us(DISPLAY_DEL_37US);
        displayPinWrite( DISPLAY_PIN_EN, OFF );
        systick_delay_us(DISPLAY_DEL_37US); // Espera después del pulso
        return; // Salir de la función aquí
    }

    // Si estamos en modo 8 bits, hacer el pulso EN y salir.
    if (display.connection == DISPLAY_CONNECTION_GPIO_8BITS)
    {
        displayPinWrite( DISPLAY_PIN_EN, ON );
        systick_delay_us(DISPLAY_DEL_37US);
        displayPinWrite( DISPLAY_PIN_EN, OFF );
        systick_delay_us(DISPLAY_DEL_37US); // Espera después del pulso
        return; // Salir
    }

    // --- A partir de aquí, solo se ejecuta en modo 4-bits REAL ---
    // (initial8BitCommunicationIsCompleted es true)

    // Hacer el pulso EN para el nibble superior
    displayPinWrite( DISPLAY_PIN_EN, ON );
    systick_delay_us(DISPLAY_DEL_37US);
    displayPinWrite( DISPLAY_PIN_EN, OFF );
    systick_delay_us(DISPLAY_DEL_37US); // Espera

    // Escribir el nibble inferior (bits 3-0) en los pines D7-D4
    displayPinWrite( DISPLAY_PIN_D7, (dataBus & 0b00001000) ? 1 : 0 );
    displayPinWrite( DISPLAY_PIN_D6, (dataBus & 0b00000100) ? 1 : 0 );
    displayPinWrite( DISPLAY_PIN_D5, (dataBus & 0b00000010) ? 1 : 0 );
    displayPinWrite( DISPLAY_PIN_D4, (dataBus & 0b00000001) ? 1 : 0 );

    // Hacer el pulso EN para el nibble inferior
    displayPinWrite( DISPLAY_PIN_EN, ON );
    systick_delay_us(DISPLAY_DEL_37US);
    displayPinWrite( DISPLAY_PIN_EN, OFF );
    systick_delay_us(DISPLAY_DEL_37US); // Espera después del pulso
}
*/




static void displayDataBusWrite( uint8_t dataBus )
{
    displayPinWrite( DISPLAY_PIN_EN, OFF );
    displayPinWrite( DISPLAY_PIN_D7, dataBus & 0b10000000 );
    displayPinWrite( DISPLAY_PIN_D6, dataBus & 0b01000000 );
    displayPinWrite( DISPLAY_PIN_D5, dataBus & 0b00100000 );
    displayPinWrite( DISPLAY_PIN_D4, dataBus & 0b00010000 );
    switch( display.connection ) {
        case DISPLAY_CONNECTION_GPIO_8BITS:
            displayPinWrite( DISPLAY_PIN_D3, dataBus & 0b00001000 );
            displayPinWrite( DISPLAY_PIN_D2, dataBus & 0b00000100 );
            displayPinWrite( DISPLAY_PIN_D1, dataBus & 0b00000010 );
            displayPinWrite( DISPLAY_PIN_D0, dataBus & 0b00000001 );
        break;

        case DISPLAY_CONNECTION_GPIO_4BITS:
            if ( initial8BitCommunicationIsCompleted == true) {
                displayPinWrite( DISPLAY_PIN_EN, ON );
                systick_delay_us(DISPLAY_DEL_37US);
                displayPinWrite( DISPLAY_PIN_EN, OFF );
                systick_delay_us(DISPLAY_DEL_37US);
                displayPinWrite( DISPLAY_PIN_D7, dataBus & 0b00001000 );
                displayPinWrite( DISPLAY_PIN_D6, dataBus & 0b00000100 );
                displayPinWrite( DISPLAY_PIN_D5, dataBus & 0b00000010 );
                displayPinWrite( DISPLAY_PIN_D4, dataBus & 0b00000001 );
            }
        break;

    }
    displayPinWrite( DISPLAY_PIN_EN, ON );
    systick_delay_us(DISPLAY_DEL_37US);
    displayPinWrite( DISPLAY_PIN_EN, OFF );
    systick_delay_us(DISPLAY_DEL_37US);
}

