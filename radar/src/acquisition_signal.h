#ifndef ACQUISITION_SIGNAL_H
#define ACQUISITION_SIGNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "radar-targets/arduino/hardware_config.h"

#define RADAR_CAPTURE_BYTE_CAPACITY 512u
#define RADAR_CAPTURE_MAX_QUEUED_FRAMES 8u
#define RADAR_TARGET_COUNT 8u
#define RADAR_RECORD_START_BYTE 0x55u
#define RADAR_RECORD_LENGTH 6u
#define RADAR_RECORD_ID_OFFSET 1u
#define RADAR_RECORD_DISTANCE_OFFSET 2u
#define RADAR_RECORD_SPEED_OFFSET 3u
#define RADAR_RECORD_CHECKSUM_OFFSET 5u

#ifndef RADAR_UART_INIT
#error RADAR_UART_INIT doit etre defini pour ce backend
#endif
#ifndef RADAR_UART_RX_READY
#error RADAR_UART_RX_READY doit etre defini pour ce backend
#endif
#ifndef RADAR_UART_READ_STATUS
#error RADAR_UART_READ_STATUS doit etre defini pour ce backend
#endif
#ifndef RADAR_UART_READ_BYTE
#error RADAR_UART_READ_BYTE doit etre defini pour ce backend
#endif
#ifndef RADAR_UART_STATUS_HAS_ERROR
#error RADAR_UART_STATUS_HAS_ERROR doit etre defini pour ce backend
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uint8_t Id;
	uint8_t Distance;
	uint8_t Speed;
} Structure_info;

extern Structure_info targets_info[RADAR_TARGET_COUNT];

void aquisition_init(void);
void read_radar(void);

/* Analyse les records configurés ci-dessus et actualise les cibles valides. */
bool radar_capture_receive_frame(const uint8_t *frame, size_t length);

#ifdef __cplusplus
}
#endif

#endif