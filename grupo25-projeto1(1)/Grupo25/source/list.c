/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _LIST_C
#define _LIST_C

#include <stdlib.h>
#include <string.h>
#include "list-private.h"
#include "list.h"

/* Função que cria e inicializa uma nova lista (estrutura list_t a
 * ser definida pelo grupo no ficheiro list-private.h).
 * Retorna a lista ou NULL em caso de erro.
 */
struct list_t *list_create()
{
    //reserves memory with the sizr of a struct list_t
    struct list_t *list = (struct list_t *)malloc(sizeof(struct list_t));
    if (list == NULL)
    {
        return NULL; // Failed to allocate memory to the list
    }
    //initialize the atributes of list 
    list->size = 0;
    list->head = NULL;

    return list;
}

/* Função auxiliar para libertar um nó da lista e seu conteúdo. No caso de erro retorna -1 se não retorna 0 */
int free_node(struct node_t *node)
{
    //in case the given node is NULL we return to the function that called the auxilarry function 
    if (node == NULL)
        return -1; 
    //free the space allocated for the node 
    //checks if it was able to destroy the entry so it may inform of an error
   int ret =  entry_destroy(node->entry);
   free(node);
   return ret;

}

/* Função que elimina uma lista, libertando *toda* a memória utilizada
 * pela lista.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_destroy(struct list_t *list)
{
    if (list == NULL)
    {
        return -1; 
    }

    // Deletes every node from the list
    struct node_t *current = list->head;
    while (current != NULL)
    {
        struct node_t *next = current->next; //saves the next node before freeing the current so the next may also be destroyed
        free_node(current);
        current = next;
    }

    // Frees the structure of the list.
    free(list);

    return 0; // Successfully destroyed the list.
}

/* Função auxiliar para encontrar a posição correta para inserir a entry na lista */
struct node_t *find_insert_position(struct list_t *list, struct entry_t *entry)
{
    struct node_t *current = list->head;
    struct node_t *previous = NULL;

    while (current != NULL)
    {
        int cmp = entry_compare(entry, current->entry);
        if (cmp <= 0)
            break; //Found the correct position.

        previous = current;   
        current = current->next;  
    }

    return previous;
}

/* Função que adiciona à lista a entry passada como argumento.
 * A entry é inserida de forma ordenada, tendo por base a comparação
 * de entries feita pela função entry_compare do módulo entry e
 * considerando que a entry menor deve ficar na cabeça da lista.
 * Se já existir uma entry igual (com a mesma chave), a entry
 * já existente na lista será substituída pela nova entry,
 * sendo libertada a memória ocupada pela entry antiga.
 * Retorna 0 se a entry ainda não existia, 1 se já existia e foi
 * substituída, ou -1 em caso de erro.
 */
int list_add(struct list_t *list, struct entry_t *entry)
{
    if (list == NULL || entry == NULL)
    {
        return -1; // Invalid arguments.
    }
    struct entry_t *result = list_get(list, entry->key);
    if (result != NULL)
    { // Verifies if there is already an entry with that key in the list
        list_remove(list, result->key);
        list_add(list, entry);
        return 1;
    }
    struct node_t *previous = find_insert_position(list, entry);
    struct node_t *new_node = (struct node_t *)malloc(sizeof(struct node_t));
    if (new_node == NULL)
    {
        return -1; // Failed to allocate memory to the node.
    }

    new_node->entry = entry;
    new_node->next = NULL;

    if (previous == NULL)
    {
        // if it is the smallest entry New entry must be the first of the list.
        new_node->next = list->head;
        list->head = new_node;
    }
    else
    {
        new_node->next = previous->next;
        previous->next = new_node;
    }

    return 0; // Successfully add the entry.
}

/* Função que elimina da lista a entry com a chave key, libertando a
 * memória ocupada pela entry.
 * Retorna 0 se encontrou e removeu a entry, 1 se não encontrou a entry,
 * ou -1 em caso de erro.
 */
int list_remove(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL)
    {
        return -1; // Invalid arguments.
    }

    struct node_t *current = list->head;
    struct node_t *previous = NULL;

    while (current != NULL)
    {
        if (strcmp(current->entry->key, key) == 0)
        {
            // Found the entry with its corresponding key.
            if (previous == NULL)
            {   //if there is no previous node it meanns we have to make the head of the list the head->next
                list->head = current->next;
            }
            else
            {   //if not it associates the next of the rpevious node to the next of the current 
                previous->next = current->next;
            }
            free_node(current); // Frees the entry's memory 
            return 0;           // Successfully removed the entry.
        }

        previous = current;
        current = current->next;
    }

    return 1; // Entry not found in the list.
}

/* Função que obtém da lista a entry com a chave key.
 * Retorna a referência da entry na lista ou NULL se não encontrar a
 * entry ou em caso de erro.
 */
struct entry_t *list_get(struct list_t *list, char *key)
{
    if (list == NULL || key == NULL)
    {
        return NULL; // Invalid arguments.
    }

    struct node_t *current = list->head;

    while (current != NULL)
    {
        if (strcmp(current->entry->key, key) == 0)
        {
            return current->entry; // Found the entry with its corresponding key.
        }
        current = current->next;
    }

    return NULL; // Entry not found in the list.
}

/* Função que conta o número de entries na lista passada como argumento.
 * Retorna o tamanho da lista ou -1 em caso de erro.
 */
int list_size(struct list_t *list)
{
    if (list == NULL)
    {
        return -1; // Invalid argument.
    }

    int size = 0;
    struct node_t *current = list->head;
    //traverses the list until it finds NULL
    while (current != NULL)
    {
        size++;
        current = current->next;
    }

    return size; // Returns the list size.
}

/* Função que constrói um array de char* com a cópia de todas as keys na
 * lista, colocando o último elemento do array com o valor NULL e
 * reservando toda a memória necessária.
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **list_get_keys(struct list_t *list)
{
    if (list == NULL || list->head == NULL)
    {
        return NULL; // Invalid argument.
    }

    int size = list_size(list);
    if (size == -1)
    {
        return NULL; // List size error.
    }
    //allocates space for a pointer-to-pointer, with the size of the number of entrys in the list multyplying by the size of a char* plus 1 to account for \0 in the char* 
    char **keys = (char **)malloc((size + 1) * sizeof(char *));
    if (keys == NULL)
    {
        return NULL; // Failed to allocate memory to the array of keys.
    }

    int i = 0;
    struct node_t *current = list->head;

    while (current != NULL)
    {
        keys[i] = strdup(current->entry->key);
        if (keys[i] == NULL)
        {
            // Failed to allocate memory to one of the keys hence destroying all keys previously allocated 
            for (int j = 0; j < i; j++)
            {
                free(keys[j]);
            }
            free(keys);
            return NULL;
        }
        i++;
        current = current->next;
    }

    keys[size] = NULL; // Last element with value NULL.
    return keys;
}

/* Função que liberta a memória ocupada pelo array de keys obtido pela
 * função list_get_keys.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int list_free_keys(char **keys)
{
    if (keys == NULL)
    {
        return -1; // Invalid argument.
    }

    int i = 0;
    while (keys[i] != NULL)
    {
        free(keys[i]);
        i++;
    }

    free(keys);
    return 0; // Successfully freed the memory.
}



#endif