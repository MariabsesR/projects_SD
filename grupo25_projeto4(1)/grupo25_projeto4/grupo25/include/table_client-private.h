/*
Grupo 25:
António Estêvão, Nº 58203
Jacky Xu, Nº 58218
Maria Rocha, Nº 58208
*/

#ifndef _TABLE_CLIENT_PRIVATE_H
#define _TABLE_CLIENT_PRIVATE_H

#include <zookeeper/zookeeper.h>

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void *context);

int compareChildNodes(const void *a, const void *b);

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx);

void sigpipe_handler(int signo);

void ctrl_c_handler(int signo);

#endif