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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>

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

/* Executa na tabela table a operação indicada pelo opcode contido em msg
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int invoke(MessageT *msg, struct table_t *table)
{
    int result = 0; // Initialize the result as OK

    if (msg == NULL || table == NULL)
    {
        return -1; // Returns an error if the message or table is invalid
    }

    // Use a switch to determine which operation to execute based on the opcode
    switch (msg->opcode)
    {
    case MESSAGE_T__OPCODE__OP_PUT:
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
        break;
    case MESSAGE_T__OPCODE__OP_GET:
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
        break;
    case MESSAGE_T__OPCODE__OP_DEL:
        // Perform the delete operation on the table
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_NONE;
        result = table_remove(table, msg->key);
        break;
    case MESSAGE_T__OPCODE__OP_SIZE:
        // Perform the operation to get the table size
        msg->result = table_size(table);
        msg->opcode += 1;
        msg->c_type = MESSAGE_T__C_TYPE__CT_RESULT;
        break;
    case MESSAGE_T__OPCODE__OP_GETKEYS:
        // Perform the operation to get the keys from the table
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
        break;
    case MESSAGE_T__OPCODE__OP_GETTABLE:
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
                    free (d[i]);
                }
            }
            else{
                msg->opcode += 1;
                msg->c_type = MESSAGE_T__C_TYPE__CT_TABLE;
                free(keys);
            }
        }
        break;
    default:
        result = -1; // Invalid opcode
        break;
    }

    return result;
}

#endif