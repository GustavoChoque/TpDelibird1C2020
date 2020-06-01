/*
 * mensajes-utils.h
 *
 *  Created on: 1 jun. 2020
 *      Author: utnso
 */

#ifndef SRC_MENSAJES_UTILS_H_
#define SRC_MENSAJES_UTILS_H_

#include <crenito-commons/conexiones/conexiones.h>
#include "hilo_appeared_pokemon.h"
#include "hilo_caught_pokemon.h"
#include "hilo_localized_pokemon.h"
#include "hilo_gameboy.h"

void mensaje_recibido(t_paquete* paquete);

#endif /* SRC_MENSAJES_UTILS_H_ */