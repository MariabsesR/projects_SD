/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#include "stats.h"

/* Initializes the statistics
 */
void initialize_statistics(struct statistics_t*stats) {
    stats->total_operations = 0;
    stats->total_time = 0;
    stats->connected_clients = 0;
}

/* Increments the total number of operations
 */
void increment_total_operations(struct statistics_t *stats){
    stats->total_operations++;
}

/* Adds the time to the statistics
 */
void add_time_to_statistics(struct statistics_t *stats, unsigned long time){
    stats->total_time += time;
}

/* Increments the connected clients
 */
void increment_connected_clients(struct statistics_t *stats){
    stats->connected_clients++;
}

/* Decrements the connected clients
 */
void decrement_connected_clients(struct statistics_t *stats){
    stats->connected_clients--;
}