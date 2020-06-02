#include "mensaje.h"

//Fix para que compile esto hay que arreglarlo!!
static int get_id_mensaje(char* mensaje);

int get_id_mensaje(char* mensaje) {
	return 1;
}

t_paquete* crearMensaje(char* argumentos[], int longitud) {

	t_buffer* mensaje_serializado;
	t_id_cola Tipomensaje;
	char* especie;
	int atrapado;
	int cantidad, posx, posy;

	switch (get_id_mensaje(argumentos[2])) {

	case NEW_POKEMON:

		Tipomensaje = NEW_POKEMON;
		validar_quien_conoce_newpokemon(argumentos[1]);
		validar_mayor_igual_a_cero(&posx, argumentos[4]);
		validar_mayor_igual_a_cero(&posy, argumentos[5]); //en ambos casos valido esto

		if (strcmp(argumentos[1], BROKER_STRING) == 0) {

			validar_cantidad_argumentos(longitud, 7);
			//la especie no hace falta ser validada, ya que no puede recibir un espacio en blanco
			especie = argumentos[3];
			validar_mayor_a_cero(&cantidad, argumentos[6]);
			t_mensaje_new_pokemon* msj = mensaje_new_pokemon_crear(especie,
					posx, posy, cantidad);
			mensaje_serializado = mensaje_new_pokemon_serializar(msj);

		} else {
			//el mensaje serializado del new_pokemon del gamecard
			validar_cantidad_argumentos(longitud, 8);
			especie = argumentos[3];
			t_mensaje_new_pokemon* msj = mensaje_new_pokemon_crear(especie,
					posx, posy, cantidad);
			mensaje_serializado = mensaje_new_pokemon_serializar(msj);
		}

		break;

	case APPEARED_POKEMON:

		Tipomensaje = APPEARED_POKEMON;
		validar_quien_conoce_appearedpokemon(argumentos[1]);

		validar_mayor_igual_a_cero(&posx, argumentos[4]);
		validar_mayor_igual_a_cero(&posy, argumentos[5]);
		especie = argumentos[3];

		if (strcmp(argumentos[1], BROKER_STRING) == 0) {
			validar_cantidad_argumentos(longitud, 7);
			//se valida el id?? si no lo envio como parametro...
			t_mensaje_appeared_catch_pokemon* msj =
					mensaje_appeared_catch_pokemon_crear(especie, posx, posy);
			mensaje_serializado = mensaje_appeared_catch_pokemon_serializar(
					msj);

		} else {
			//para el team
			validar_cantidad_argumentos(longitud, 6);
			t_mensaje_appeared_catch_pokemon* msj =
					mensaje_appeared_catch_pokemon_crear(especie, posx, posy);
			mensaje_serializado = mensaje_appeared_catch_pokemon_serializar(
					msj);

		}

		break;
	case CATCH_POKEMON:

		Tipomensaje = CATCH_POKEMON;
		validar_quien_conoce_catchpokemon(argumentos[1]);
		especie = argumentos[3];
		validar_mayor_igual_a_cero(&posx, argumentos[4]);
		validar_mayor_igual_a_cero(&posy, argumentos[5]);

		if (strcmp(argumentos[1], BROKER_STRING) == 0) {

			validar_cantidad_argumentos(longitud, 6);
			t_mensaje_appeared_catch_pokemon* msj =
					mensaje_appeared_catch_pokemon_crear(especie, posx, posy);
			mensaje_serializado = mensaje_appeared_catch_pokemon_serializar(
					msj);
		} else {
			//para el gamecard......
			validar_cantidad_argumentos(longitud, 7);
			t_mensaje_appeared_catch_pokemon* msj =
					mensaje_appeared_catch_pokemon_crear(especie, posx, posy);
			mensaje_serializado = mensaje_appeared_catch_pokemon_serializar(
					msj);
		}

		break;
	case CAUGHT_POKEMON:

		Tipomensaje = CAUGHT_POKEMON;
		validar_quien_conoce_caughtpokemon(argumentos[1]);
		validar_cantidad_argumentos(longitud, 5);
		//validar_ok_fail(argumentos[4]); ver despues si es OK=1 o si es FAIL=0 x ejemplo....
		//atrapado=argumentos[4];
		atrapado = atoi(argumentos[4]);		//pasa el 1 o 0 a entero ver....

		t_mensaje_caught_pokemon* msj = mensaje_caught_pokemon_crear(atrapado);
		mensaje_serializado = mensaje_caught_pokemon_serializar(msj);

		break;
	case GET_POKEMON:

		Tipomensaje = GET_POKEMON;
		validar_quien_conoce_getpokemon(argumentos[1]);
		especie = argumentos[3];

		if (strcmp(argumentos[1], BROKER_STRING) == 0) {
			validar_cantidad_argumentos(longitud, 4);
			t_mensaje_get_pokemon* msj = mensaje_get_pokemon_crear(especie);
			mensaje_serializado = mensaje_get_pokemon_serializar(msj);

		} else {
			validar_cantidad_argumentos(longitud, 5);
			t_mensaje_get_pokemon* msj = mensaje_get_pokemon_crear(especie);
			mensaje_serializado = mensaje_get_pokemon_serializar(msj);
		}
		break;

	}

	return paquete_crear(paquete_header_crear(MENSAJE, GAMEBOY, Tipomensaje),
			mensaje_serializado);
}

