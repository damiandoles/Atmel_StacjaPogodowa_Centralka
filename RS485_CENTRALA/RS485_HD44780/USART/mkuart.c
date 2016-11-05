/*
 * mkuart.c
 *
 *  Created on: 2010-09-04
 *       Autor: Miroslaw Kardas
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "systime.h"
#include "mkuart.h"
// definiujemy w ko�cu nasz bufor UART_RxBuf
//volatile char UART_RxBuf[UART_RX_BUF_SIZE];
// definiujemy indeksy okre�laj�ce ilo�� danych w buforze
volatile uint8_t UART_RxHead; // indeks oznaczaj�cy �g�ow� w�a�
volatile uint8_t UART_RxTail; // indeks oznaczaj�cy �ogon w�a�

// definiujemy w ko�cu nasz bufor UART_RxBuf
volatile char UART_TxBuf[UART_TX_BUF_SIZE];
// definiujemy indeksy okre�laj�ce ilo�� danych w buforze
volatile uint8_t UART_TxHead; // indeks oznaczaj�cy �g�ow� w�a�
volatile uint8_t UART_TxTail; // indeks oznaczaj�cy �ogon w�a�

void USART_Init(uint16_t baud ) 
{
	/* Ustawienie predkosci */
	UBRRH = (uint8_t)(baud >> 8);
	UBRRL = (uint8_t)baud;
	/* Zalaczenie nadajnika I odbiornika */
	//UCSRB |= (1 << RXEN) | (1 << RXCIE) ;
	/* Ustawienie format ramki: 8bitow danych, 1 bit stopu */
	//UCSRC = (3 << UCSZ0);
	
	
	//UART_DE_DIR |= UART_DE_BIT;
	//UART_DE_ODBIERANIE;
	
	//debug
	//UCSR0B |= (1 << TXEN0);// | (1 << TXCIE); //115200
	
	/*Set baud rate */
	//UBRRH = (unsigned char)(baud>>8);
	//UBRRL = (unsigned char)baud;
	//Enable receiver and transmitter */
	//UCSRB |= (1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	
		//UBRRH = (BAUD_PRESCALE >> 8);
		//UBRRL = BAUD_PRESCALE;

		UCSRB = ((1<<TXEN) | (1<<RXEN) | (1<<RXCIE));
		UCSRC = ((1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0));
}

// definiujemy funkcj� dodaj�c� jeden bajtdoz bufora cyklicznego
void uart_putc(char data) 
{
	uint8_t tmp_head;

    tmp_head  = (UART_TxHead + 1) & UART_TX_BUF_MASK;

          // p�tla oczekuje je�eli brak miejsca w buforze cyklicznym na kolejne znaki
    while ( tmp_head == UART_TxTail ){}

    UART_TxBuf[tmp_head] = data;
    UART_TxHead = tmp_head;

    // inicjalizujemy przerwanie wyst�puj�ce, gdy bufor jest pusty, dzi�ki
    // czemu w dalszej cz�ci wysy�aniem danych zajmie si� ju� procedura
    // obs�ugi przerwania
    UCSRB |= (1<<UDRIE);
}

void uart_puts(char *s)		// wysy�a �a�cuch z pami�ci RAM na UART
{
#ifdef DEBUG_GLOBAL

	register char c;
	while ((c = *s++))
		uart_putc(c);			// dop�ki nie napotkasz 0 wysy�aj znak
#endif
}

void uart_putint(int value, int radix)	// wysy�a na port szeregowy tekst
{
	char string[17];			// bufor na wynik funkcji itoa
	itoa(value, string, radix);		// konwersja value na ASCII
	uart_puts(string);			// wy�lij string na port szeregowy
}

// definiujemy procedur� obs�ugi przerwania nadawczego, pobieraj�c� dane z bufora cyklicznego
ISR(USART_UDRE_vect)  
{
    // sprawdzamy czy indeksy s� r�ne
    if (UART_TxHead != UART_TxTail) 
	{
    	// obliczamy i zapami�tujemy nowy indeks ogona w�a (mo�e si� zr�wna� z g�ow�)
    	UART_TxTail = (UART_TxTail + 1) & UART_TX_BUF_MASK;
    	// zwracamy bajt pobrany z bufora  jako rezultat funkcji
    	UDR = UART_TxBuf[UART_TxTail];
    } 
	else 
	{
		// zerujemy flag� przerwania wyst�puj�cego gdy bufor pusty
		UCSRB &= ~(1<<UDRIE);
    }
}


// definiujemy funkcj� pobieraj�c� jeden bajt z bufora cyklicznego
char uart_getc(void) 
{
    // sprawdzamy czy indeksy s� r�wne
    if (UART_RxHead == UART_RxTail) 
		return 0;

    // obliczamy i zapami�tujemy nowy indeks �ogona w�a� (mo�e si� zr�wna� z g�ow�)
    UART_RxTail = (UART_RxTail + 1) & UART_RX_BUF_MASK;
    // zwracamy bajt pobrany z bufora  jako rezultat funkcji
    return UART_RxBuf[UART_RxTail];
}

//////////////////////////////////////////////////////////////////////////
ISR(USART_RXC_vect)
{
	UART_RxBuf[gRxBuffIndex] = UDR;
	gRxBuffIndex++;
	
	uartTimeoutTimer = systimeGet();
	if (gRxBuffIndex == 10)
	{
		//uartTimeoutTimer = systimeGet();
		gReceivedDataFlag = 1; 
		//gRxBuffIndex = 0;
	}
}