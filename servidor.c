#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>

#define PORTA 9000

int contador;

//Estructura necesaria para acceso excluyente
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *AtenderCliente (void *socket);


int main(int argc, char *argv[]){
	
	int sock_conn, sock_listen;
	struct sockaddr_in serv_adr;
	
	// Inicializamos socket
	if((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("Error creant socket");
	}

	// Fazemos bind
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	
	// Pode receber de qualquer IP
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Porta de escuta
	serv_adr.sin_port = htons(PORTA);

	if(bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0){
		printf ("Error al bind");
	}
	
	if(listen(sock_listen, 3) < 0){
		printf("Error en el Listen");
	}
	
	contador =0;
	int i;
	int sockets[100];
	pthread_t thread;
	i=0;

	// Espera por clientes
	while(1){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		
		sockets[i] =sock_conn;
		
		// Thread para atender cliente
		pthread_create (&thread, NULL, AtenderCliente,&sockets[i]);
		i=i+1;
		
	}

	return 0;
}
	
	
void *AtenderCliente (void *socket){
	
	int sock_conn;
	int *s;
	s= (int *) socket;
	sock_conn= *s;
	
	//int socket_conn = * (int *) socket;
	
	char peticion[512];
	char respuesta[512];
	int ret;
	
	
	int terminar =0;
	// Entramos en un bucle para atender todas las peticiones de este cliente
	//hasta que se desconecte
	while (terminar ==0){
		
		// Ahora recibimos la petici?n
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		
		
		printf ("Peticion: %s\n",peticion);
		
		// vamos a ver que quieren
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p);
		// Ya tenemos el c?digo de la petici?n
		char nombre[20];
		
		if ((codigo !=0)&&(codigo!=4)){
			p = strtok( NULL, "/");
			
			strcpy (nombre, p);
			// Ya tenemos el nombre
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
			
		}if(codigo ==0){ // Solicita desconectar
			terminar=1;
			
		}else if(codigo ==4){
			sprintf (respuesta,"%d",contador);
			
		}else if(codigo ==1){ // Tamanho nome
			sprintf (respuesta,"%d",(int)strlen(nombre));
			
		}else if(codigo ==2){ // Se nome eh bonito
			
			if((nombre[0]=='M') || (nombre[0]=='S')){
				strcpy (respuesta,"SI");
			}else{
				strcpy (respuesta,"NO");
			}
			
		}else{ //quiere saber si es alto
			
			p = strtok( NULL, "/");
			float altura =  atof (p);
			if(altura > 1.70){
				sprintf (respuesta, "%s: eres alto",nombre);
			}else{
				sprintf (respuesta, "%s: eresbajo",nombre);
			}
			
		}if(codigo!=0){
			
			printf ("Respuesta: %s\n", respuesta);
			// Enviamos respuesta
			write (sock_conn,respuesta, strlen(respuesta));
		}
		
		if((codigo ==1)||(codigo==2)|| (codigo==3)){
			
			pthread_mutex_lock(&mutex); // Nao pode interromper
			contador = contador +1;
			pthread_mutex_unlock(&mutex); // Pode interromper
		}
		
	}
	
	// Servico para esse cliente acabou
	close(sock_conn);
	
	return NULL;
}
		
