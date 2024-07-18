/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _SERIALIZATION_H
#define _SERIALIZATION_H

#include "data.h"
#include <stdlib.h>
#include <string.h>
#include "table-private.h"
#include "table.h"
#include "list.h"
#include "table.h"
#include "serialization.h"
#include <arpa/inet.h>


/* Serializa todas as chaves presentes no array de strings keys para o
 * buffer keys_buf, que ser� alocado dentro da fun��o. A serializa��o
 * deve ser feita de acordo com o seguinte formato:
 *    | int   | string | string | string |
 *    | nkeys | key1   | key2   | key3   |
 * Retorna o tamanho do buffer alocado ou -1 em caso de erro.
 */
int keyArray_to_buffer(char **keys, char **keys_buf)
{

    if (keys == NULL || keys_buf == NULL) {
        return -1;
    }


    int num_keys = 0;

    while (keys[num_keys] != NULL)
    {
        num_keys += 1;
    }

    int total_size = 0;

    for (int i = 0; i < num_keys; i++) {
        total_size += strlen(keys[i]) + 1;  // Tamanho da string mais tanmanho dos separadores
    }

    
    *keys_buf = (char *)malloc(sizeof(int) + total_size);

    if (*keys_buf == NULL) {
        return -1;
    }

    /*The htonl() function translates a long integer from host byte order to network byte order.
     The unsigned long integer to be put into network byte order.
     Is typed to the unsigned long integer to be put into network byte order.*/
    // Serialize the number of keys (nkeys) into the buffer
    int nkeys_network_order = htonl(num_keys);
    memcpy(*keys_buf, &nkeys_network_order, sizeof(int));
    
    /*Since keys_buf is a pointer of pointer we need to get the adress inside of it and add
     the size of the num_keys we already put into it so we can get to a free space of memory
     in which to input  the keys*/
    char *current_ptr = *keys_buf + sizeof(int);
    
    //Serialize each key into the buffer
    for (int i = 0; i < num_keys; i++) {
        strcpy(current_ptr, keys[i]);
        current_ptr += strlen(keys[i]) + 1; // +1 to move to the next position
    }

    return sizeof(int) + total_size;

}

/* De-serializa a mensagem contida em keys_buf, colocando-a num array de
 * strings cujo espaco em mem�ria deve ser reservado. A mensagem contida
 * em keys_buf dever� ter o seguinte formato:
 *    | int   | string | string | string |
 *    | nkeys | key1   | key2   | key3   |
 * Retorna o array de strings ou NULL em caso de erro.
 */
char **buffer_to_keyArray(char *keys_buf) {
    
    if (keys_buf == NULL) {
        return NULL;
    }

    char **all_keys;
    int num_keys = 0;

    //Deserialize the number of keys (nkeys) from the buffer
    int transformed_keys;
    memcpy(&transformed_keys, keys_buf, sizeof(int)); //this line gets the first int in the given buffer which is the number of keys in it and copys it to our transformed key  so we may have acces to that int
    
    //We are transforming the bytes we just copied into host bytes so we may use them to know the amount of keys in the buffer
    num_keys = ntohl(transformed_keys); //The ntohl() function converts the unsigned integer netlong from network byte order to host byte order

    //We are getting the size of the pointer multiplied by the number of keys present in the buffer plus 1 that represents \0
    int size = (num_keys + 1) * sizeof(char *);

    all_keys = (char **)malloc(size);

    //We are getting the initial position that keys_buf is pointing to and are adding the size of the already coppied num_keys so we may start copying the keys to a free space in our allocated space
    char *pointer = keys_buf + sizeof(int);

    // Deserialize each key from the buffer
    for (int i = 0; i < num_keys; i++) {
        all_keys[i] = strdup(pointer);
        if (all_keys[i] == NULL) {
            // Memory allocation failed, clean up and return NULL
            for (int j = 0; j < i; j++) {
                free(all_keys[j]);
            }
            free(all_keys);
            return NULL;
        }
        pointer += strlen(all_keys[i]) + 1;
    }

    /*The last key in the array needs to be NULL to indicate the end of the list.
    This is a common convention in C when working with arrays of strings.
    It allows you to iterate through the array of strings until you encounter a NULL pointer,
    signaling that you've reached the end of the list. We can see this convention in the 
    function serialize where we count the num of keys until we find a null.*/
    all_keys[num_keys] = NULL;

    return all_keys;
}
#endif