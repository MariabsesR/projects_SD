/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

#include <unistd.h> // For write and read functions
#include <errno.h>  // For error handling
#include <stdio.h>  // For perror function

/*
 * Escreve toda a data para o socket, tenta enviar toda a data em buf para o socket especificado, continua a enviar
 * ate todos os byted de data de tamanho len tenham sido enviados ou ate ocorrer um erro
 * 
 *
 * Em caso de sucesso a funcao retorna o total de bytes enviados, em caso de falha, caso o socket nao esteja aberto retorna 0, outros tipos de erro retorna -1
 */
int write_all(int sock, uint8_t  *buf, int len);


/*
 * Le toda a data para do socket, tenta receber toda a data em socket para o buf especificado, continua a receber
 * ate todos os bytes de data de tamanho len tenham sido recebidos ou ate ocorrer um erro
 * 
 *
 * Em caso de sucesso a funcao retorna o total de bytes recebidos, em caso de falha, caso o socket nao esteja aberto retorna 0, outros tipos de erro retorna -1
 */
int read_all(int sock,uint8_t  *buf, int len);

/*
*Counts the words in a string and returns the number of words
*
*/
int countWords(char *str);
#endif