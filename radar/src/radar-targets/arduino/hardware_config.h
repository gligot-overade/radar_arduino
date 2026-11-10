#ifndef RADAR_ARDUINO_HARDWARE_CONFIG_H
#define RADAR_ARDUINO_HARDWARE_CONFIG_H

#define RADAR_UART_BAUD 115200UL
#define RADAR_OLED_WIDTH 128u
#define RADAR_OLED_HEIGHT 64u
#define RADAR_OLED_I2C_ADDRESS 0x3Cu
#define RADAR_OLED_RESET_PIN (-1)

/* Backend UART pour les cartes Arduino AVR à ATmega328P, comme Uno/Nano classiques. */
#ifndef RADAR_UART_CUSTOM_BACKEND
/*
 * Use the Arduino Serial API: its RX interrupt fills the core's 64-byte
 * buffer while the sketch performs other work.
 */
#ifdef __cplusplus
extern "C" {
#endif
uint8_t radar_uart_rx_ready(void);
uint8_t radar_uart_rx_available(void);
uint8_t radar_uart_read_status(void);
uint8_t radar_uart_read_byte(void);
#ifdef __cplusplus
}
#endif

#define RADAR_UART_INIT() do { } while (0)
#define RADAR_UART_RX_READY() radar_uart_rx_ready()
#define RADAR_UART_RX_AVAILABLE() radar_uart_rx_available()
#define RADAR_UART_READ_STATUS() radar_uart_read_status()
#define RADAR_UART_READ_BYTE() radar_uart_read_byte()
#define RADAR_UART_STATUS_HAS_ERROR(status) ((status) != 0u)
#endif

#endif
