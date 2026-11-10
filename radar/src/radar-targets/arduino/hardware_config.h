#ifndef RADAR_ARDUINO_HARDWARE_CONFIG_H
#define RADAR_ARDUINO_HARDWARE_CONFIG_H

#define RADAR_UART_BAUD 115200UL
#define RADAR_OLED_WIDTH 128u
#define RADAR_OLED_HEIGHT 64u
#define RADAR_OLED_I2C_ADDRESS 0x3Cu
#define RADAR_OLED_RESET_PIN (-1)

#ifndef RADAR_UART_CLOCK_HZ
#ifdef F_CPU
#define RADAR_UART_CLOCK_HZ F_CPU
#else
#define RADAR_UART_CLOCK_HZ 16000000UL
#endif
#endif

/* Backend UART pour les cartes Arduino AVR à ATmega328P, comme Uno/Nano classiques. */
#ifndef RADAR_UART_CUSTOM_BACKEND
#include <avr/io.h>

#define RADAR_UART_UBRR_VALUE \
	((RADAR_UART_CLOCK_HZ / (8UL * RADAR_UART_BAUD)) - 1UL)
#define RADAR_UART_INIT() do { \
	UBRR0H = (uint8_t)(RADAR_UART_UBRR_VALUE >> 8); \
	UBRR0L = (uint8_t)RADAR_UART_UBRR_VALUE; \
	UCSR0A = (1u << U2X0); \
	UCSR0B = (1u << RXEN0); \
	UCSR0C = (1u << UCSZ01) | (1u << UCSZ00); \
} while (0)
#define RADAR_UART_RX_READY() ((UCSR0A & (1u << RXC0)) != 0u)
#define RADAR_UART_READ_STATUS() (UCSR0A)
#define RADAR_UART_READ_BYTE() (UDR0)
#define RADAR_UART_STATUS_HAS_ERROR(status) \
	(((status) & ((1u << FE0) | (1u << DOR0) | (1u << UPE0))) != 0u)
#endif

#endif
