/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "list.h"

struct table_t {
	struct list_t **lists;
	int size;
};

/* Função que calcula o índice da lista a partir da chave
 */
int hash_code(char *key, int n);

#endif