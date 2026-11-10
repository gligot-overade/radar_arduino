#include "acquisition_signal.h"

#include <stdint.h>
#include <string.h>

/* Accumule un enregistrement radar jusqu'à ses 6 octets complets. */
static uint8_t radar_record[RADAR_RECORD_LENGTH];
static uint8_t radar_record_length = 0u;
Structure_info targets_info[RADAR_TARGET_COUNT];

void aquisition_init(void)
{
    radar_record_length = 0u;
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
            info.Distance + info.Speed);

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


static void process_radar_byte(uint8_t byte)
{
	/* Hors d'un enregistrement, ignore le bruit jusqu'au marqueur 0x55. */
    if (radar_record_length == 0u) {
        if (byte != RADAR_RECORD_START_BYTE) {
            return;
        }
        radar_record[radar_record_length++] = byte;
        return;
    }

    radar_record[radar_record_length++] = byte;
    if (radar_record_length == RADAR_RECORD_LENGTH) {
		/* Le parseur vérifie l'ID et le checksum, puis actualise la cible. */
        if (radar_capture_receive_frame(radar_record, RADAR_RECORD_LENGTH)) {
            radar_record_length = 0u;
            return;
        }

        /* Si l'enregistrement est invalide, cherche un 0x55 interne pour
         * reprendre la synchronisation sans perdre le début suivant. */
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

void read_radar(void)
{
	/* Traite tous les octets déjà arrivés; RXC0 indique que UDR0 est prêt. */
	while (RADAR_UART_RX_READY()) {
		/* Le backend fournit les indicateurs d'erreur et l'octet reçu. */
		uint8_t status = RADAR_UART_READ_STATUS();
		uint8_t byte = RADAR_UART_READ_BYTE();
		if (RADAR_UART_STATUS_HAS_ERROR(status)) {
			/* Erreur de trame, débordement UART ou parité : abandonne l'enregistrement. */
            printf("Erreur: UART status=%02X, abandon de l'enregistrement\n", status);
			radar_record_length = 0u;
			continue;
		}
		process_radar_byte(byte);
	}
}