/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _ENTRY_C
#define _ENTRY_C /* Módulo entry */

#include "entry.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Função que cria uma entry, reservando a memória necessária e
 * inicializando-a com a string e o bloco de dados de entrada.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_create(char *key, struct data_t *data)
{
    struct entry_t *entry_new;

    // Check for null values in the provided data

    if (data == NULL || key == NULL)
        return NULL;

    entry_new = (struct entry_t *)malloc(sizeof(struct entry_t));

    // Verify if the new entry has been created successfully
    if (entry_new == NULL)
        return NULL;

    /* Assign the values received in the function to the new entry,
     but do not allocate new space for the data part of data_t;
     instead, redirect the pointer to the original data */
    entry_new->key = key;
    entry_new->value = data;

    return entry_new;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_destroy(struct entry_t *entry)
{
    // Check if entry is not NULL to avoid freeing unnecessary memory
    if (entry == NULL || entry->key == NULL || entry->value == NULL)
        return -1;

    data_destroy(entry->value);
    free(entry->key);
    free(entry);
    return 0;
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 * Retorna a nova entry ou NULL em caso de erro.
 */
struct entry_t *entry_dup(struct entry_t *entry)
{
    // Verify all data we wish to copy exists
    if (entry == NULL || entry->key == NULL || entry->value == NULL)
        return NULL;

    struct data_t *data_duplicate = data_dup(entry->value);

    struct entry_t *entry_created = entry_create(strdup(entry->key), data_duplicate);

    return entry_created;
}

/* Função que substitui o conteúdo de uma entry, usando a nova chave e
 * o novo valor passados como argumentos, e eliminando a memória ocupada
 * pelos conteúdos antigos da mesma.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value)
{
    // Check if the new given entry and its key and value are not NULL
    if (entry == NULL || new_key == NULL || new_value == NULL)
        return -1;

    free(entry->key);
    data_destroy(entry->value);
    entry->key = new_key;
    entry->value = new_value;

    return 0;
}

/* Função que compara duas entries e retorna a ordem das mesmas, sendo esta
 * ordem definida pela ordem das suas chaves.
 * Retorna 0 se as chaves forem iguais, -1 se entry1 < entry2,
 * 1 se entry1 > entry2 ou -2 em caso de erro.
 */
int entry_compare(struct entry_t *entry1, struct entry_t *entry2)
{

    if (entry1 == NULL || entry2 == NULL)
    {
        // Invalid input entries
        return -2;
    }

    // Compare the keys using strcmp
    int key_comparison = strcmp(entry1->key, entry2->key);

    if (key_comparison < 0)
    {
        return -1; // entry1 < entry2
    }
    else if (key_comparison > 0)
    {
        return 1; // entry1 > entry2
    }
    else
    {
        return 0; // Keys are equal
    }
}

#endif