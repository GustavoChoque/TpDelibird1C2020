/*
 ============================================================================
 Name        : GameCard.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "gamecard.h"

int main(void) {

	inicializar();

	sem_wait(&objetivos_gamecard);
	return EXIT_SUCCESS;
}

void inicializar(void) {

	sem_init(&objetivos_gamecard, 0, 0);

	inicializar_config();
	inicializar_logs();
	crearEstructuras();
	inicializar_conexiones();
}

void inicializar_config() {

	config = config_create("./config/gamecard.config");
}

void inicializar_conexiones() {

	conexion_broker = conexion_server_crear(
			config_get_string_value(config, "IP_BROKER"),
			config_get_string_value(config, "PUERTO_BROKER"), GAMECARD);

	pthread_mutex_init(&mutex_subscripcion, NULL);

	subscribir_y_escuchar_cola_catch_pokemon((void*) mensaje_recibido);
	subscribir_y_escuchar_cola_get_pokemon((void*) mensaje_recibido);
	subscribir_y_escuchar_cola_new_pokemon((void*) mensaje_recibido);

	conectar_gameboy((void*) mensaje_recibido);

}

void inicializar_logs() {

	logger = log_create("./log/gamecard.log", GAMECARD_STRING, 1,LOG_LEVEL_INFO);
	event_logger = log_create("./log/gamecard_event.log", "GAME_CARD_EVENT", 1, LOG_LEVEL_INFO);
}


void crearEstructuras(){

	tiempo_de_reintento_operacion=config_get_int_value(config,"TIEMPO_DE_REINTENTO_OPERACION");
	tiempo_retardo_operacion=config_get_int_value(config,"TIEMPO_RETARDO_OPERACION");
	punto_montaje_tallgrass=config_get_string_value(config,"PUNTO_MONTAJE_TALLGRASS");
	// Inicializo semaforo
	pthread_mutex_init(&mutBitarray, NULL);
	semaforosDePokemons=dictionary_create();

	char* dir_metadata = string_new();
	char* dir_files = string_new();
	char* dir_blocks = string_new();
	char* bin_metadata = string_new();
	char* bin_bitmap = string_new();
	char* bin_metadataFiles = string_new();

	char* bin_block = string_new();

	FILE* f_metadata;

	mkdir(punto_montaje_tallgrass,0777);

	//-------Crecion de Directorios
	string_append(&dir_metadata,punto_montaje_tallgrass);
	string_append(&dir_metadata,"Metadata/");
	mkdir(dir_metadata,0777);
	log_info(event_logger,"Creada carpeta Metadata");

	string_append(&dir_files,punto_montaje_tallgrass);
	string_append(&dir_files,"Files/");
	mkdir(dir_files,0777);
	log_info(event_logger,"Creada carpeta Files");

	string_append(&dir_blocks,punto_montaje_tallgrass);
	string_append(&dir_blocks,"Blocks/");
	mkdir(dir_blocks,0777);
	log_info(event_logger,"Creada carpeta Blocks");

	//-----Creacion del Metadata, si es que no existe
	string_append(&bin_metadata,dir_metadata);
	string_append(&bin_metadata,"/Metadata.bin");

	if((f_metadata=fopen(bin_metadata,"r"))==NULL){ //si no existe el archivo metadata
		log_info(event_logger,"[ERROR FATAL] FILESYSTEM NO ENCONTRADO (se creara uno nuevo)");
		f_metadata=fopen(bin_metadata,"wb+");
		config_metadata=config_create(bin_metadata);
		config_set_value(config_metadata,"BLOCK_SIZE","64");
		config_set_value(config_metadata,"BLOCKS","5192");
		config_set_value(config_metadata,"MAGIC_NUMBER","TALL_GRASS");
		config_save(config_metadata);
	}else
		config_metadata=config_create(bin_metadata);

	fclose(f_metadata);
	log_info(event_logger,"Creado archivo Metadata.bin");

	//---------------Creacion de Bitmap--------------------

	string_append(&bin_bitmap,dir_metadata);
	string_append(&bin_bitmap,"/Bitmap.bin");

	if((f_bitmap=fopen(bin_bitmap,"rb+"))==NULL){// si no existe archivo bitmap
		f_bitmap=fopen(bin_bitmap,"wb+");

		char* bitarray_temp=malloc(tope(config_get_int_value(config_metadata,"BLOCKS"),8));
		fwrite((void*)bitarray_temp,tope(config_get_int_value(config_metadata,"BLOCKS"),8),1,f_bitmap);
		fflush(f_bitmap);
		free(bitarray_temp);

	}
	fseek(f_bitmap, 0, SEEK_END);
	int file_size = ftell(f_bitmap);
	fseek(f_bitmap, 0, SEEK_SET);

	char* bitarray_str=(char*)mmap(NULL,file_size,PROT_READ | PROT_WRITE | PROT_EXEC,MAP_SHARED,fileno(f_bitmap),0);

	if(bitarray_str == (char*) -1) {
			log_error(logger, "Fallo el mmap");
	}

	fread((void*) bitarray_str, sizeof(char), file_size, f_bitmap);
	bitmap = bitarray_create_with_mode(bitarray_str, file_size, MSB_FIRST);

	//seteo de bitmap
	for(int i=0;i<config_get_int_value(config_metadata,"BLOCKS");i++){
		bitarray_clean_bit(bitmap,i);
	}

	log_info(logger, "Creado el archivo Bitmap.bin");

	//-----------------------------

	string_append(&bin_metadataFiles,dir_files);
	string_append(&bin_metadataFiles,"/Metadata.bin");
	if((f_metadata=fopen(bin_metadataFiles,"r"))==NULL){ //si no existe el archivo metadata
			f_metadata=fopen(bin_metadataFiles,"wb+");
			config_metadata_directorio_files=config_create(bin_metadataFiles);
			config_set_value(config_metadata_directorio_files,"DIRECTORY","Y");
			config_save(config_metadata_directorio_files);
		}else
			config_metadata_directorio_files=config_create(bin_metadataFiles);

		fclose(f_metadata);
		config_destroy(config_metadata_directorio_files);
		log_info(event_logger,"Creado archivo Metadata.bin de directorio Files");


	///---------------Verificacion y Creacion de bloques--------
		FILE* f_block;

		string_append(&bin_block,dir_blocks);
		string_append(&bin_block,"/0.bin");

		if((f_block=fopen(bin_block,"r"))==NULL){
		free(bin_block);

		int x;
		for(x=0;x<config_get_int_value(config_metadata,"BLOCKS");x++){
			bin_block = string_new();
			string_append(&bin_block,dir_blocks);
			string_append(&bin_block,string_itoa(x));
			string_append(&bin_block,".bin");
			f_block=fopen(bin_block,"wb+");
			fclose(f_block);
			free(bin_block);
		}
		}else{
			free(bin_block);
			fclose(f_block);
		}

		log_info(event_logger,"Creado los bloques .bin");
	//----------------------
		paths_estructuras[METADATA] = dir_metadata;
		paths_estructuras[FILES] = dir_files;
		paths_estructuras[BLOCKS] = dir_blocks;

	free(bin_metadata);
	free(bin_bitmap);
	free(bin_metadataFiles);

}

void gamecard_New_Pokemon(t_mensaje_new_pokemon* unMsjNewPoke){
	char* dir_unNuevoPokemon = string_new();
	char* bin_metadata = string_new();

	FILE* f_metadata;

	t_config* config_metadata_pokemon;

	string_append(&dir_unNuevoPokemon,paths_estructuras[FILES]);
	string_append(&dir_unNuevoPokemon,unMsjNewPoke->pokemon.especie);
	mkdir(dir_unNuevoPokemon,0777);

	string_append(&bin_metadata,dir_unNuevoPokemon);
	string_append(&bin_metadata,"/Metadata.bin");

	if((f_metadata=fopen(bin_metadata,"r"))==NULL){ //si no existe el archivo metadata
			f_metadata=fopen(bin_metadata,"wb+");

			pthread_mutex_t* mutexMetadataPokemon=malloc(sizeof(pthread_mutex_t));
			pthread_mutex_init(mutexMetadataPokemon, NULL);

			dictionary_put(semaforosDePokemons,unMsjNewPoke->pokemon.especie,mutexMetadataPokemon);

			config_metadata_pokemon=config_create(bin_metadata);
			config_set_value(config_metadata_pokemon,"DIRECTORY","N");
			config_set_value(config_metadata_pokemon,"SIZE","0");
			config_set_value(config_metadata_pokemon,"BLOCKS","[]");
			config_set_value(config_metadata_pokemon,"OPEN","N");
			config_save(config_metadata_pokemon);
		}else{
			//este if es para cuando ya existe el pokemon en disco, pero no su mutex
			if(!dictionary_has_key(semaforosDePokemons,unMsjNewPoke->pokemon.especie)){

				pthread_mutex_t* mutexMetadataPokemon=malloc(sizeof(pthread_mutex_t));
				pthread_mutex_init(mutexMetadataPokemon, NULL);
				dictionary_put(semaforosDePokemons,unMsjNewPoke->pokemon.especie,mutexMetadataPokemon);

			}


			pthread_mutex_lock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));

			config_metadata_pokemon=config_create(bin_metadata);

			pthread_mutex_unlock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));

		}
		fclose(f_metadata);

		pthread_mutex_lock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));
		char* estadoArchivo=config_get_string_value(config_metadata_pokemon,"OPEN");
		pthread_mutex_unlock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));

		//------Ver si el archivo esta abierto------------
		if(strcmp(estadoArchivo,"Y")==0){
			//abro otro hilo con un sleep que volvera a atender al Mensaje

			config_destroy(config_metadata_pokemon);
			free(bin_metadata);
			free(dir_unNuevoPokemon);

			log_info(event_logger,"Esta operacion se reintentara luego: New_Pokemon ::%s ::pos (%i,%i)::cant %i"
									,unMsjNewPoke->pokemon.especie
									,unMsjNewPoke->pokemon.posicion.pos_x
									,unMsjNewPoke->pokemon.posicion.pos_y
									,unMsjNewPoke->cantidad);
			pthread_t unHilo;
			pthread_create(&unHilo, NULL,(void*) gamecard_New_Pokemon_ReIntento, unMsjNewPoke);
			pthread_detach(unHilo);


			//y finalizo este hilo
			pthread_cancel(pthread_self());
			log_info(event_logger,"no se corto el hilo");

		}


		pthread_mutex_lock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));
		config_set_value(config_metadata_pokemon,"OPEN","Y");
		config_save(config_metadata_pokemon);
		pthread_mutex_unlock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));


		char** bloquesDelPokemon=config_get_array_value(config_metadata_pokemon,"BLOCKS");


		//--------Comienzo a operar con el pokemon------------
		if(cant_elemetos_array(bloquesDelPokemon)==0){
			//el bitarray cuenta desde 0
			pthread_mutex_lock(&mutBitarray);
			int nrobloque=bloque_disponible(bitmap,config_get_int_value(config_metadata,"BLOCKS"));
			bitarray_set_bit(bitmap,nrobloque);
			pthread_mutex_unlock(&mutBitarray);

			char* bin_block = string_new();
			string_append(&bin_block,paths_estructuras[BLOCKS]);
			string_append(&bin_block,string_itoa(nrobloque));
			string_append(&bin_block,".bin");



			char* nuevalinea=crearLinea(unMsjNewPoke);
			int longitud=string_length(nuevalinea);

			guardarLinea(bin_block,nuevalinea,longitud);

			char* listaBloques=string_new();
			string_append(&listaBloques,"[");
			string_append(&listaBloques,string_itoa(nrobloque));
			string_append(&listaBloques,"]");


			int size=size_bloque(bin_block);

			//retardo para simular acceso a disco
			sleep(tiempo_retardo_operacion);

			pthread_mutex_lock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));

			config_set_value(config_metadata_pokemon,"BLOCKS",listaBloques);
			config_set_value(config_metadata_pokemon,"SIZE",string_itoa(size));
			config_set_value(config_metadata_pokemon,"OPEN","N");
			config_save(config_metadata_pokemon);

			pthread_mutex_unlock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));


			free(listaBloques);
			free(nuevalinea);
			free(bin_block);

			log_info(event_logger,"no posee bloques, se usara el bloque vacio %i",nrobloque);
		}else{
			/*
			if(){
				//todo buscar  espacio en sus bloques
			}else{
			*/
			//no hay espacio en sus bloques,
			//entonces buscar en el bitmap los bloques libres
			pthread_mutex_lock(&mutBitarray);
			int nrobloque=bloque_disponible(bitmap,config_get_int_value(config_metadata,"BLOCKS"));
			bitarray_set_bit(bitmap,nrobloque);
			pthread_mutex_unlock(&mutBitarray);

			char* bin_block = string_new();
			string_append(&bin_block,paths_estructuras[BLOCKS]);
			string_append(&bin_block,string_itoa(nrobloque));
			string_append(&bin_block,".bin");


			char* nuevalinea=crearLinea(unMsjNewPoke);
			int longitud=string_length(nuevalinea);

			guardarLinea(bin_block,nuevalinea,longitud);

			char* listaBloques=string_new();

			int len_array=cant_elemetos_array(bloquesDelPokemon);

			string_append(&listaBloques,"[");
			for(int i=0;i<len_array;i++){
				string_append(&listaBloques,bloquesDelPokemon[i]);
				string_append(&listaBloques,",");
			}

			string_append(&listaBloques,string_itoa(nrobloque));
			string_append(&listaBloques,"]");


			int size=size_bloque(bin_block)+config_get_int_value(config_metadata_pokemon,"SIZE");

			//retardo para simular acceso a disco
			sleep(tiempo_retardo_operacion);

			pthread_mutex_lock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));

			config_set_value(config_metadata_pokemon,"BLOCKS",listaBloques);
			config_set_value(config_metadata_pokemon,"SIZE",string_itoa(size));
			config_set_value(config_metadata_pokemon,"OPEN","N");
			config_save(config_metadata_pokemon);

			pthread_mutex_unlock(dictionary_get(semaforosDePokemons,unMsjNewPoke->pokemon.especie));


			free(listaBloques);
			free(nuevalinea);
			free(bin_block);


			/*
			}
			*/

			log_info(event_logger,"ya tiene bloques asignados");
		}

		log_info(event_logger,"pokemon guardado:%s ::pos (%i,%i)::cant %i"
				,unMsjNewPoke->pokemon.especie
				,unMsjNewPoke->pokemon.posicion.pos_x
				,unMsjNewPoke->pokemon.posicion.pos_y
				,unMsjNewPoke->cantidad);


		//creacion de  paquete appeared pokemon y envio a Broker

		t_mensaje_appeared_catch_pokemon* mensajeAEnviar=mensaje_appeared_catch_pokemon_crear(unMsjNewPoke->pokemon.especie,unMsjNewPoke->pokemon.posicion.pos_x,unMsjNewPoke->pokemon.posicion.pos_y);
		mensaje_appeared_catch_pokemon_set_id_correlativo(mensajeAEnviar,unMsjNewPoke->mensaje_header.id);

		t_paquete_header header=paquete_header_crear(MENSAJE,GAMECARD,APPEARED_POKEMON);
		t_buffer* bufferDepaquete=mensaje_appeared_catch_pokemon_serializar(mensajeAEnviar);
		t_paquete* paqueteAEnviar=paquete_crear(header,bufferDepaquete);


		t_conexion_server* unaConexion=conexion_server_crear(
					config_get_string_value(config, "IP_BROKER"),
					config_get_string_value(config, "PUERTO_BROKER"), GAMECARD);

		if(enviar(unaConexion,paqueteAEnviar)==ERROR_SOCKET){
			log_warning(logger,"NO se puede realizar la conexion con el BROKER");
		}


		//------------------------
		split_liberar(bloquesDelPokemon);
		config_destroy(config_metadata_pokemon);
		free(bin_metadata);
		free(dir_unNuevoPokemon);

}

void gamecard_Catch_Pokemon(t_mensaje_appeared_catch_pokemon* unMsjCatchPoke){

}
void gamecard_Get_Pokemon(t_mensaje_get_pokemon* unMsjGetPoke){

}

void gamecard_New_Pokemon_ReIntento(t_mensaje_new_pokemon* unMsjNewPoke){
	sleep(tiempo_de_reintento_operacion);
	gamecard_New_Pokemon(unMsjNewPoke);
}


int cant_elemetos_array(char** array){
	int i=0;

	while(array[i]!=NULL){
		i++;
	}

	return i;
}
void split_liberar(char** split){
	unsigned int i = 0;
	for(; split[i] != NULL;i++){
		free(split[i]);
	}
	free(split);
}

int bloque_disponible(t_bitarray* bitmap,int totalBloques){

	int i=0;
	while(i<totalBloques && bitarray_test_bit(bitmap,i)){
		i++;
	}

	return i;
}
void guardarLinea(char* path,char* nuevalinea,int len){
	FILE* f_block;

	f_block=fopen(path,"rb+");

	fseek(f_block,0,SEEK_END);

	fwrite(nuevalinea,sizeof(char),len,f_block);

	fclose(f_block);

}
char* crearLinea(t_mensaje_new_pokemon* unMsjNewPoke ){
	    char* unalinea=string_new();
	    string_append(&unalinea,string_itoa(unMsjNewPoke->pokemon.posicion.pos_x));
		string_append(&unalinea,"-");
		string_append(&unalinea,string_itoa(unMsjNewPoke->pokemon.posicion.pos_y));
		string_append(&unalinea,"=");
		string_append(&unalinea,string_itoa(unMsjNewPoke->cantidad));
		string_append(&unalinea,"\n");
		return unalinea;
}
int size_bloque(char* path){
	FILE* f_block;
	f_block=fopen(path,"rb");
	fseek(f_block, 0, SEEK_END);
	int file_size = ftell(f_block);
	fseek(f_block, 0, SEEK_SET);

	fclose(f_block);

	return file_size;
}
