/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "network_server.h"
#include "table_skel.h"
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    // Check if the correct number of command line arguments is provided
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <n_lists>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command line arguments to get the port and number of lists
    int port = atoi(argv[1]);
    int n_lists = atoi(argv[2]);

    // Check if the provided port and number of lists are valid
    if (port <= 0 || n_lists <= 0) {
        fprintf(stderr, "Invalid port or num_lists. Both must be positive integers.\n");
        return EXIT_FAILURE;
    }

    // Initialize the table with the specified number of lists
    struct table_t *table = table_skel_init(n_lists);

    // Check if table initialization was successful
    if (table == NULL) {
        fprintf(stderr, "Error: Failed to initialize the table.\n");
        return EXIT_FAILURE;
    }

    // Initialize the server network with the specified port
    int listening_socket = network_server_init(port);

    // Check if server network initialization was successful
    if (listening_socket == -1) {
        fprintf(stderr, "Error: Failed to initialize the server network.\n");
        // Clean up and release resources
        table_skel_destroy(table);
        return EXIT_FAILURE;
    }

    int reuse = 1;
    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        // Handle the error and exit if necessary
    }

    // Entre no loop principal do servidor
    while (1) {
        printf("Server ready, waiting for connections \n");
        // Accept a client connection and handle it using network_main_loop
        network_main_loop(listening_socket, table);
    }

    // Close the server
    int close_result = network_server_close(listening_socket);

    // Check if server closing was successful
    if (close_result == -1) {
        fprintf(stderr, "Error: Failed to close the server.\n");
    }

    // Clean up and release resources
    table_skel_destroy(table);
    network_server_close(listening_socket);

    printf("Server shutdown.\n");
    return EXIT_SUCCESS;
}