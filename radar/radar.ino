#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdio.h>

#include "src/acquisition_signal.h"
#include "src/affichage.h"
#include "config.h"
#include "src/radar-targets/arduino/hardware_config.h"

static Adafruit_SSD1306 radar_display(
	RADAR_OLED_WIDTH, RADAR_OLED_HEIGHT, &Wire, RADAR_OLED_RESET_PIN);
static unsigned long last_radar_controller_ms;
static unsigned long last_affichage_controller_ms;
static uint8_t traced_distance[RADAR_TARGET_COUNT];
static uint8_t traced_speed[RADAR_TARGET_COUNT];

static int serial_printf_putc(char character, FILE *stream)
{
	(void)stream;
	if (character == '\n' && Serial.write('\r') != 1u) {
		return -1;
	}
	return Serial.write(static_cast<uint8_t>(character)) == 1u ? 0 : -1;
}

static bool connect_c_output_to_serial()
{
	return fdevopen(serial_printf_putc, nullptr) != nullptr;
}

extern "C" uint8_t radar_uart_rx_ready(void)
{
	return Serial.available() > 0 ? 1u : 0u;
}

extern "C" uint8_t radar_uart_rx_available(void)
{
	return static_cast<uint8_t>(Serial.available());
}

extern "C" uint8_t radar_uart_read_status(void)
{
	return 0u;
}

extern "C" uint8_t radar_uart_read_byte(void)
{
	int value = Serial.read();
	return value < 0 ? 0u : static_cast<uint8_t>(value);
}

extern "C" bool radar_oled_write_line(void *context, uint8_t row,
									  const char *text)
{
	Adafruit_SSD1306 *display = static_cast<Adafruit_SSD1306 *>(context);

	if (row >= AFFICHAGE_LCD_ROWS) {
		return false;
	}
	if (row == 0u) {
		display->clearDisplay();
	}
	display->setTextSize(1);
	display->setTextColor(SSD1306_WHITE);
	display->setCursor(0, row * 8u);
	display->print(text);
	if (row == AFFICHAGE_LCD_ROWS - 1u) {
		display->display();
	}
	return true;
}

static void trace_radar_targets()
{
	for (uint8_t i = 0u; i < RADAR_TARGET_COUNT; i++) {
		const Structure_info *target = &targets_info[i];
		if ((target->Distance != 0u || target->Speed != 0u) &&
			(target->Distance != traced_distance[i] ||
			 target->Speed != traced_speed[i])) {
			Serial.print(F("Radar ID="));
			Serial.print(target->Id);
			Serial.print(F(" distance="));
			Serial.print(target->Distance);
			Serial.print(F(" m vitesse="));
			Serial.print(target->Speed);
			Serial.println(F(" km/h"));
			traced_distance[i] = target->Distance;
			traced_speed[i] = target->Speed;
		}
	}
}


void setup()
{
	Serial.begin(RADAR_UART_BAUD);
	if (!connect_c_output_to_serial()) {
		Serial.println(F("Erreur: impossible de connecter printf a Serial"));
	}
	Wire.begin();
	if (!radar_display.begin(SSD1306_SWITCHCAPVCC,
							RADAR_OLED_I2C_ADDRESS)) {
		for (;;) {
		}
	}
	affichage_init(radar_oled_write_line, &radar_display);
	aquisition_init();
	last_radar_controller_ms = millis();
	last_affichage_controller_ms = last_radar_controller_ms;
}

void loop()
{
	unsigned long now = millis();

	if ((unsigned long)(now - last_radar_controller_ms) >=
		CONFIG_RADAR_CONTROLLER_PERIODE_MS) {
		last_radar_controller_ms = now;
		read_radar((uint32_t)now);
		trace_radar_targets();
	}

	if ((unsigned long)(now - last_affichage_controller_ms) >=
		CONFIG_AFFICHAGE_CONTROLLER_PERIODE_MS) {
		last_affichage_controller_ms = now;
		(void)affichage_update((uint32_t)now);
	}
}
