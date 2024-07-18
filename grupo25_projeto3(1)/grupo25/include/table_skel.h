#ifndef _TABLE_SKEL_H
#define _TABLE_SKEL_H

#include "table.h"
#include "sdmessage.pb-c.h"

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Retorna a tabela criada ou NULL em caso de erro.
 */
struct table_t *table_skel_init(int n_lists);

/* Liberta toda a memória ocupada pela tabela e todos os recursos 
 * e outros recursos usados pelo skeleton.
 * Retorna 0 (OK) ou -1 em caso de erro.
 */
int table_skel_destroy(struct table_t *table);

/* Executa na tabela table a operação indicada pelo opcode contido em msg 
 * e utiliza a mesma estrutura MessageT para devolver o resultado.
 * Retorna 0 (OK) ou -1 em caso de erro.
*/
int invoke(MessageT *msg, struct table_t *table);

/* Retorna os stats
 */
struct statistics_t * get_stats();

/* Liberta os stats
 */
void free_stats();

/* Aguarda até que todos os clientes tenham terminado as operações de escrita.
 */
void wait_all_clients_done_writing();

/* Incrementa o contador de leitores.
 */
void increment_readers();

/* Decrementa o contador de leitores.
 */
void decrement_readers_and_signal();

/* Incrementa o contador de escritores.
 */
void increment_writers();

/* Decrementa o contador de escritores.
 */
void decrement_writers_and_signal();

#endif