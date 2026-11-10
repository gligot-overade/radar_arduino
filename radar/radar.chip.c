#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>

#define DELAI_ENVOI_US 2000000u
#define NOMBRE_CIBLES  4u
#define TAILLE_TRAME   6u

/* Header, ID, distance, vitesse, reserve, checksum. */
static const uint8_t cibles[NOMBRE_CIBLES][TAILLE_TRAME] = {
  { 0x55, 0x00, 0x64, 0x32, 0x00, 0xEB },
  { 0x55, 0x01, 0x32, 0x32, 0x00, 0xBA },
  { 0x55, 0x07, 0x28, 0x50, 0x00, 0xD4 },
  { 0x55, 0x02, 0x50, 0x28, 0x00, 0xCF }
};

typedef struct {
  uart_dev_t uart;
} radar_state_t;

static void on_timer(void *user_data) {
  radar_state_t *radar = (radar_state_t *)user_data;
  uint8_t donnees[NOMBRE_CIBLES * TAILLE_TRAME];
  uint32_t longueur = 0u;

  for (uint8_t cible = 0u; cible < NOMBRE_CIBLES; cible++) {
    uint8_t checksum = 0u;
    for (uint8_t octet = 0u; octet < TAILLE_TRAME - 1u; octet++) {
      uint8_t valeur = cibles[cible][octet];
      donnees[longueur++] = valeur;
      checksum = (uint8_t)(checksum + valeur);
    }
    donnees[longueur++] = checksum;
  }

  if (!uart_write(radar->uart, donnees, longueur)) {
    printf("Erreur: uart_write a echoue\n");
  }
}

void chip_init(void) {
  radar_state_t *radar = malloc(sizeof(*radar));
  if (radar == NULL) {
    return;
  }

  uart_config_t config = {
    .tx = pin_init("TX", INPUT_PULLUP),
    .rx = pin_init("RX", INPUT),
    .baud_rate = 115200,
    .user_data = radar,
  };
  radar->uart = uart_init(&config);

  timer_config_t timer_config = {
    .callback = on_timer,
    .user_data = radar,
  };
  timer_start(timer_init(&timer_config), DELAI_ENVOI_US, true);
}