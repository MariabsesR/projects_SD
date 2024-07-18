/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "table.h"
#include "sdmessage.pb-c.h"
#include "table_skel.h"
#include "client_stub.h"
#include "client_stub-private.h"
#include "sdmessage.pb-c.h"
#include "network_client.h"
#include "stats.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "stats.h"
#include <signal.h>

#include <pthread.h>

/* Define a mutex to synchronize access to put function and conditions */
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;        // mutex that allows changing the table
pthread_cond_t all_finished_reading = PTHREAD_COND_INITIALIZER; // conditional to signal that all clients have finished reading, so the writers wait for all the reading to be done before updating the table
pthread_cond_t all_finished_writing = PTHREAD_COND_INITIALIZER; // conditional to signal that all clients have finished writing, so the readers wait for all the writing before gettign information from the table
int readers_count;
int writers_count;

struct statistics_t *stats;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists)
{
    struct table_t *table = table_create(n_lists);
    if (table == NULL)
    {
        return NULL;
    }
    stats = (struct statistics_t *)malloc(sizeof(struct statistics_t));
    stats->total_operations = 0;
    stats->total_time = 0;
    stats->connected_clients = 0;

    readers_count = 0;
    return table;
}

/* Liberta toda a memória ocupada pela tabela e todos os recursos
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_skel_destroy(struct table_t *table)
{
    if (table == NULL)
    {
        return -1;
    }
    return table_destroy(table);
}

/* Retorna os stats
 */
struct statistics_t *get_stats()
{
    return stats;
}

/* Liberta os stats
 */
void free_stats()
{
    free(stats);
}

/* Aguarda até que todos os clientes tenham terminado as operações de escrita.
 */
void wait_all_clients_done_writing()
{
    pthread_mutex_lock(&write_mutex);
    while (writers_count > 0)
    {
        pthread_cond_wait(&all_finished_writing, &write_mutex);
    }
    pthread_mutex_unlock(&write_mutex);
}

/* Incrementa o contador de leitores.
 */
void increment_readers()
{
    pthread_mutex_lock(&write_mutex);
   
    readers_count += 1;
    pthread_mutex_unlock(&write_mutex);
   
}

/* Decrementa o contador de leitores.
 */
void decrement_readers_and_signal()
{
    pthread_mutex_lock(&write_mutex);
 
    readers_count -= 1;
   
    if (readers_count == 0)
    {
        pthread_cond_signal(&all_finished_reading);
    }
     pthread_mutex_unlock(&write_mutex);
}

/* Incrementa o contador de escritores.
 */
void increment_writers()
{
    pthread_mutex_lock(&write_mutex);
  
    writers_count += 1;
    pthread_mutex_unlock(&write_mutex);
  
}

/* Decrementa o contador de escritores.
 */
void decrement_writers_and_signal()
{
    pthread_mutex_lock(&write_mutex);
  
    writers_count -= 1;

    if (writers_count == 0)
    {
        pthread_cond_broadcast(&all_finished_writing);
    }
    pthread_mutex_unlock(&write_mutex);
}

/* Executa na tabela table a operação indicada pelo opcode contido em msg
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int invoke(MessageT *msg, struct table_t *table)
{

    struct timeval start_time, end_time;
    long time_taken = 0;
    int result = 0; // Initialize the result as OK
    pthread_mutex_lock(&write_mutex);
    stats->total_operations++;
    pthread_mutex_unlock(&write_mutex);
    if (msg == NULL || table == NULL)
    {
        return -1; // Returns an error if the message or table is invalid
    }

    // Use a switch to determine which operation to execute based on the opcode
    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_PUT:
        increment_writers();

        // makes it so only one can write in the table
        pthread_mutex_lock(&write_mutex);

        gettimeofday(&start_time, NULL);
        // Check if msg->entry is valid
        if (msg->entry == NULL || msg->entry->key == NULL)
        {
            result = -1; // Error in message data
        }
        else
        {
            // Convert ProtobufCBinaryData to struct data_t
            struct data_t value;
            value.data = msg->entry->value.data;
            value.datasize = msg->entry->value.len;
            msg->opcode += 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
            // Perform the insert operation on the table
            result = table_put(table, msg->entry->key, &value);
        }
        gettimeofday(&end_time, NULL);
        // unlocks the table and decrement the writers to allow readers to read
        pthread_mutex_unlock(&write_mutex);
        decrement_writers_and_signal();

        break;
    case MESSAGE_T__OPCODE__OP_GET:
        // waits for all clients to be done writing before trying to read
        wait_all_clients_done_writing();
        increment_readers();

        gettimeofday(&start_time, NULL);
        struct data_t *d;
        d = table_get(table, msg->key);
        if (d == NULL)
        {
            result = -1;
        }
        else
        {
            msg->opcode += 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_VALUE;
            msg->value.data = (uint8_t *)d->data;
            msg->value.len = d->datasize;
        }
        free(d);
        gettimeofday(&end_time, NULL);

        decrement_readers_and_signal();

        break;
    case MESSAGE_T__OPCODE__OP_DEL:
        // Perform the delete operation on the table
        increment_writers();
        pthread_mutex_lock(&write_mutex);
        gettimeofday(&start_time, NULL);
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        result = table_remove(table, msg->key);
        pthread_mutex_unlock(&write_mutex);
        decrement_writers_and_signal();
        gettimeofday(&end_time, NULL);
        break;
    case MESSAGE_T__OPCODE__OP_SIZE:
        // Perform the operation to get the table size
        // waits for all clients to be done writing before trying to read
        wait_all_clients_done_writing();
        increment_readers();
        gettimeofday(&start_time, NULL);
        msg->result = table_size(table);
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        gettimeofday(&end_time, NULL);
        decrement_readers_and_signal();
        break;
    case MESSAGE_T__OPCODE__OP_GETKEYS:
        // Perform the operation to get the keys from the table
        // waits for all clients to be done writing before trying to read
        wait_all_clients_done_writing();
        increment_readers();
        gettimeofday(&start_time, NULL);
        char **keys = table_get_keys(table);
        if (keys != NULL)
        {
            size_t num_keys = 0;
            while (keys[num_keys] != NULL)
            {
                num_keys++;
            }
            msg->n_keys = num_keys;
            msg->keys = keys;
            msg->opcode += 1;
            msg->c_type = MESSAGE_T__C_TYPE__CT_KEYS;
        }
        else
        {
            result = -1; // Error in getting keys from the table
        }
        gettimeofday(&end_time, NULL);
        decrement_readers_and_signal();
        break;
    case MESSAGE_T__OPCODE__OP_GETTABLE:
        // waits for all clients to be done writing before trying to read
        wait_all_clients_done_writing();
        increment_readers();
        gettimeofday(&start_time, NULL);
        if (msg->c_type != MESSAGE_T__C_TYPE__CT_NONE)
        {
            result = -1; // Invalid message type
        }
        else
        {
            // Retrieve the keys from the table
            char **keys = table_get_keys(table);
            size_t num_keys = table_size(table);

            // Create an array of pointers to EntryT
            EntryT **entries = (EntryT **)malloc(sizeof(EntryT *) * num_keys);

            if (num_keys > 0)
            {
                struct data_t *d[num_keys];
                for (size_t i = 0; i < num_keys; i++)
                {
                    entries[i] = (EntryT *)malloc(sizeof(EntryT));
                    entry_t__init(entries[i]);
                    entries[i]->key = keys[i];
                    d[i] = table_get(table, keys[i]);
                    entries[i]->value.data = d[i]->data;
                    entries[i]->value.len = d[i]->datasize;
                }

                // Prepare the response message with entries
                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                msg->n_entries = num_keys;
                msg->entries = entries;

                result = 0;
                free(keys);
                for (size_t i = 0; i < num_keys; i++)
                {
                    free(d[i]);
                }
            }
            else
            {
                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                free(keys);
            }
        }
        gettimeofday(&end_time, NULL);
        decrement_readers_and_signal();
        break;
    case MESSAGE_T__OPCODE__OP_STATS:
        pthread_mutex_lock(&write_mutex);

        stats->total_operations--;

        msg->stats = (StatisticsT *)malloc(sizeof(StatisticsT));

        statistics_t__init(msg->stats);

        msg->stats->total_operations = stats->total_operations;
        msg->stats->total_time = stats->total_time;
        msg->stats->connected_clients = stats->connected_clients;
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_STATS;
        pthread_mutex_unlock(&write_mutex);

        break;
    default:
        result = -1; // Invalid opcode
        break;
    }
    pthread_mutex_lock(&write_mutex);
    if (msg->opcode != MESSAGE_T__OPCODE__OP_STATS + 1)
        time_taken = (end_time.tv_sec - start_time.tv_sec) * 1000000L + end_time.tv_usec - start_time.tv_usec;
    stats->total_time += time_taken;
    pthread_mutex_unlock(&write_mutex);
    return result;
}

#endif