#ifndef RADAR_CONFIG_H
#define RADAR_CONFIG_H

#define CONFIG_RADAR_CONTROLLER_PERIODE_MS (1UL)
#define CONFIG_AFFICHAGE_CONTROLLER_PERIODE_MS (100UL)

/* Le protocole n'a pas de longueur totale : une pause UART termine la trame. */
#define RADAR_FRAME_END_TIMEOUT_MS (5UL)

#endif
