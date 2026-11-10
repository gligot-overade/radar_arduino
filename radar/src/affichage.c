#include "affichage.h"

#include "acquisition_signal.h"

#include <stdio.h>

static affichage_write_line_fn write_line;
static void *lcd_context;
static uint32_t last_update_ms;
static bool has_updated;

void affichage_init(affichage_write_line_fn write_fn, void *context)
{
	write_line = write_fn;
	lcd_context = context;
	last_update_ms = 0u;
	has_updated = false;
}

bool affichage_update(uint32_t now_ms)
{
	char line[AFFICHAGE_LINE_LENGTH];

	if (write_line == 0) {
		return false;
	}
	if (has_updated && (uint32_t)(now_ms - last_update_ms) <
		AFFICHAGE_REFRESH_INTERVAL_MS) {
		return true;
	}

	for (uint8_t row = 0u; row < AFFICHAGE_LCD_ROWS; row++) {
		const Structure_info *target = &targets_info[row];
		(void)snprintf(line, sizeof(line), "ID:%u D:%u V:%u",
					(unsigned int)target->Id,
					(unsigned int)target->Distance,
					(unsigned int)target->Speed);
		if (!write_line(lcd_context, row, line)) {
			return false;
		}
	}

	last_update_ms = now_ms;
	has_updated = true;
	return true;
}
