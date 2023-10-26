//network server

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
//extra
#include "network_server.h"
#include "table_skel.h"
#include "sdmessage.pb-c.h"

//tamanho maximo da mensagem enviada pelo cliente
#define MAX_MSG 2048

int is_connected = 0;
/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */
int network_server_init(short port){
    
    int sockfd = socket(AF_INET, SOCK_STREAM,0);
    if (sockfd<0){
        perror("Erro ao criar socket");
        return -1;
    }

    //https://linux.die.net/man/3/setsockopt <- reference
    //https://stackoverflow.com/questions/24194961/how-do-i-use-setsockoptso-reuseaddr
    //this setting is used to make sure local address can be reused
    //SOL_SOCKET:   - set options at socket level (unsure?)
    //SO_REUSEADDR  - address can be reused
    //&(int){1}     - 1 as in enable the previous option
    //sizeof(int)   - specification indicates that this option takes an int value
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int))==-1){
        error("setsockopt(SO_REUSEADDR) failed");
        return -1;
    }

    //handles bind, listening, and finally accept -> reading loop
    struct sockaddr_in server_addr;
    //fill in the struct
    server_addr.sin_family = AF_INET;                   // always set to AF_INET for IP interface
    server_addr.sin_port = htons(port);                 // the port in network byte order
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // binds to all local interfaces

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("Error in Bind()");
        close(sockfd);
        return -1;
    };

    if (listen(sockfd, 5) < 0){ // only accepts 1 client, no request is queued?
        perror("Error in Listen()");
        close(sockfd);
        return -1;
    };

    return sockfd;

}

/* A função network_main_loop() deve:
 * - Aceitar uma conexão de um cliente;
 * - Receber uma mensagem usando a função network_receive;
 * - Entregar a mensagem de-serializada ao skeleton para ser processada
     na tabela table;
 * - Esperar a resposta do skeleton;
 * - Enviar a resposta ao cliente usando a função network_send.
 * A função não deve retornar, a menos que ocorra algum erro. Nesse
 * caso retorna -1.
 */
int network_main_loop(int listening_socket, struct table_t *table){
    // handling SIGPIPE
    // sigaction(SIGPIPE, &(sigaction){sigpipe_handler}, NULL); // CHECKTHIS: should we use sigaction instead?
    // reference: https://stackoverflow.com/questions/18935446/program-received-signal-sigpipe-broken-pipe?noredirect=1&lq=1
    signal(SIGPIPE,signal_handler); //CHECKTHIS: should this be in table_server instead?

    //connect to socket, send/receive
    int connsockfd;
    struct sockaddr_in client_addr;
    while(1){
        if(connsockfd = accept(listening_socket,(struct sockaddr *) &client_addr,sizeof(client_addr))!= -1){
            is_connected = 1;
            while (is_connected){
                //receive a message, deserialize it
                MessageT *msg = network_receive(listening_socket);
                if(!msg){
                    perror("Error in receiving"); //CHECKTHIS should it close socket on error?
                    close(connsockfd);
                    continue;
                }
                //get table_skel to process and get response
                if(msg) invoke(msg,table);
                //wait until response is here?
                if (network_send(listening_socket, msg) == -1){
                    perror("Error in sending");
                    close(connsockfd);
                    continue;
                }
            }
        }
    }
    //close shop baby
    table_skel_destroy(table);
    network_server_close(listening_socket); //does this go first?

}

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket){
    // read short first
    // and then the rest of the socket 

}

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg){

}

/* Liberta os recursos alocados por network_server_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_server_close(int socket){

}


void signal_handler(int signal, int socket) //TODO: add header of this 
{
    if (signal == SIGPIPE){
        printf("Caught SIGPIPE, ignore and continue"); 
        is_connected = 0;
        // since we want it to ignore, eventually we can swap this function out for SIG_IGN instead...
        // however, for the sake of testing,there's a print here. 
        // SIGPIPE: Broken pipe: write to pipe with no readers
    }
}
