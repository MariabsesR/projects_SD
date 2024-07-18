/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _NETWORK_SERVER_C
#define _NETWORK_SERVER_C

#include "table.h"
#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "network_server.h"
#include "table_skel.c"
#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <errno.h> // for error handling and error code definitions
#include "message-private.h"
#include <signal.h>

#define MAX_MSG 2048
#define MAX_THREADS 100
int connected_clients = 0;
pthread_t thread_array[MAX_THREADS];
int client_array[MAX_THREADS];
int thread_count = 0;


struct HandleClientArgs
{
    int client_socket;
    struct table_t *table;
};

void handler(int signo) // trata do control+c
{
    if (connected_clients == 0)
    {
        sigpipe_handler(signo);
    }
    else
    {
        for (int i = 0; i < thread_count; i++)
        {
            close(client_array[i]);
        }
        sigpipe_handler(signo);
    }
}

/* Função para preparar um socket de receção de pedidos de ligação
 * num determinado porto.
 * Retorna o descritor do socket ou -1 em caso de erro.
 */
int network_server_init(short port)
{

    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }

    // Initialize the server_addr structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all available network interfaces
    server_addr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sockfd); // Close the socket before returning
        return -1;
    }

    // Set the socket to listen for incoming connections
    if (listen(sockfd, SOMAXCONN) < 0)
    {
        close(sockfd); // Close the socket before returning
        return -1;
    }

    return sockfd; // Return the socket descriptor
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
int network_main_loop(int listening_socket, struct table_t *table)
{
    int client_socket = -1;
    signal(SIGINT, handler);

    while ((client_socket = accept(listening_socket, NULL, NULL)) >= 0)
    {
        if (thread_count >= MAX_THREADS)
        {
            fprintf(stderr, "Maximum number of threads reached. Cannot accept more clients.\n");
            close(client_socket);
            continue;
        }
        struct HandleClientArgs *args = malloc(sizeof(struct HandleClientArgs));
        args->client_socket = client_socket; // Set the client socket
        args->table = table;

        // Increment the number of connected clients

        get_stats()->connected_clients++; // este nao precisa de mutex poisthread principal que trabalha com ele
        printf("Client connection established\n");
        pthread_t thr;
        pthread_create(&thr, NULL, (void *(*)(void *))handle_client, args);
        // Save the thread identifier in the array
        thread_array[thread_count++] = thr;
        client_array[thread_count++] = client_socket;
        pthread_detach(thr);
    }

    return 0;
}

int handle_client(void *args)
{
    struct HandleClientArgs *arg = (struct HandleClientArgs *)args;
    int client_socket = arg->client_socket;
    struct table_t *table = arg->table;

    free(args);

    int valid = 0;
    while (valid == 0)
    {

        // Receive a message from the client.
        MessageT *request = network_receive(client_socket);
        if (request == NULL)
        {
            valid = -1; // Didnt receive a request.
            continue;
        }
        int result;
        result = invoke(request, table);
        if (result != 0)
        {
            request->opcode = MESSAGE_T__OPCODE__OP_ERROR;
            request->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        }

        // Send the response back to the client.
        result = network_send(client_socket, request);
        if (result != 0)
        {
            return -1; // Handle error in sending the response.
        }
        message_t__free_unpacked(request, NULL);
    }
    get_stats()->connected_clients--;
    printf("Client connection closed\n");
    close(client_socket);

    return -1;
}

/* A função network_receive() deve:
 * - Ler os bytes da rede, a partir do client_socket indicado;
 * - De-serializar estes bytes e construir a mensagem com o pedido,
 *   reservando a memória necessária para a estrutura MessageT.
 * Retorna a mensagem com o pedido ou NULL em caso de erro.
 */
MessageT *network_receive(int client_socket)
{
    uint16_t network_response_size = 0;
    int nbytes = -1;

    if ((nbytes = read(client_socket, &network_response_size, sizeof(uint16_t)) == -1))
    {
        return NULL;
    }
 
    if (network_response_size == 0)
    {
        return NULL;
    }
    uint16_t response_size = -1;

    // Convert the received size from network byte order to host byte order
    response_size = ntohs(network_response_size);
  
    if (response_size == 0)
    {
        return NULL;
    }

    // Allocate a buffer to receive the response
    uint8_t *response_buffer = malloc(response_size);
    if (response_buffer == NULL)
    {
        return NULL;
    }

    // Receive the response data
    if ((nbytes = read_all(client_socket, response_buffer, response_size)) == -1)
    {
        free(response_buffer);
        return NULL;
    }

    MessageT *response_msg = message_t__unpack(NULL, response_size, response_buffer);

    if (response_msg == NULL)
    { // Check if the message unpacking failed
        free(response_buffer);
        return NULL;
    }

    if (response_size == 0)
    {
        free(response_buffer);
        return NULL;
    }

    // Clean up memory
    free(response_buffer);
    return response_msg;
}

/* A função network_send() deve:
 * - Serializar a mensagem de resposta contida em msg;
 * - Enviar a mensagem serializada, através do client_socket.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_send(int client_socket, MessageT *msg)
{
    if (client_socket == -1 || msg == NULL)
    {
        return -1; // Invalid input or uninitialized rtable
    }

    // Serialize the message
    size_t serialized_len = message_t__get_packed_size(msg);
    uint8_t *serialized_msg = malloc(serialized_len);
    if (serialized_msg == NULL)
    {
        return -1;
    }
    message_t__pack(msg, serialized_msg);

    int nbytes;

    // Determine the buffer size for sending the serialized message
    uint16_t network_serialized_len = (uint16_t)serialized_len;
    uint16_t network_serialized_len_n = htons(network_serialized_len); // Convert to network byte order

    // Send the size (2-byte short) first
    if ((nbytes = write(client_socket, &network_serialized_len_n, sizeof(uint16_t))) != sizeof(uint16_t))
    {
        free(serialized_msg);
        return -1;
    }

    // Send the serialized message buffer
    if ((nbytes = write_all(client_socket, serialized_msg, serialized_len) != serialized_len))
    {
        free(serialized_msg);
        return -1;
    }

    free(serialized_msg);

    return 0;
}

/* Liberta os recursos alocados por network_server_init(), nomeadamente
 * fechando o socket passado como argumento.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int network_server_close(int socket)
{
    if (close(socket) == 0)
    {

        return 0; // Successfully closed the socket
    }
    else
    {
        return -1; // Failed to close the socket
    }
}
#endif