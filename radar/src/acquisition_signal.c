#include "acquisition_signal.h"
#include "../config.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 * Le radar envoie jusqu'à 8 records de 6 octets, sans longueur globale ni
 * marqueur de fin. radar_record assemble un record; radar_frame conserve les
 * records valides jusqu'à la fin détectée par le silence UART.
 */
static uint8_t radar_record[RADAR_RECORD_LENGTH];
static uint8_t radar_record_length = 0u;
static uint8_t radar_frame[RADAR_FRAME_MAX_LENGTH];
static uint8_t radar_frame_length = 0u;
static bool radar_frame_overflow = false;
static uint8_t radar_last_available_count = 0u;
static uint32_t radar_last_rx_activity_ms = 0u;
static bool radar_rx_activity_seen = false;
Structure_info targets_info[RADAR_TARGET_COUNT];

/* Réinitialiser les buffers puis initialiser le backend UART choisi. */
void aquisition_init(void)
{
    radar_record_length = 0u;
    radar_frame_length = 0u;
    radar_frame_overflow = false;
	radar_last_available_count = 0u;
	radar_last_rx_activity_ms = 0u;
	radar_rx_activity_seen = false;
    for (uint8_t i = 0u; i < RADAR_TARGET_COUNT; i++) {
        targets_info[i].Id = i;
        targets_info[i].Distance = 0u;
        targets_info[i].Speed = 0u;
    }

	/* L'initialisation matérielle est fournie par le backend du header. */
	RADAR_UART_INIT();
}


bool radar_capture_receive_frame(const uint8_t *data_brut, size_t len)
{
	size_t i = 0u;
	bool updated = false;

	if (data_brut == NULL) {
		return false;
	}

    while (len - i >= RADAR_RECORD_LENGTH) {
		Structure_info info;
		uint8_t checksum;

        if (data_brut[i] != RADAR_RECORD_START_BYTE ||
            data_brut[i + RADAR_RECORD_ID_OFFSET] >= RADAR_TARGET_COUNT) {
			i++;
			continue;
		}

        info.Id = data_brut[i + RADAR_RECORD_ID_OFFSET];
        info.Distance = data_brut[i + RADAR_RECORD_DISTANCE_OFFSET];
        info.Speed = data_brut[i + RADAR_RECORD_SPEED_OFFSET];
        checksum = (uint8_t)(RADAR_RECORD_START_BYTE + info.Id +
            info.Distance + info.Speed +
            data_brut[i + RADAR_RECORD_RESERVED_OFFSET]);

        if (checksum == data_brut[i + RADAR_RECORD_CHECKSUM_OFFSET]) {
			targets_info[info.Id] = info;
			updated = true;
            i += RADAR_RECORD_LENGTH;
		} else {
			i++;
		}
	}

	return updated;
}

static bool radar_record_is_valid(void)
{
	/* Le checksum est la somme modulo 256 des cinq premiers octets. */
	uint8_t checksum = (uint8_t)(radar_record[0] + radar_record[1] +
		radar_record[2] + radar_record[3] + radar_record[4]);

	return radar_record[0] == RADAR_RECORD_START_BYTE &&
		radar_record[1] < RADAR_TARGET_COUNT &&
		checksum == radar_record[RADAR_RECORD_CHECKSUM_OFFSET];
}

static void finish_radar_frame(void)
{
	if (radar_frame_length > 0u && !radar_frame_overflow) {
		/* Valider les records et mettre directement à jour les cibles. */
		(void)radar_capture_receive_frame(radar_frame, radar_frame_length);
	}

	radar_frame_length = 0u;
	radar_frame_overflow = false;
}

static void process_radar_byte(uint8_t byte)
{
	/* Chaque record commence par 0x55; ignorer le bruit avant cet octet. */
    if (radar_record_length == 0u) {
        if (byte != RADAR_RECORD_START_BYTE) {
            return;
        }
        radar_record[radar_record_length++] = byte;
        return;
    }

    radar_record[radar_record_length++] = byte;
    if (radar_record_length == RADAR_RECORD_LENGTH) {
		if (radar_record_is_valid()) {
			/* Ajouter uniquement les records complets et vérifiés au buffer. */
			if ((size_t)radar_frame_length + RADAR_RECORD_LENGTH <=
				RADAR_FRAME_MAX_LENGTH) {
				memcpy(&radar_frame[radar_frame_length], radar_record,
					RADAR_RECORD_LENGTH);
				radar_frame_length = (uint8_t)(radar_frame_length +
					RADAR_RECORD_LENGTH);
			} else {
				radar_frame_overflow = true;
			}
            radar_record_length = 0u;
            return;
        }

        /*
         * Record invalide : chercher un éventuel 0x55 dans les octets déjà
         * reçus pour se resynchroniser sans perdre le record suivant.
         */
        uint8_t next_start = 1u;
        while (next_start < RADAR_RECORD_LENGTH &&
             radar_record[next_start] != RADAR_RECORD_START_BYTE) {
            next_start++;
        }
        if (next_start == RADAR_RECORD_LENGTH) {
            radar_record_length = 0u;
        } else {
        radar_record_length = (uint8_t)(RADAR_RECORD_LENGTH - next_start);
        memmove(radar_record, &radar_record[next_start], radar_record_length);
        }
    }
}

void read_radar(uint32_t now_ms)
{
	uint8_t available_count = RADAR_UART_RX_AVAILABLE();

	/*
	 * La boucle appelle cette fonction régulièrement. Le buffer RX est rempli
	 * par l'interruption série Arduino. Comme on ne le lit qu'après 5 ms de
	 * silence, comparer son nombre d'octets permet de repérer une arrivée.
	 */
	if (available_count == 0u) {
		radar_last_available_count = 0u;
		radar_rx_activity_seen = false;
		return;
	}

	if (!radar_rx_activity_seen ||
		available_count != radar_last_available_count) {
		radar_last_available_count = available_count;
		radar_last_rx_activity_ms = now_ms;
		radar_rx_activity_seen = true;
		return;
	}

	if ((uint32_t)(now_ms - radar_last_rx_activity_ms) <
		RADAR_FRAME_END_TIMEOUT_MS) {
		return;
	}

	/*
	 * Aucun nouvel octet depuis 5 ms : vider le buffer série, assembler et
	 * vérifier chaque record, puis publier la trame complète.
	 */
	while (RADAR_UART_RX_READY()) {
		/* Le backend fournit les indicateurs d'erreur et l'octet reçu. */
		uint8_t status = RADAR_UART_READ_STATUS();
		uint8_t byte = RADAR_UART_READ_BYTE();
		if (RADAR_UART_STATUS_HAS_ERROR(status)) {
			/* Une erreur UART invalide toute la trame en cours. */
            printf("Erreur: UART status=%02X, abandon de l'enregistrement\n", status);
            radar_record_length = 0u;
            radar_frame_length = 0u;
            radar_frame_overflow = false;
            radar_rx_activity_seen = false;
            continue;
		}
		process_radar_byte(byte);
	}

	/* Abandonner un éventuel record incomplet, puis publier la trame reçue. */
	radar_record_length = 0u;
	if (radar_frame_length > 0u || radar_frame_overflow) {
		finish_radar_frame();
	}

	radar_last_available_count = 0u;
	radar_rx_activity_seen = false;
}