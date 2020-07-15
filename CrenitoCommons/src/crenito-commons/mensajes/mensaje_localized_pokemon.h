/*
 * mensaje_localized_pokemon.h
 *
 *  Created on: 24 may. 2020
 *      Author: utnso
 */

#ifndef SRC_CRENITO_COMMONS_MENSAJES_MENSAJE_LOCALIZED_POKEMON_H_
#define SRC_CRENITO_COMMONS_MENSAJES_MENSAJE_LOCALIZED_POKEMON_H_

#include "../conexiones/buffer.h"
#include "../modelo/posicion.h"
#include <commons/collections/list.h>

#include "mensaje_cache_metadata.h"
#include "mensaje_header.h"

typedef struct {
	t_mensaje_header mensaje_header;
	char* especie;
	uint32_t especie_lenght;
	uint32_t posiciones_lenght;
	t_list* posiciones;
} t_mensaje_localized_pokemon;

// Creación y destrucción
t_mensaje_localized_pokemon* mensaje_localized_pokemon_crear(char* especie, t_list* posiciones);
void mensaje_localized_pokemon_destruir(t_mensaje_localized_pokemon* localized_pokemon);

// Serialización
t_buffer* mensaje_localized_pokemon_serializar(t_mensaje_localized_pokemon* localized_pokemon);
t_mensaje_localized_pokemon* mensaje_localized_pokemon_deserializar(void* stream);

// Cache
void* mensaje_localized_pokemon_formato_cache(void* stream, t_mensaje_cache_metadata* metadata);
t_mensaje_localized_pokemon* mensaje_localized_pokemon_restaurar_desde_cache(void* cache, t_mensaje_header header);

//Log
void mensaje_localized_pokemon_log(t_log* un_logger, t_mensaje_localized_pokemon* localized_pokemon);
char* mensaje_localized_pokemon_to_string(t_mensaje_localized_pokemon* localized_pokemon);

// Getters
uint32_t mensaje_localized_pokemon_get_id(t_mensaje_localized_pokemon* msj);
uint32_t mensaje_localized_pokemon_get_id_correlativo(t_mensaje_localized_pokemon* msj);
t_mensaje_header mensaje_localized_pokemon_get_header(t_mensaje_localized_pokemon* msj);

// Setters
void mensaje_localized_pokemon_set_id(t_mensaje_localized_pokemon* msj, uint32_t id);
void mensaje_localized_pokemon_set_id_correlativo(t_mensaje_localized_pokemon* msj, uint32_t id_correlativo);
void mensaje_localized_pokemon_set_header(t_mensaje_localized_pokemon* msj, t_mensaje_header header);

#endif /* SRC_CRENITO_COMMONS_MENSAJES_MENSAJE_LOCALIZED_POKEMON_H_ */
