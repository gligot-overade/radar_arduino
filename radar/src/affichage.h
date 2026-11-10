#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <stdbool.h>
#include <stdint.h>

#define AFFICHAGE_LCD_ROWS 8u
#define AFFICHAGE_LINE_LENGTH 21u
#define AFFICHAGE_REFRESH_INTERVAL_MS 100u

#ifdef __cplusplus
extern "C" {
#endif

/* Le pilote OLED I2C Telink écrit une ligne numérotée de 0 à 7. */
typedef bool (*affichage_write_line_fn)(void *context, uint8_t row,
										const char *text);

void affichage_init(affichage_write_line_fn write_fn, void *context);
bool affichage_update(uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif