/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _CLIENT_STUB_C
#define _CLIENT_STUB_C

#include "data.h"
#include "entry.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "network_client.h"
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "sdmessage.pb-c.h" 
// tamanho maximo da mensagem enviada pelo cliente
#define MAX_MSG 2048


/*
struct rtable_t {
    char *server_address;
    int server_port;
    int sockfd;
};*/

/* Função para estabelecer uma associação entre o cliente e o servidor,
 * em que address_port é uma string no formato <hostname>:<port>.
 * Retorna a estrutura rtable preenchida, ou NULL em caso de erro.
 */
struct rtable_t *rtable_connect(char *address_port)
{
    if (address_port == NULL)
    {
        return NULL; // Check for a valid input string
    }

    char *address_port_copy = strdup(address_port);
    //gets the first part that corresponds to the server adress
    char *token = strtok(address_port_copy, ":");

    if (token == NULL)
    {
        free(address_port_copy);
        return NULL; // No tokens found
    }
    //initializes the table
    struct rtable_t *table = (struct rtable_t *)malloc(sizeof(struct rtable_t));
    if (table == NULL)
    {
        free(address_port_copy);
        return NULL; // Memory allocation failed
    }

    // Make a copy of the server address
    char *token_copy = strdup(token);
    //initialize the server adress
    table->server_address = token_copy;
    if (table->server_address == NULL)
    {
        free(table->server_address);
        free(address_port_copy);
        free(table);
        return NULL; // Memory allocation failed
    }
    //gets the second part of adress_port more especifically the port
    token = strtok(NULL, ":");
    if (token == NULL)
    {

        free(address_port_copy);
        free(table->server_address);
        free(table);
        return NULL; // No second token found
    }
    //initializes the port numbe, in case the port number is letters it will return 0 
    table->server_port = atoi(token);
    //creates a socket using the socket function
    //AF_INET: This specifies the address family, in this case, it's AF_INET for IPv4 (Internet Protocol version 4) addresses.
    //SOCK_STREAM: This s0: The protocol parameter. When set to 0, 
    //the system will choose the appropriate protocol based on the combination of address family and socket type.
    //specifies the socket type, in this case, it's a stream socket (TCP)
    //
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {

        free(table->server_address);
        free(table);
        free(address_port_copy);
        return NULL; // Socket creation error
    }

    table->sockfd = sockfd;
    free(address_port_copy);

    return table;
}

/* Termina a associação entre o cliente e o servidor, fechando a
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem, ou -1 em caso de erro.
 */
int rtable_disconnect(struct rtable_t *rtable)
{

    if (rtable == NULL || rtable->server_address == NULL || rtable->server_port < 0)
    {

        return -1;
    }
    
    close(rtable->sockfd);
    // frees all memory associated with struc rtable_t that are char*
    free(rtable->server_address);

    // sockfd doesnt need to be free since its an int
    free(rtable);
    return 0;
}

/* Função para adicionar um elemento na tabela.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Retorna 0 (OK, em adição/substituição), ou -1 (erro).
 */
int rtable_put(struct rtable_t *rtable, struct entry_t *entry)
{
    // Create a MessageT message
    MessageT message = MESSAGE_T__INIT; // Initialize with default values

    // Set the fields of the message
    message.opcode = MESSAGE_T__OPCODE__OP_PUT;
    message.c_type = MESSAGE_T__C_TYPE__CT_ENTRY;

    // Initialize EntryT message
    EntryT entry_message = ENTRY_T__INIT;
    entry_message.key = entry->key;

    // Allocate and copy data for the value field
    entry_message.value.data = (uint8_t *)malloc(entry->value->datasize);
    memcpy(entry_message.value.data, entry->value->data, entry->value->datasize);
    entry_message.value.len = entry->value->datasize;

    // Assign the EntryT message to the MessageT message
    message.entry = &entry_message;

    // Convert the data from the entry into a bytes field in MessageT
    message.value.len = entry_message.value.len;
    message.value.data = entry_message.value.data;

    // Send and receive the message
    MessageT *response = network_send_receive(rtable, &message);
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        free(entry_message.value.data);
        if (response != NULL)

            message_t__free_unpacked(response, NULL);
        return -1;
    }
    // Free dynamically allocated data to prevent memory leaks
    free(entry_message.value.data); // nao sei se é preciso

    message_t__free_unpacked(response, NULL);
    return 0;
}

/* Retorna o elemento da tabela com chave key, ou NULL caso não exista
 * ou se ocorrer algum erro.
 */
struct data_t *rtable_get(struct rtable_t *rtable, char *key)
{

    if (rtable == NULL || rtable->server_address == NULL || rtable->server_port <= 0 || key == NULL)
    {

        return NULL; // Checks if all the inputs are valid
    }

    // Create a message_t message
    MessageT message = MESSAGE_T__INIT; // Initialize with default values

    // Set the fields of the message
    message.opcode = MESSAGE_T__OPCODE__OP_GET;
    message.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    message.key = key;

    // Sends message op_get with the given key and receives a response
    MessageT *response = network_send_receive(rtable, &message);

    // Checks if response received an existing value
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        if (response != NULL)
            message_t__free_unpacked(response, NULL);
        return NULL;
    }
    //creates a new data to return
    void *data_copy = malloc(response->value.len);
    memcpy(data_copy, response->value.data, response->value.len);
    struct data_t *data = data_create(response->value.len, data_copy);
    //frees the return message
    message_t__free_unpacked(response, NULL);

    return data;
}

/* Função para remover um elemento da tabela. Vai libertar
 * toda a memoria alocada na respetiva operação rtable_put().
 * Retorna 0 (OK), ou -1 (chave não encontrada ou erro).
 */
int rtable_del(struct rtable_t *rtable, char *key)
{

    if (rtable == NULL || rtable->server_address == NULL || rtable->server_port <= 0 || key == NULL)
    {
        return -1; // Checks if all the inputs are valid
    }

    // Create a message_t message
    MessageT message = MESSAGE_T__INIT; // Initialize with default values

    // Set the fields of the message
    message.opcode = MESSAGE_T__OPCODE__OP_DEL;
    message.c_type = MESSAGE_T__C_TYPE__CT_KEY;
    message.key = key;

    // Sends message op_del with the given key and receives a response
    MessageT *response = network_send_receive(rtable, &message);

    // Checks if response received an existing value
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        if (response != NULL)
            message_t__free_unpacked(response, NULL);
        return -1;
    }
    message_t__free_unpacked(response, NULL);
    return 0;
}
/* Retorna o número de elementos contidos na tabela ou -1 em caso de erro.
 */
int rtable_size(struct rtable_t *rtable)
{
    if (rtable == NULL || rtable->server_address == NULL || rtable->server_port <= 0)
    {

        return -1; // Checks if all the inputs are valid
    }

    // Create a message_t message
    MessageT message = MESSAGE_T__INIT; // Initialize with default values

    // Set the fields of the message
    message.opcode = MESSAGE_T__OPCODE__OP_SIZE;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    // Sends message op_del with the given key and receives a response
    MessageT *response = network_send_receive(rtable, &message);

    // Checks if response received an existing value
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        if (response != NULL)
            message_t__free_unpacked(response, NULL);
        return -1;
    }
    int size = response->result;

    message_t__free_unpacked(response, NULL);
    return size;
}

/* Retorna um array de char* com a cópia de todas as keys da tabela,
 * colocando um último elemento do array a NULL.
 * Retorna NULL em caso de erro.
 */
char **rtable_get_keys(struct rtable_t *rtable)
{

    if (rtable == NULL || rtable->server_address == NULL || rtable->server_port <= 0)
    {
        return NULL; // Checks if all the inputs are valid
    }

    // Create a message_t message
    MessageT message = MESSAGE_T__INIT; // Initialize with default values

    // Set the fields of the message
    message.opcode = MESSAGE_T__OPCODE__OP_GETKEYS;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    // Sends message op_del with the given key and receives a response
    MessageT *response = network_send_receive(rtable, &message);
    // Checks if response received an existing value
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {
        if (response != NULL)

            message_t__free_unpacked(response, NULL);
        return NULL;
    }
    // Count the number of keys in the response
    int num_keys = response->n_keys;

    char **allkeys = malloc((num_keys + 1) * sizeof(char *)); // +1 for the NULL terminator

    // Copy the keys and add a NULL terminator
    for (int i = 0; i < num_keys; i++)
    {
        allkeys[i] = strdup(response->keys[i]);
    }
    allkeys[num_keys] = NULL;

    message_t__free_unpacked(response, NULL);
    return allkeys;
}

/* Liberta a memória alocada por rtable_get_keys().
 */
void rtable_free_keys(char **keys)
{
    int count = 0;
    while (keys[count] != NULL)
    {

        free(keys[count]);
        count += 1;
    }
    free(keys);
}

/* Retorna um array de entry_t* com todo o conteúdo da tabela, colocando
 * um último elemento do array a NULL. Retorna NULL em caso de erro.
 */
struct entry_t **rtable_get_table(struct rtable_t *rtable)
{
    if (rtable == NULL || rtable->server_address == NULL || rtable->server_port <= 0)
    {
        return NULL; // Checks if all the inputs are valid
    }

    // Create a message_t message
    MessageT message = MESSAGE_T__INIT; // Initialize with default values

    // Set the fields of the message
    message.opcode = MESSAGE_T__OPCODE__OP_GETTABLE;
    message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

    // Sends message op_del with the given key and receives a response
    MessageT *response = network_send_receive(rtable, &message);
    // Checks if response received an existing value
    if (response == NULL || response->opcode == MESSAGE_T__OPCODE__OP_ERROR)
    {

        if (response != NULL)
            message_t__free_unpacked(response, NULL);
        return NULL;
    }
    // Count the number of keys in the response
    int num_entrys = response->n_entries;
    struct entry_t **allentrys = malloc((num_entrys + 1) * sizeof(struct entry_t)); // +1 for the NULL terminator

    // Copy the entrys and adds a NULL terminator
    for (int i = 0; i < num_entrys; i++)
    {
        
        char *save_key = strdup(response->entries[i]->key);
        void *data_data;
        if (response->entries[i]->value.len > 0)
        {

            data_data = malloc(response->entries[i]->value.len); // Allocate memory
            if (data_data)
            {
                memcpy(data_data, response->entries[i]->value.data, response->entries[i]->value.len);

                struct data_t *new_data = data_create(response->entries[i]->value.len, data_data);

                allentrys[i] = entry_create(save_key, new_data);
            }
        }
    }
    allentrys[num_entrys] = NULL;

    message_t__free_unpacked(response, NULL);
    return allentrys;
}

/* Liberta a memória alocada por rtable_get_table().
 */
void rtable_free_entries(struct entry_t **entries)
{
    int count = 0;
    while (entries[count] != NULL)
    {

        entry_destroy(entries[count]);
        count += 1;
    }
    free(entries);
}

#endif
