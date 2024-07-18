/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _TABLE_H
#define _TABLE_H /* Módulo table */

#include "data.h"
#include <stdlib.h>
#include <string.h>
#include "table-private.h"
#include "table.h"
#include "list.h"

struct table_t; /* A definir pelo grupo em table-private.h */

/* Função para criar e inicializar uma nova tabela hash, com n
 * linhas (n = módulo da função hash).
 * Retorna a tabela ou NULL em caso de erro.
 */
struct table_t *table_create(int n)
{

    if (n <= 0)
        return NULL;
    struct table_t *table_new = (struct table_t *)malloc(sizeof(struct table_t));
    if (table_new == NULL) // was not able to allocate space
        return NULL;

    table_new->size = n;                                                     
    table_new->lists = (struct list_t **)malloc(n * sizeof(struct list_t *)); // allocates space for the ammount of lists we need 

    if (table_new->lists == NULL)
    {                           // checks if the memory was allocated
        free(table_new->lists); 
        return NULL;
    }
    for (int i = 0; i < n; i++)
    {
        table_new->lists[i] = list_create();
        if (table_new->lists[i] == NULL)
        {
            // Failure initializing one of the list so it destroys all
            for (int j = 0; j < i; j++)
            {
               int ret = list_destroy(table_new->lists[j]); 
               if(ret==-1)return NULL;
            }
            free(table_new->lists);
            free(table_new);
            return NULL;
        }
    }
    return table_new;
}

/* Função que elimina uma tabela, libertando *toda* a memória utilizada
 * pela tabela.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_destroy(struct table_t *table)
{

    int ret = 0;

    if (table == NULL)
        return -1; 
    for (int i = 0; i < table->size; i++)
    {    //destroys each list before destroying the list of lists and the table
        ret = list_destroy(table->lists[i]);
        if (ret == -1)
            return -1; 
    }
    free(table->lists);
    free(table);
    return 0;
}




/* Função para adicionar um par chave-valor à tabela. Os dados de entrada
 * desta função deverão ser copiados, ou seja, a função vai criar uma nova
 * entry com *CÓPIAS* da key (string) e dos dados. Se a key já existir na
 * tabela, a função tem de substituir a entry existente na tabela pela
 * nova, fazendo a necessária gestão da memória.
 * Retorna 0 (ok) ou -1 em caso de erro.
 *
 */
int table_put(struct table_t *table, char *key, struct data_t *value)
{ 
    if (key == NULL || value == NULL || table == NULL)
        return -1;
     // gets the hash ncode associated with this key so we can search and add in the specific list not needing to search in all list 
    int index = hash_code(key, table->size); // check which list contains this key

  
    struct entry_t *entry_new = entry_create(strdup(key), data_dup(value)); // creates a new key with duplicates of the value and key

    if (entry_new == NULL)
        return -1;

    int ret = list_add(table->lists[index], entry_new); // list_add checks if the entry is in the specific list and removes the old one before adding the new
    if(ret==-1) return -1;
    return 0;
}

/* Função que procura na tabela uma entry com a chave key.
 * Retorna uma *CÓPIA* dos dados (estrutura data_t) nessa entry ou
 * NULL se não encontrar a entry ou em caso de erro.
 */
struct data_t *table_get(struct table_t *table, char *key)
{

    if (table == NULL || key == NULL)
        return NULL;
    struct entry_t *entry_get;
    // gets the hash code associated with the given key so it doesn't require to search the lists of lists, being able to directly going to the associated list 
    int index = hash_code(key, table->size);
    entry_get = list_get(table->lists[index], key); 

    if (entry_get == NULL)
        return NULL;

    return (data_dup(entry_get->value));
}


/* Função que remove da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int table_remove(struct table_t *table, char *key)
{
    if (table == NULL || key == NULL)
        return -1;

    int index = hash_code(key, table->size); // gets the hashcode associated with the specific key
    if (list_get(table->lists[index], key) == NULL)//if the lsit doesnt exist return 1
        return 1;

    int ret =list_remove(table->lists[index], key);
    if(ret==-1) return-1;
    return 0;
}

/* Função que conta o número de entries na tabela passada como argumento.
 * Retorna o tamanho da tabela ou -1 em caso de erro.
 */
int table_size(struct table_t *table)
{
    if (table == NULL)
    {
        return -1;
    }

    int size = 0;

    for (int i = 0; i < table->size; i++)
    {
        size += list_size(table->lists[i]);
    }

    return size;
}

/* Função que constrói um array de char* com a cópia de todas as keys na
 * tabela, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **table_get_keys(struct table_t *table)
{
    if (table == NULL)
    {
        return NULL; // Invalid argument
    }

    int total_entries = table_size(table);
    if (total_entries == -1) 
    {
        return NULL; 
    }
    //allocates memory for a pointer to pointer with the size of the total entrys in the table multiplied by size of char* plus 1 that represents \0 at the end of each char*
    char **keys = (char **)malloc((total_entries + 1) * sizeof(char *));
    if (keys == NULL)
    {
        return NULL; 
    }

    int k = 0;

    // traverses the lists getting the keys to each 
    for (int i = 0; i < table->size; i++)
    {
        char **list_keys = list_get_keys(table->lists[i]); 
        if (list_keys == NULL)
        {
            // if one of the keys is not able to be allocated frees all of them
            for (int j = 0; j < k; j++)
            {
                free(keys[j]);  // destroys the char* pointer that contains each  list of keys
            }
            free(keys);
            return NULL;
        }

        // Copies the keys pointers to the keys meant to be returned
        for (int j = 0; list_keys[j] != NULL; j++)
        {
            keys[k++] = strdup(list_keys[j]);//creates a duplicate
        }

        // frees the memory associated with the keys 
        list_free_keys(list_keys);
    }

    keys[k] = NULL; // Adds a NULL to the end of the array to signify the end of it 
    return keys;
}

/* Função que liberta a memória ocupada pelo array de keys obtido pela
 * função table_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_free_keys(char **keys)
{
    if (keys == NULL)
    {
        return -1; 
    }

    //frees the memory associated with each list of keys
    for (int i = 0; keys[i] != NULL; i++)
    {
        free(keys[i]); // frees keys[i] which is a char*
    }

    free(keys);
    return 0;
}

/* Função que calcula o índice da lista a partir da chave
 */
int hash_code(char *key, int n)
{
    //uses the ASCII value to create an hashcode acording to the side of our table
    int hash = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hash += key[i];
    }
    return hash % n;
}

#endif