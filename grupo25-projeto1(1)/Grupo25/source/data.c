/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _DATA_C
#define _DATA_c /* Módulo data */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

/* Função que cria um novo elemento de dados data_t e que inicializa
 * os dados de acordo com os argumentos recebidos, sem necessidade de
 * reservar memória para os dados.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_create(int size, void *data)
{

  // Verifications of null values in needed data
  if (size <= 0 || data == NULL)
    return NULL;

  struct data_t *data_new;
  data_new = (struct data_t *)malloc(sizeof(struct data_t));

  // Verify if the new data has been created
  if (data_new == NULL)
    return NULL;

  // assigns the values received in the function to the new data,
  // does not assign a new space for the data part of data_t, only
  // redirects the pointer to the original
  data_new->datasize = size;
  data_new->data = data;

  if (data_new->data == NULL)
  {
    free(data_new->data); // Free the allocated space if its null
    return NULL;
  }

  return data_new;
};

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int data_destroy(struct data_t *data)
{
  // Checks if data is not NULL so it does not free unecessary memory
  if (data == NULL)
    return -1;

  if (data->data != NULL)
    free(data->data);

  free(data);

  return 0;
};

/* Função que duplica uma estrutura data_t, reservando a memória
 * necessária para a nova estrutura.
 * Retorna a nova estrutura ou NULL em caso de erro.
 */
struct data_t *data_dup(struct data_t *data)
{
  // Verify all data we wish to copy exists
  if (data == NULL || data->data == NULL || data->datasize <= 0)
    return NULL;
  // Copy the value of datasize and create a new space for data
  int size_new = data->datasize;
  void *data_new = malloc(size_new);

  // Copy the data inside the received data to the new space created

  memcpy(data_new, data->data, size_new);

  struct data_t *data_created = data_create(size_new, data_new);

  return data_created;
};

/* Função que substitui o conteúdo de um elemento de dados data_t.
 * Deve assegurar que liberta o espaço ocupado pelo conteúdo antigo.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int data_replace(struct data_t *data, int new_size, void *new_data)
{
  // Check if the new given data and size and the ones inside data are not NULL
  if (data == NULL || data->data == NULL || data->datasize <= 0 || new_data == NULL || new_size <= 0)
    return -1;

  free(data->data); // Free the old data
  data->datasize = new_size;
  data->data = new_data; // Here we are copying the newdata to data->data
  return 0;
}

#endif