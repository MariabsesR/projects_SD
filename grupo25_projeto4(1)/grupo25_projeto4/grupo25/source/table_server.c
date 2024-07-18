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
#include "client_stub.h"
#include "client_stub-private.h"
#include "network_client.h"
#include "stats.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <zookeeper/zookeeper.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
struct table_t *table; // Declares the table as a global variable
char node_path[1024];  // Saves node path of current server
char node_path_next[1024];
int listening_socket;
char *host_port = NULL;
const char *ip_port = "127.0.0.1:4000";
char *next = NULL;
const char *before = NULL;
static char *watcher_ctx = "ZooKeeper Data Watcher";
typedef struct String_vector zoo_string;
#define ZDATALEN 1024 * 1024

struct rtable_t *rtable;
static zhandle_t *zh;
static int is_connected;

/**
 * Watcher function for connection state change events
 */
void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void *context)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            is_connected = 1;
        }
        else
        {
            is_connected = 0;
        }
    }
}

// Structure to hold the information of a child node
typedef struct
{
    char node_name[256];
    int sequence_number;
} ChildNodeInfo;

// Comparison function for qsort
int compareChildNodes(const void *a, const void *b)
{
    const ChildNodeInfo *nodeA = (const ChildNodeInfo *)a;
    const ChildNodeInfo *nodeB = (const ChildNodeInfo *)b;

    // Extracts the sequence numbers from the node names
    int sequenceA = atoi(nodeA->node_name + strlen(nodeA->node_name) - 10);
    int sequenceB = atoi(nodeB->node_name + strlen(nodeB->node_name) - 10);

    return sequenceA - sequenceB;
}

// Data Watcher function for /MyData node
static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx)
{
    zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));
    
    if (state == ZOO_CONNECTED_STATE)
    {
        if (type == ZOO_CHILD_EVENT)
        {
            // Gets the updated children and reset the watch
            char *root_path = "/chain";
            if (ZOK != zoo_wget_children(wzh, root_path, child_watcher, watcher_ctx, children_list))
            {
                fprintf(stderr, "Error setting watch at %s!\n", root_path);
            }

            // Converts the children_list to an array of ChildNodeInfo for sorting
            ChildNodeInfo *nodeInfoArray = malloc(children_list->count * sizeof(ChildNodeInfo));
            for (int i = 0; i < children_list->count; i++)
            {
                strcpy(nodeInfoArray[i].node_name, children_list->data[i]);
                nodeInfoArray[i].sequence_number = atoi(children_list->data[i] + strlen(children_list->data[i]) - 10);
            }

            // Sorts the array based on sequence numbers
            qsort(nodeInfoArray, children_list->count, sizeof(ChildNodeInfo), compareChildNodes);

            for (int i = 0; i < children_list->count; i++)
            {
                // Checks if node path is a substring of children
                const char *prefix = "/chain/";

                // Allocates memory for the result string
                char *resultString = (char *)malloc(strlen(prefix) + strlen(nodeInfoArray[i].node_name) + 1);

                // Checks if memory allocation is successful
                if (resultString == NULL)
                {
                    fprintf(stderr, "Memory allocation error.\n");
                }

                // Copies the prefix to the result string
                strcpy(resultString, prefix);

                // Concatenates the original string to the result string
                strcat(resultString, nodeInfoArray[i].node_name);

                if (strstr(resultString, node_path) != NULL)
                {
                    char ipAddress[150]; // Assuming IPv4 address
                    int port;

                    if (i > 0)
                    {
                        sscanf(nodeInfoArray[i - 1].node_name, "node-%15[^-]-%d-", ipAddress, &port);
                        char antecessorNode[240];
                        sprintf(antecessorNode, "%s:%d\n\n", ipAddress, port);
                        before = antecessorNode;
                    }
                    if (i < children_list->count - 1)
                    {
                        sscanf(nodeInfoArray[i + 1].node_name, "node-%15[^-]-%d-", ipAddress, &port);

                        char successorNode[240];
                        sprintf(successorNode, "%s:%d\n\n", ipAddress, port);
                        update_next(successorNode);
                    }
                    else
                    {
                        update_next(NULL);
                    }
                }
                free(resultString);
            }

            // Free the memory allocated for the array
            free(nodeInfoArray);
        }
    }
    free(children_list);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

char *get_local_ip()
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("Error getting interface addresses");
        return NULL;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET)
        {
            // Exclude loopback interface
            if (strcmp(ifa->ifa_name, "lo") != 0)
            {
                char *result = strdup(inet_ntoa(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr));
                freeifaddrs(ifaddr);

                return result;
            }
        }
    }

    freeifaddrs(ifaddr);
    fprintf(stderr, "Error getting IP address\n");
    return NULL;
}

void sigpipe_handler(int signo) // Handles the ctrl-c
{
    update_next(NULL);
    zookeeper_close(zh);
    table_skel_destroy(table);
    network_server_close(listening_socket);
    free_stats();

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    // Checks if the correct number of command line arguments is provided
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <port> <n_lists> <host_port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parses command line arguments to get the port, number of lists, and host port
    int port = atoi(argv[1]);
    int n_lists = atoi(argv[2]);
    host_port = argv[3];

    // Checks if the provided port and number of lists are valid
    if (port <= 0 || n_lists <= 0)
    {
        fprintf(stderr, "Invalid port or num_lists. Both must be positive integers.\n");
        return EXIT_FAILURE;
    }

    // Connect to ZooKeeper server
    zh = zookeeper_init(host_port, connection_watcher, 2000, 0, NULL, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
        exit(EXIT_FAILURE);
    }
    sleep(3); // Waits for it to be connected

    // Initializes the table with the specified number of lists
    table = table_skel_init(n_lists);

    // Checks if table initialization was successful
    if (table == NULL)
    {
        fprintf(stderr, "Error: Failed to initialize the table.\n");
        return EXIT_FAILURE;
    }

    // Initializes the server network with the specified port
    int listening_socket = network_server_init(port);

    // Checks if server network initialization was successful
    if (listening_socket == -1)
    {
        fprintf(stderr, "Error: Failed to initialize the server network.\n");
        // Cleans up and releases resources
        table_skel_destroy(table);
        return EXIT_FAILURE;
    }

    int reuse = 1;
    if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        // Handles the error and exit if necessary
    }
    signal(SIGINT, sigpipe_handler);

    if (is_connected)
    {
        // Creates a ZNode for this server with the ZOO_SEQUENCE flag
        char *local_ip = get_local_ip();
        sprintf(node_path, "/chain/node-%s-%s-", local_ip, argv[1]); // Uses server IP port as part of the ZNode path

        // Checks if the parent ZNode exists
        int exists_result = zoo_exists(zh, "/chain", 0, NULL);
        if (exists_result == ZNONODE)
        {
            // Parent ZNode does not exist, creates it
            int create_parent_result = zoo_create(zh, "/chain", NULL, 0, &ZOO_OPEN_ACL_UNSAFE, 0, NULL, 0);
            if (create_parent_result != ZOK)
            {
                fprintf(stderr, "Error creating parent ZNode: %s\n", zerror(create_parent_result));
                exit(EXIT_FAILURE);
            }
        }
        else if (exists_result != ZOK)
        {
            fprintf(stderr, "Error checking parent ZNode existence: %s\n", zerror(exists_result));
            exit(EXIT_FAILURE);
        }
        free(local_ip);
        // Now create the sequential ZNode
        int create_result = zoo_create(zh, node_path, NULL, 0, &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL | ZOO_SEQUENCE, NULL, 0);
        if (create_result != ZOK)
        {
            fprintf(stderr, "Error creating sequential ZNode for the server: %s\n", zerror(create_result));
            exit(EXIT_FAILURE);
        }

        // GETs the next port and the before from thje list of children and the index
        zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));

        // Get the updated children and reset the watch
        char *root_path = "/chain";
        int index = 0;
        if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list))
        {
            fprintf(stderr, "Error setting watch at %s!\n", root_path);
        }

        // Converts the children_list to an array of ChildNodeInfo for sorting
        ChildNodeInfo *nodeInfoArray = malloc(children_list->count * sizeof(ChildNodeInfo));
        for (int i = 0; i < children_list->count; i++)
        {
            strcpy(nodeInfoArray[i].node_name, children_list->data[i]);
            nodeInfoArray[i].sequence_number = atoi(children_list->data[i] + strlen(children_list->data[i]) - 10);
        }
        // Sorts the array based on sequence numbers
        qsort(nodeInfoArray, children_list->count, sizeof(ChildNodeInfo), compareChildNodes);

        // Prints the sorted list
        for (int i = 0; i < children_list->count; i++)
        {
            // Checks if node path is a substring of children
            const char *prefix = "/chain/";

            // Allocates memory for the result string
            char *resultString = (char *)malloc(strlen(prefix) + strlen(nodeInfoArray[i].node_name) + 1);

            // Checks if memory allocation is successful
            if (resultString == NULL)
            {
                fprintf(stderr, "Memory allocation error.\n");
            }

            // Copies the prefix to the result string
            strcpy(resultString, prefix);

            // Concatenates the original string to the result string
            strcat(resultString, nodeInfoArray[i].node_name);

            if (strstr(resultString, node_path) != NULL)
            {
                index = i;
                char ipAddress[150]; // Assuming IPv4 address
                int port;
                sscanf(nodeInfoArray[i - 1].node_name, "node-%15[^-]-%d-", ipAddress, &port);

                if (i > 0)
                {

                    char antecessorNode[240];

                    sprintf(antecessorNode, "%s:%d\n\n", ipAddress, port);
                    before = antecessorNode;
                }
                if (i < children_list->count - 1)
                {
                    sscanf(nodeInfoArray[i + 1].node_name, "node-%15[^-]-%d-", ipAddress, &port);

                    char successorNode[240];
                    sprintf(successorNode, "%s:%d\n\n", ipAddress, port);
                    update_next(successorNode);
                }
            }
            free(resultString);
        }

        // Frees the memory allocated for the array

        free(nodeInfoArray);
        free(children_list);

        if (index > 0)
        {
            rtable = rtable_connect((char *)before);
            if (rtable == NULL)
            {
                return EXIT_FAILURE;
            }
            int connected = network_connect(rtable);
            if (connected == -1)
            {
                free(rtable->server_address);
                free(rtable);
                printf("Error in connect.\n");
                return -1;
            }

            // Creates a message_t message
            MessageT message = MESSAGE_T__INIT; // Initializes with default values

            // Sets the fields of the message
            message.opcode = MESSAGE_T__OPCODE__OP_STATS;
            message.c_type = MESSAGE_T__C_TYPE__CT_NONE;

            // Sends message op_del with the given key and receives a response
            MessageT *response = network_send_receive(rtable, &message);

            struct entry_t **allentrys = rtable_get_table(rtable);

            if (allentrys != NULL)
            {

                int count = 0;

                while (allentrys[count] != NULL)
                {
                    // Converts ProtobufCBinaryData to struct data_t
                    struct data_t value;
                    value.data = allentrys[count]->value->data;
                    value.datasize = allentrys[count]->value->datasize;
                    // Performs the insert operation on the table
                    table_put(table, allentrys[count]->key, &value);

                    count += 1;
                }

                rtable_free_entries(allentrys);
                message_t__free_unpacked(response, NULL);
                rtable_disconnect(rtable);
            }
            else
            {
                printf("Error in rtable_get_table\n");
            }
        }

        while (1)
        {

            printf("Server ready, waiting for connections \n");
            // Accepts a client connection and handles it using network_main_loop
            network_main_loop(listening_socket, table);
        }

        // Closes the server
        int close_result = network_server_close(listening_socket);

        // Checks if server closing was successful
        if (close_result == -1)
        {
            fprintf(stderr, "Error: Failed to close the server.\n");
        }
    }
    // Cleans up and release resources
    table_skel_destroy(table);
    network_server_close(listening_socket);

    printf("Server shutdown.\n");
    return EXIT_SUCCESS;
}