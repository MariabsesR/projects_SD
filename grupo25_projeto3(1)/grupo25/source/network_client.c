/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _NETWORK_CLIENT_C
#define _NETWORK_CLIENT_C

#include "client_stub.h"
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"
#include "network_client.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include "message-private.h"

/* Esta função deve:
 * - Obter o endereço do servidor (struct sockaddr_in) com base na
 *   informação guardada na estrutura rtable;
 * - Estabelecer a ligação com o servidor;
 * - Guardar toda a informação necessária (e.g., descritor do socket)
 *   na estrutura rtable;
 * - Retornar 0 (OK) ou -1 (erro).
 */
int network_connect(struct rtable_t *rtable)
{
    if (rtable == NULL || rtable->server_address == NULL || rtable->server_port <= 0 || rtable->sockfd <= 0)
    {
        return -1; // Invalid input or uninitialized rtable
    }

    // Save socket value
    int sockfd = rtable->sockfd;
    if (sockfd < 0)
    {
        return -1;
    }
    // sets up a up a struct sockaddr_in
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    // sets the address family of server_addr to AF_INET, indicating that it's an IPv4 address structure.
    server_addr.sin_family = AF_INET;
    // sets the port number for the server address.
    // It uses the htons function to convert the port number from host byte order to network byte order (big-endian).
    // Network byte order is important for consistency in cross-platform communication
    server_addr.sin_port = htons(rtable->server_port);
    // converting a human-readable IPv4 address (in the rtable->server_address variable) to a binary format and storing it in the server_addr.sin_addr field
    if (inet_pton(AF_INET, rtable->server_address, &server_addr.sin_addr) <= 0)
    {
        close(sockfd);
        return -1;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        close(sockfd);
        return -1;
    }

    rtable->sockfd = sockfd; // Store the socket descriptor

    return 0; // Success
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtable_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Tratar de forma apropriada erros de comunicação;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
MessageT *network_send_receive(struct rtable_t *rtable, MessageT *msg)
{

    if (rtable == NULL || rtable->sockfd == -1 || msg == NULL)
    {
        return NULL; // Invalid input or uninitialized rtable
    }

    // Serialize the message
    size_t serialized_len = message_t__get_packed_size(msg);
    uint8_t *serialized_msg = malloc(serialized_len);
    if (serialized_msg == NULL)
    {
        return NULL;
    }
    message_t__pack(msg, serialized_msg);

    int nbytes;

    // Determine the buffer size for sending the serialized message
    // uint16_t is an unsigned 16-bit integer, which guarantees a fixed size of 16 bits (2 bytes).
    // This size is common in many network protocols for representing data lengths or fields.
    uint16_t network_serialized_len = (uint16_t)serialized_len;
    uint16_t network_serialized_len_n = htons(network_serialized_len); // Convert to network byte order
   
    // Send the size (2-byte short) first
    if ((nbytes = write(rtable->sockfd, &network_serialized_len_n, sizeof(uint16_t))) != sizeof(uint16_t))
    {
        free(serialized_msg);
        close(rtable->sockfd);
        return NULL;
    }

    // Send the serialized message buffer
    if ((nbytes = write_all(rtable->sockfd, serialized_msg, serialized_len) != serialized_len))
    {
        free(serialized_msg);
        close(rtable->sockfd);
        return NULL;
    }
   
    // Receive the response buffer size
    uint16_t network_response_size;
    if ((nbytes = read(rtable->sockfd, &network_response_size, sizeof(uint16_t)) == -1))
    {
        close(rtable->sockfd);
        free(serialized_msg);
        return NULL;
    }

    // Convert the received size from network byte order to host byte order
    uint16_t response_size = ntohs(network_response_size);

    // Allocate a buffer to receive the response
    uint8_t *response_buffer = malloc(response_size);
    if (response_buffer == NULL || response_size==0)
    {
        close(rtable->sockfd);
        free(serialized_msg);
        return NULL;
    }

    // Receive the response data
    if ((nbytes = read_all(rtable->sockfd, response_buffer, response_size)) == -1)

    {
        close(rtable->sockfd);
        free(serialized_msg);
        free(response_buffer);
        return NULL;
    }

    // De-serialize the response into a MessageT structure
    MessageT *response_msg = message_t__unpack(NULL, response_size, response_buffer); 

    // Clean up memory
    free(serialized_msg);
    free(response_buffer);

    return response_msg;
}

/* Fecha a ligação estabelecida por network_connect().
 * Retorna 0 (OK) ou -1 (erro).
 */
int network_close(struct rtable_t *rtable)
{
    if (rtable == NULL || rtable->sockfd == -1)
    {
        return -1; // Invalid input or uninitialized rtable
    }

    int close_result = close(rtable->sockfd);
    if (close_result == -1)
    {
        return -1;
    }

    rtable->sockfd = -1; // Reset the socket descriptor
    return 0;           
}

#endif