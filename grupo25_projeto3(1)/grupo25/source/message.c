/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _MESSAGE_PRIVATE_C
#define _MESSAGE_PRIVATE_C

#include <unistd.h> // For write and read functions
#include <errno.h>  // For error handling
#include <stdio.h>  // For perror function
#include <stdint.h>
#include "message-private.h"

int countWords(char *str)
{
    int words = 0;
    int inWord = 0; // Flag to track if we are inside a word

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            // We are inside a word
            if (!inWord)
            {
                words++;
                inWord = 1;
            }
        }
        else
        {
            // We are not inside a word
            inWord = 0;
        }
    }

    return words;
}

int write_all(int sock, uint8_t  *buf, int len)
{
    int bufsize = len;
    while (len > 0)
    {
        int res =write(sock, buf, len);
        
        if (res < 0)
        {
        
        if (errno == EINTR)continue;
        //perror("write failed:");
        return res; /* Error != EINTR */
        }
        if (res == 0)return res; /* Socket was closed */
        buf += res;
        len -= res;
    }
    return bufsize;
}

int read_all(int sock, uint8_t  *buf, int len)
{
    int bufsize = len;
    while (len > 0)
    {
        int res =read(sock, buf, len);
        
        if (res < 0)
        {
        
        if (errno == EINTR)continue;
        //perror("write failed:");
        return res; /* Error != EINTR */
        }
        if (res == 0)return res; /* Socket was closed */
        buf += res;
        len -= res;
    }
    return bufsize;
}

#endif