#include "wokwi-api.h"
#include <stdlib.h>
#include <stdio.h>

#define DELAI_ENVOI_US 120000u
#define TAILLE_TRAME 6u
#define NOMBRE_CIBLES 4u
#define TAILLE_JEU (TAILLE_TRAME * NOMBRE_CIBLES)

/* Chaque jeu est un instantane : les quatre vehicules se rapprochent du radar.
 * Une trame contient : entete, ID, distance (m), vitesse (km/h), reserve, checksum. */
static uint8_t jeux_donnees[][TAILLE_JEU] = {
  { /* t=0 ms */
    0x55, 0x00, 0x64, 0x32, 0x00, 0xEB, /* Vehicule 0 : 100 m, 50 km/h. */
    0x55, 0x01, 0x32, 0x32, 0x00, 0xBA, /* Vehicule 1 : 50 m, 50 km/h. */
    0x55, 0x07, 0x28, 0x50, 0x00, 0xD4, /* Vehicule 7 : 40 m, 80 km/h. */
    0x55, 0x02, 0x50, 0x28, 0x00, 0xCF  /* Vehicule 2 : 80 m, 40 km/h. */
  },
  { /* t=120 ms : chaque vehicule s'est rapproche de 5 m. */
    0x55, 0x00, 0x5F, 0x32, 0x00, 0xE6,
    0x55, 0x01, 0x2D, 0x32, 0x00, 0xB5,
    0x55, 0x07, 0x23, 0x50, 0x00, 0xCF,
    0x55, 0x02, 0x4B, 0x28, 0x00, 0xCA
  },
  { /* t=240 ms : poursuite de l'approche. */
    0x55, 0x00, 0x5A, 0x32, 0x00, 0xE1,
    0x55, 0x01, 0x28, 0x32, 0x00, 0xB0,
    0x55, 0x07, 0x1E, 0x50, 0x00, 0xCA,
    0x55, 0x02, 0x46, 0x28, 0x00, 0xC5
  },
  { /* t=360 ms : poursuite de l'approche. */
    0x55, 0x00, 0x55, 0x32, 0x00, 0xDC,
    0x55, 0x01, 0x23, 0x32, 0x00, 0xAB,
    0x55, 0x07, 0x19, 0x50, 0x00, 0xC5,
    0x55, 0x02, 0x41, 0x28, 0x00, 0xC0
  },
  { /* t=480 ms : poursuite de l'approche. */
    0x55, 0x00, 0x50, 0x32, 0x00, 0xD7,
    0x55, 0x01, 0x1E, 0x32, 0x00, 0xA6,
    0x55, 0x07, 0x14, 0x50, 0x00, 0xC0,
    0x55, 0x02, 0x3C, 0x28, 0x00, 0xBB
  }
};

typedef struct {
  uart_dev_t uart;
} radar_state_t;

static void on_timer(void *user_data) {
  radar_state_t *radar = (radar_state_t *)user_data;
  static uint8_t jeu_index;
  uint8_t *jeu = jeux_donnees[jeu_index];

  if (!uart_write(radar->uart, jeu, sizeof(jeux_donnees[0]))) {
    printf("Erreur: uart_write a echoue\n");
  }
  jeu_index++;
  if (jeu_index >= sizeof(jeux_donnees) / sizeof(jeux_donnees[0])) {
    jeu_index = 0u;
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