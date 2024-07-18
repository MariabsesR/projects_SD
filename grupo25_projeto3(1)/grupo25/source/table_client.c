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

#define MAX_MSG 2048
 struct rtable_t *rtable ;
// in case the server closes the client doesnt do anything
void sigpipe_handler(int signo)
{
   
}

void ctrl_c_handler(int signo) //takes care of the ctrl-c
{
    rtable_disconnect(rtable);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <server_address:server_port>\n", argv[0]);

        return EXIT_FAILURE;
    }
     signal(SIGINT, ctrl_c_handler);
    char *server_address_port = argv[1];
    char buffer[100]; // Make sure it's large enough to hold the concatenated string

    // Use sprintf to concatenate the strings with a \0
    snprintf(buffer, sizeof(buffer), "%s", server_address_port);

    // Extract the server address and port from the single argument
    char *server_address = strtok(buffer, ":");
    char *port_str = strtok(NULL, ":");
    if (server_address == NULL || port_str == NULL) 
    {
        fprintf(stderr, "Invalid args!\nUsage: table-client <server>:<port>.\n");
        return EXIT_FAILURE;
    }

    // creates the socket to talk  and in itializes the value in rtable so it may connect to the server
    rtable = rtable_connect(server_address_port);
    if (rtable == NULL)
    {
        return EXIT_FAILURE;
    }
    // connect to the server
    int connected = network_connect(rtable);
    if (connected == -1)
    {
        free(rtable->server_address);
        free(rtable);
        printf("Error in connect.\n");
        return -1;
    }

 
    while (1)
    {
        signal(SIGPIPE, sigpipe_handler);
        char command[100];

        printf("Command: ");

        fgets(command, sizeof(command), stdin);
        
        // Remove the newline character from the input
        command[strcspn(command, "\n")] = '\0';
        // counts number of words in the given command
        int num_words = countWords(command);
       
        if (strcmp(command, "quit") == 0)
        {
            if (num_words == 1)
            {
                // Close the client connection
                int disconnect_result = rtable_disconnect(rtable);

                if (disconnect_result != 0)
                {
                    fprintf(stderr, "Error: Failed to disconnect from the server.\n");
                    free(rtable);
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
        else if (strncmp(command, "put ", 4) == 0) // NOTAAAA isto esta errado ele pode conteroutras coisas sem ser strings mas nao sei como  lidar com isso
        {
            char *ptr = command + 4; // Move the pointer to the first character after "put "

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
                int result = rtable_put(rtable, entry_new);
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

                struct data_t *test = rtable_get(rtable, key);
                if (test != NULL)
                {

                    if (test->datasize > 0 && test->data != NULL)
                    {
                        // Convert the binary data to a null-terminated string or else it wont printf
                        char *data_as_string = (char *)malloc(test->datasize + 1);
                        memcpy(data_as_string, test->data, test->datasize);
                        data_as_string[test->datasize] = '\0'; // Null-terminate the string

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
            char *ptr = command + 4; // Move the pointer to the first character after "get "

            // Attempt to extract the key
            if (num_words == 2)
            {
                char *key = NULL;
                sscanf(ptr, "%ms", &key);

                if (key == NULL)
                {
                    perror("Error scanning  key\n");
                    return -1;
                }

                int result = rtable_del(rtable, key);
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
                int result = rtable_size(rtable);
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
                char **allkeys = rtable_get_keys(rtable);
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
                struct entry_t **allentrys = rtable_get_table(rtable);

                if (allentrys != NULL)
                {

                    int count = 0;

                    while (allentrys[count] != NULL)
                    {
                        printf("%s :: ", allentrys[count]->key);
                        // Convert the binary data to a null-terminated string

                        char *data_as_string = (char *)malloc((allentrys[count]->value->datasize) + 1);
                        memcpy(data_as_string, allentrys[count]->value->data, allentrys[count]->value->datasize);
                        data_as_string[allentrys[count]->value->datasize] = '\0';

                        printf("%s\n", data_as_string);

                        free(data_as_string);
                        count += 1;
                    }

                    rtable_free_entries(allentrys);
                }else{
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
                rtable_stats(rtable);
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
