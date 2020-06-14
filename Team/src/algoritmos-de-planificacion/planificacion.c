#include "planificacion.h"
#include "../team.h"

//Inicializar
void cargar_algoritmo_planificacion(){ //TODO A FUTURO

	char*algoritmoLeido = config_get_string_value(config,"ALGORITMO_PLANIFICACION");

	if(string_equals_ignore_case(algoritmoLeido, "FIFO")){
		inicializar_fifo();
	}

//	else if(string_equals_ignore_case(algoritmoLeido, "RR")){
//	}
//
//	else if(string_equals_ignore_case(algoritmoLeido, "SJF_SD")){ //SJF //Sin desalojo va a leer lo mismo que con
//		inicializar_sjf();
//		proximo_a_ejecutar_segun_criterio = proximo_segun_sjf;
//		entrenador_puede_seguir_ejecutando_segun_algoritmo = puede_seguir_sin_desalojo;
//	}

//	else if(string_equals_ignore_case(algoritmoLeido, "SJF_CD")){
//		inicializar_sjf();
//	}

	else{
		printf("El algoritmo %s no esta implementado... aun ;)", algoritmoLeido);
		exit(1);
	}

	RETARDO_CICLO_CPU = config_get_int_value(config, "RETARDO_CICLO_CPU");
}

void consumir_ciclo_cpu(){
	sleep(RETARDO_CICLO_CPU);
	//incrementar contador TODO
}

//Usar en funciones que requieren cpu del entrenador. Ej ir a (Ver abajo de todo)
void entrenador_esperar_y_consumir_cpu(entrenador*unEntrenador){
	sem_wait(&EjecutarEntrenador[unEntrenador->id]);
	consumir_ciclo_cpu();
}//	sem_signal(&ConsumioCpu); se deberia hacer fuera de la funcion, despues de la actividad que tenia que hacer

void entrenador_consumir_N_cpu(entrenador*unEntrenador, numero cantidad){
	int i;
	for(i=1; i<cantidad; i++){
		entrenador_esperar_y_consumir_cpu(unEntrenador);
		consumir_ciclo_cpu();
		sem_post(&FinDeCiclo_CPU);
	}

	entrenador_esperar_y_consumir_cpu(unEntrenador);
}

bool entrenador_esta_ejecutando(entrenador*unEntrenador){
	pthread_mutex_lock(&mutexEstadoEntrenador[unEntrenador->id]);
	bool estaEjecutando = entrenador_en_estado(unEntrenador, EXECUTE);
	pthread_mutex_unlock(&mutexEstadoEntrenador[unEntrenador->id]);
	return estaEjecutando;
}

bool entrenador_termino_de_ejecutar(entrenador*unEntrenador){
	bool termino = unEntrenador && !entrenador_esta_ejecutando(unEntrenador);
	return termino;
}

void ejecutar_entrenador(entrenador* unEntrenador){
	numero tiempo;
	entrenador_pasar_a(unEntrenador, EXECUTE, "Es su turno de ejecutar");

	for(tiempo=0; !entrenador_termino_de_ejecutar(unEntrenador); tiempo+=RETARDO_CICLO_CPU){

		if(!entrenador_puede_seguir_ejecutando_segun_algoritmo(unEntrenador, tiempo)){
			entrenador_pasar_a(unEntrenador, READY, "Ha sido desalojado por algoritmo de planificacion");
			break;
		}

		puts("***************************************** LE DOY CPU");
		sem_post(&EjecutarEntrenador[unEntrenador->id]);

		puts("***************************************** ESPERO INTERESADOS");

		sem_wait(&FinDeCiclo_CPU);//		sem_wait(&ConsumioCpu);
		puts("***************************************** CONSUMIO CPU");

	}
}

//{
//	switch(algoritmo){
//		case FIFO: ;
//		/*no break*/
//		case SJF_SD: ;
//			return true;
//
////		case ROUND_ROBBIN: {
////			return tiempo<datosAlgoritmo.QUANTUM || cr_list_is_empty(entrenadoresReady);
////			break;
////		}
//
////		case SJF_CD{
////			return tiempo <= menor_estimacion(entrenadoresReady); //resolver con fold
////		}
//
//		default : {
//			puts("El algoritmo leido no se encuentra implementado... aun! ;)");
//			exit(1);
//			return false;
//		}
//	}
//}

//TODO ver como usar orden superior y asignar una sola funcion al momento de setear algoritmo
//entrenador* proximoAEjecutar(cola_entrenadores colaReady){ //volver a lista con mutex propio
//
//	entrenador*semilla = cr_list_wait_and_remove(colaReady, 0);
//
//	switch(ALGORITMO_PLANIFICACION){
//		case FIFO: ;
//		/*no break*/
//		case ROUND_ROBBIN ;
//			return semilla;
//
//		default: {
//
//			void*entrenador_con_menor_tiempo(entrenador*unEntrenador, entrenador*otro){
//				return
//			}
//
//			return cr_list_fold(colaReady, semilla, );
//		}
//	}
//}

//************************************************************************************
//Ejemplo de uso del cpu

void desplazar_unidimensional(coordenada* posicionInicial, coordenada posicionFinal);
void entrenador_dar_un_paso_hacia(entrenador*unEntrenador, t_posicion posicionFinal);
void entrenador_ir_hacia(entrenador* unEntrenador, t_posicion posicionFinal){
	printf("El entrenador partio de la posicion [%u %u]\n", unEntrenador->posicion.pos_x, unEntrenador->posicion.pos_y);

	bool llegoALaPosicion = entrenador_llego_a(unEntrenador, posicionFinal);

	do{
		puts("**********voy a dar un paso");

		pthread_mutex_lock(&mutexPosicionEntrenador[unEntrenador->id]);
		entrenador_dar_un_paso_hacia(unEntrenador, posicionFinal);
		llegoALaPosicion = entrenador_llego_a(unEntrenador, posicionFinal);
		pthread_mutex_unlock(&mutexPosicionEntrenador[unEntrenador->id]);

		puts("di un paso");

		sem_post(&FinDeCiclo_CPU);
		entrenador_esperar_y_consumir_cpu(unEntrenador);

	}while (!llegoALaPosicion);
}

void desplazar_unidimensional(coordenada* posicionInicial, coordenada posicionFinal){
	int desplazamiento = (posicionFinal > *posicionInicial) - (posicionFinal < *posicionInicial);
	*posicionInicial += desplazamiento;
}

void entrenador_dar_un_paso_hacia(entrenador*unEntrenador, t_posicion posicionFinal){

	t_posicion* posicionActual = &unEntrenador->posicion;

	if(posicionActual->pos_x != posicionFinal.pos_x){
		desplazar_unidimensional(&posicionActual->pos_x, posicionFinal.pos_x);
	}

	else if(posicionActual->pos_y != posicionFinal.pos_y){
		desplazar_unidimensional(&posicionActual->pos_y, posicionFinal.pos_y);
	}

	pthread_mutex_lock(&Mutex_AndoLoggeando);
	log_info(logger, "\nEl entrenador N°%u se desplazo a la posicion [%u %u]\n", unEntrenador->id, unEntrenador->posicion.pos_x, unEntrenador->posicion.pos_y);
	pthread_mutex_unlock(&Mutex_AndoLoggeando);

}
