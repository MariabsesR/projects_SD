#ifndef STATS_H
#define STATS_H

struct statistics_t {
    unsigned long total_operations;  // Total number of operations
    unsigned long total_time;       // Total time spent in operations (in microseconds)
    int connected_clients;          // Number of currently connected clients
};

/* Initializes the statistics
 */
void initialize_statistics(struct statistics_t *stats);

/* Increments the total number of operations
 */
void increment_total_operations(struct statistics_t *stats);

/* Adds the time to the statistics
 */
void add_time_to_statistics(struct statistics_t *stats, unsigned long time);

/* Increments the connected clients
 */
void increment_connected_clients(struct statistics_t *stats);

/* Decrements the connected clients
 */
void decrement_connected_clients(struct statistics_t *stats);

#endif