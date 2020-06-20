#include "../hilos-del-team/hilos_team.h"
#include "../team.h"
#include "../estructuras-auxiliares/mensajes.h"

void team_procesador_cola_LOCALIZED(cr_list* mensajes){

	while(PROCESO_ACTIVO){

		t_mensaje_localized_pokemon* pokemonLocalizado = leer_mensaje_cuando_este_disponible(mensajes);

		pthread_mutex_lock(&Mutex_AndoLoggeando);
		mensaje_localized_pokemon_log(logger, pokemonLocalizado);
		pthread_mutex_unlock(&Mutex_AndoLoggeando);

		if(!especie_recibida_con_anterioridad(pokemonLocalizado->especie, historialDePokemones)){
			especie_pokemon especie = string_duplicate(pokemonLocalizado->especie);
			posiciones_ordenar_por_cercania_al_equipo(pokemonLocalizado->posiciones);
			registrar_en_cada_posicion(especie, pokemonLocalizado->posiciones);
			free(especie);
		}

		else{
			pthread_mutex_lock(&Mutex_AndoLoggeandoEventos);
			log_info(event_logger, "Se descarto, ya que la especie %s habia sido recibida con anterioridad o el id no se corresponde con un get enviado", pokemonLocalizado->especie);
			pthread_mutex_unlock(&Mutex_AndoLoggeandoEventos);
		}

		mensaje_localized_pokemon_destruir(pokemonLocalizado);
	}
}

/*************************************** Funciones Auxiliares ************************************************/

void registrar_en_cada_posicion(especie_pokemon unaEspecie, posiciones listaDePosiciones){

	void registrarEnCadaPosicion(t_posicion* pos){
		pokemon* unPokemon = pokemon_ptr_create(string_duplicate(unaEspecie), *pos);
		registrar_pokemon(unPokemon);
	}

	list_iterate(listaDePosiciones, (void(*)(void*)) registrarEnCadaPosicion);
}

void posiciones_ordenar_por_cercania_al_equipo(posiciones listaPosiciones){
	bool cercaniaAEntrenadores(void*unaPos, void*otraPos){
		return posicion_distancia_a_equipo(unaPos) >= posicion_distancia_a_equipo(otraPos);
	}

	list_sort(listaPosiciones, &cercaniaAEntrenadores);
}


numero posicion_distancia_a_equipo(t_posicion*posicion){
	return posicion_distancia_a_entrenadores(posicion, equipo);
}

numero posicion_distancia_a_entrenadores(t_posicion* posicion, entrenadores unEquipo){

	numero distancia = NUMERO_MAXIMO;

	void menorDistancia(void*unEntrenador){
		t_id eid = ((entrenador*)unEntrenador)->id;
		pthread_mutex_lock(&mutexPosicionEntrenador[eid]);
		t_posicion posicionDelEntrenador = ((entrenador*)unEntrenador)->posicion;
		pthread_mutex_unlock(&mutexPosicionEntrenador[eid]);

		numero distanciaAlEntrenador = posicion_distancia(*posicion, posicionDelEntrenador);

		if(distancia > distanciaAlEntrenador) distancia = distanciaAlEntrenador;
	}

	pthread_mutex_lock(&mutexEntrenadores);
	list_iterate(equipo, &menorDistancia);
	pthread_mutex_unlock(&mutexEntrenadores);

	return distancia;
}

/*************************************** Historial de Pokemones ************************************************/

void agregar_al_historial(pokemon*unPokemon){
	especie_recibida_con_anterioridad(unPokemon->especie, historialDePokemones);
}

// Agrega los pokemones que van llegando al mapa en caso de ser requeridos
void registrar_pokemon(pokemon*unPokemon){
	agregar_al_historial(unPokemon);
	mapa_mapear_pokemon(pokemonesRecibidos, unPokemon);
}

// Retorna true si la especie ya se encuentra en el registro de especies
bool especie_recibida_con_anterioridad(especie_pokemon especie, especies_pokemones historial){

	pthread_mutex_lock(&mutexHistorialEspecies);
	bool siONo = list_get_by_comparation(historial, especie, (bool(*)(void*, void*))especie_cmp);
	if(!siONo) list_add(historial, string_duplicate(especie));
	pthread_mutex_unlock(&mutexHistorialEspecies);

	return siONo;
}

bool mensaje_localized_es_para_mi(t_mensaje_localized_pokemon* unMensaje){
	t_id idCorrelativo = mensaje_localized_pokemon_get_id_correlativo(unMensaje);
	return id_responde_a_mi_pedido(idCorrelativo);
}

bool id_responde_a_mi_pedido(t_id idCorrelativo){

	bool cmp(void*unId, void*otroId){
		return *((numero*)unId) == *((numero*)otroId);
	}

	pthread_mutex_lock(&mutexPedidos);
	t_id* idGet = list_get_by_comparation(registroDePedidos, &idCorrelativo, &cmp);
	pthread_mutex_unlock(&mutexPedidos);


	return idGet;
}
