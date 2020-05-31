/*
 * posicion.c
 *
 *  Created on: 2 may. 2020
 *      Author: utnso
 */

#include "posicion.h"

t_posicion posicion_crear(uint32_t pos_x, uint32_t pos_y) {

	t_posicion posicion;
	posicion.pos_x = pos_x;
	posicion.pos_y = pos_y;

	return posicion;
}

t_posicion* posicion_crear_ptr(uint32_t pos_x, uint32_t pos_y) {

	t_posicion* posicion = malloc(sizeof(t_posicion));

	*posicion = posicion_crear(pos_x, pos_y);

	return posicion;
}

void posicion_destruir(t_posicion* posicion){

	free(posicion);
}

void posicion_log(t_log* un_logger, t_posicion posicion){

	log_info(un_logger, "posición: (%d, %d)", posicion.pos_x, posicion.pos_y);
}

void posicion_list_log(t_log* un_logger, t_list* posiciones){

	for (int i = 0; i < list_size(posiciones); ++i) {
		t_posicion* posicion = list_get(posiciones, i);
		posicion_log(un_logger, *posicion);
	}
}
