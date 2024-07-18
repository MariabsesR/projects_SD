/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client_stub.h"
#include "network_client.h"
#include "client_stub-private.h"
#include "data.h"
#include "entry.h"
#include "message-private.h"
#include "stats.h"
#include <signal.h>
#include <zookeeper/zookeeper.h>

#define MAX_MSG 2048
char node_path[1024]; // saves node path of current server
char node_path_next[1024];
struct rtable_t *head_rtable;
struct rtable_t *tail_rtable;
static zhandle_t *zh;
static int is_connected;
typedef struct String_vector zoo_string;
char *next = NULL;
const char *before = NULL;
static char *watcher_ctx = "ZooKeeper Data Watcher";
#define ZDATALEN 1024 * 1024
char head_ip_port[520];
char tail_ip_port[520];

// In case the server closes the client doesn't do anything
void sigpipe_handler(int signo)
{
}

void ctrl_c_handler(int signo) // Handles the ctrl-c
{
    rtable_disconnect(head_rtable);
    rtable_disconnect(tail_rtable);
    exit(EXIT_FAILURE);
}

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

            rtable_disconnect(head_rtable);
            rtable_disconnect(tail_rtable);
            char *token;
            char copy[256]; // Makes a copy of the original string
            strcpy(copy, nodeInfoArray[0].node_name);

            // Extracts IP address and port using strtok
            token = strtok(copy, "-"); // Gets the first token
            token = strtok(NULL, "-"); // Gets the second token (IP address)

            char ip[256];
            if (token != NULL)
            {
                strcpy(ip, token);
            }
            else
            {
                printf("Failed to extract IP address.\n");
            }

            token = strtok(NULL, "-"); // Gets the third token (port)

            char port[256];
            if (token != NULL)
            {
                strcpy(port, token);
            }
            else
            {
                printf("Failed to extract port.\n");
            }

            // Concatenates IP and port into a single string
            snprintf(head_ip_port, sizeof(head_ip_port), "%s:%s", ip, port);

            // Creates the socket to talk  and in itializes the value in rtable so it may connect to the server
            head_rtable = rtable_connect(head_ip_port);
            // Connects to the server
            int connected = network_connect(head_rtable);
            if (connected == -1)
            {
                free(head_rtable->server_address);
                free(head_rtable);
                printf("Error in connect.\n");
            }

            strcpy(copy, nodeInfoArray[children_list->count - 1].node_name);

            // Extracts IP address and port using strtok
            token = strtok(copy, "-"); // Gets the first token
            token = strtok(NULL, "-"); // Gets the second token (IP address)

            if (token != NULL)
            {
                strcpy(ip, token);
            }
            else
            {
                printf("Failed to extract IP address.\n");
               
            }

            token = strtok(NULL, "-"); // Gets the third token (port)

            if (token != NULL)
            {
                strcpy(port, token);
            }
            else
            {
                printf("Failed to extract port.\n");
               
            }

            // Concatenates IP and port into a single string
            snprintf(tail_ip_port, sizeof(tail_ip_port), "%s:%s", ip, port);

            // Creates the socket to talk and initializes the value in rtable so it may connect to the server
            tail_rtable = rtable_connect(tail_ip_port);
            // Connects to the server
            connected = network_connect(tail_rtable);
            if (connected == -1)
            {
                free(tail_rtable->server_address);
                free(tail_rtable);
                printf("Error in connect.\n");
            }
        }
    }
    free(children_list);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        return EXIT_FAILURE;
    }
    signal(SIGINT, ctrl_c_handler);
    char *server_address_port = argv[1];
    char buffer[100]; // Makes sure it's large enough to hold the concatenated string

    // Uses sprintf to concatenate the strings with a \0
    snprintf(buffer, sizeof(buffer), "%s", server_address_port);

    // Extracts the server address and port from the single argument
    char *server_address = strtok(buffer, ":");
    char *port_str = strtok(NULL, ":");
    if (server_address == NULL || port_str == NULL)
    {
        fprintf(stderr, "Invalid args!\nUsage: table-client <server>:<port>.\n");
        return EXIT_FAILURE;
    }

    // Connects to ZooKeeper server
    zh = zookeeper_init(server_address_port, connection_watcher, 2000, 0, NULL, 0);
    if (zh == NULL)
    {
        fprintf(stderr, "Error connecting to ZooKeeper server!\n");
        exit(EXIT_FAILURE);
    }

    sleep(3); // Waits for it to be connected
    if (is_connected)
    {

        // GETs the next port and the before from the list of children and the index
        zoo_string *children_list = (zoo_string *)malloc(sizeof(zoo_string));

        // Get the updated children and reset the watch
        char *root_path = "/chain";
        
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

        char *token;
        char copy[256]; // Makes a copy of the original string
        strcpy(copy, nodeInfoArray[0].node_name);

        // Extracts IP address and port using strtok
        token = strtok(copy, "-"); // Gets the first token
        token = strtok(NULL, "-"); // Gets the second token (IP address)

        char ip[256];
        if (token != NULL)
        {
            strcpy(ip, token);
        }
        else
        {
            printf("Failed to extract IP address.\n");
            return 1;
        }

        token = strtok(NULL, "-"); // Gets the third token (port)

        char port[256];
        if (token != NULL)
        {
            strcpy(port, token);
        }
        else
        {
            printf("Failed to extract port.\n");
            return 1;
        }

        // Concatenates IP and port into a single string
        snprintf(head_ip_port, sizeof(head_ip_port), "%s:%s", ip, port);

        // Creates the socket to talk  and initializes the value in rtable so it may connect to the server
        head_rtable = rtable_connect(head_ip_port);
        if (head_rtable == NULL)
        {
            return EXIT_FAILURE;
        }
        // Connects to the server
        int connected = network_connect(head_rtable);
        if (connected == -1)
        {
            free(head_rtable->server_address);
            free(head_rtable);
            printf("Error in connect.\n");
            return -1;
        }

        strcpy(copy, nodeInfoArray[children_list->count - 1].node_name);

        // Extracts IP address and port using strtok
        token = strtok(copy, "-"); // Gets the first token
        token = strtok(NULL, "-"); // Gets the second token (IP address)

        if (token != NULL)
        {
            strcpy(ip, token);
        }
        else
        {
            printf("Failed to extract IP address.\n");
            return 1;
        }

        token = strtok(NULL, "-"); // Gets the third token (port)

        if (token != NULL)
        {
            strcpy(port, token);
        }
        else
        {
            printf("Failed to extract port.\n");
            return 1;
        }

        // Concatenates IP and port into a single string
        snprintf(tail_ip_port, sizeof(tail_ip_port), "%s:%s", ip, port);

        // Creates the socket to talk  and initializes the value in rtable so it may connect to the server
        tail_rtable = rtable_connect(tail_ip_port);
        if (tail_rtable == NULL)
        {
            return EXIT_FAILURE;
        }
        // Connects to the server
        connected = network_connect(tail_rtable);
        if (connected == -1)
        {
            free(tail_rtable->server_address);
            free(tail_rtable);
            printf("Error in connect.\n");
            return -1;
        }

        while (1)
        {
            signal(SIGPIPE, sigpipe_handler);
            char command[100];

            printf("Command: ");

            fgets(command, sizeof(command), stdin);

            // Removes the newline character from the input
            command[strcspn(command, "\n")] = '\0';
            // Counts number of words in the given command
            int num_words = countWords(command);

            if (strcmp(command, "quit") == 0)
            {
                if (num_words == 1)
                {
                    // Closes the client connection
                    int disconnect_result = rtable_disconnect(head_rtable);

                    if (disconnect_result != 0)
                    {
                        fprintf(stderr, "Error: Failed to disconnect from the server.\n");
                        free(head_rtable);
                        free(tail_rtable);
                        return -1;
                    }
                    disconnect_result = rtable_disconnect(tail_rtable);

                    if (disconnect_result != 0)
                    {
                        fprintf(stderr, "Error: Failed to disconnect from the server.\n");
                        free(head_rtable);
                        free(tail_rtable);
                        return -1;
                    }
                    printf("Bye, bye!\n");
                    return 0;
                }
                else
                {
                    printf("Invalid arguments. Usage: quit\n");
                }
            }
            else if (strncmp(command, "put ", 4) == 0)
            {
                char *ptr = command + 4; // Moves the pointer to the first character after "put "

                if (num_words >= 3)
                {

                    char *key = NULL;
                    char *data = NULL;

                    char *token = strtok(ptr, " ");
                    if (token != NULL)
                    {
                        // The first token is the key
                        key = strdup(token);

                        // The remaining part is the data
                        token = strtok(NULL, "");
                        if (token != NULL)
                        {
                            data = strdup(token);
                        }
                    }

                    if (key == NULL || data == NULL)
                    {

                        perror("Error scanning either key or data\n");
                        return -1;
                    }

                    struct data_t *data_new = data_create(strlen(data), data);
                    struct entry_t *entry_new = entry_create(key, data_new);

                    if (data == NULL || entry_new == NULL)
                    {

                        perror("Error creating data or entry\n");
                        return -1;
                    }
                    int result = rtable_put(head_rtable, entry_new);
                    if (result == -1)
                        printf("Error in rtable_put!\n");

                    free(data_new->data);
                    free(entry_new->value);
                    free(entry_new);
                    free(key);
                }
                else
                {
                    printf("Invalid arguments. Usage: put <key> <value>\n");
                }
            }
            else if (strncmp(command, "get ", 4) == 0)
            {
                char *ptr = command + 4; // Move the pointer to the first character after "get "

                // Attempt to extract the key
                if (num_words == 2)
                {
                    char *key = NULL;
                    sscanf(ptr, "%ms", &key);

                    if (key == NULL)
                    {
                        perror("Error scanning key\n");
                        return -1;
                    }

                    struct data_t *test = rtable_get(tail_rtable, key);
                    if (test != NULL)
                    {

                        if (test->datasize > 0 && test->data != NULL)
                        {
                            // Converts the binary data to a null-terminated string or else it wont printf
                            char *data_as_string = (char *)malloc(test->datasize + 1);
                            memcpy(data_as_string, test->data, test->datasize);
                            data_as_string[test->datasize] = '\0'; // Null-terminates the string

                            printf("%s\n", data_as_string);
                            data_destroy(test);
                            free(data_as_string);
                        }
                        else
                        {
                            perror("Value Content is empty\n");
                        }
                    }
                    else
                    {
                        printf("Error in rtable_get or key not found!\n");
                    }
                    free(key);
                }
                else
                {
                    printf("Invalid arguments. Usage: get <key>\n");
                }
            }
            else if (strncmp(command, "del ", 4) == 0)
            {
                char *ptr = command + 4; // Moves the pointer to the first character after "get "

                // Attempts to extract the key
                if (num_words == 2)
                {
                    char *key = NULL;
                    sscanf(ptr, "%ms", &key);

                    if (key == NULL)
                    {
                        perror("Error scanning  key\n");
                        return -1;
                    }

                    int result = rtable_del(head_rtable, key);
                    if (result == -1)
                        printf("Error in rtable_del or key not found!\n");
                    else
                        printf("Entry removed\n");
                    free(key);
                }
                else
                {
                    printf("Invalid arguments. Usage: del <key>\n");
                }
            }
            else if (strcmp(command, "size") == 0)
            {
                if (num_words == 1)
                {
                    int result = rtable_size(tail_rtable);
                    if (result == -1)
                    {
                        printf("Error in rtable_size\n");
                    }
                    else
                    {
                        printf("Table size: %d\n", result);
                    }
                }
                else
                {
                    printf("Invalid arguments. Usage: size\n");
                }
            }
            else if (strcmp(command, "getkeys") == 0)
            {
                if (num_words == 1)
                {
                    char **allkeys = rtable_get_keys(tail_rtable);
                    if (allkeys == NULL)
                    {
                        printf("Error in rtable_getkeys\n");
                    }
                    else
                    {
                        if (allkeys != NULL)
                        {
                            int count = 0;
                            while (allkeys[count] != NULL)
                            {
                                printf("%s\n", allkeys[count]);

                                count += 1;
                            }
                            rtable_free_keys(allkeys);
                        }
                    }
                }
                else
                {
                    printf("Invalid arguments. Usage: getkeys\n");
                }
            }
            else if (strcmp(command, "gettable") == 0)
            {
                if (num_words == 1)
                {
                    struct entry_t **allentrys = rtable_get_table(tail_rtable);

                    if (allentrys != NULL)
                    {

                        int count = 0;

                        while (allentrys[count] != NULL)
                        {
                            printf("%s :: ", allentrys[count]->key);
                            // Converts the binary data to a null-terminated string

                            char *data_as_string = (char *)malloc((allentrys[count]->value->datasize) + 1);
                            memcpy(data_as_string, allentrys[count]->value->data, allentrys[count]->value->datasize);
                            data_as_string[allentrys[count]->value->datasize] = '\0';

                            printf("%s\n", data_as_string);

                            free(data_as_string);
                            count += 1;
                        }

                        rtable_free_entries(allentrys);
                    }
                    else
                    {
                        printf("Error in rtable_get_table\n");
                    }
                }
                else
                {
                    printf("Invalid arguments. Usage: gettable\n");
                }
            }
            else if (strcmp(command, "stats") == 0)
            {
                if (num_words == 1)
                {
                    rtable_stats(tail_rtable);
                }
                else
                {
                    printf("Invalid arguments. Usage: stats\n");
                }
            }
            else
            {
                printf("Invalid command.\n");
                printf("Usage: put <key> <value> | get <key> | del <key> | size | getkeys | gettable | stats | quit\n");
            }
        }
    }
}