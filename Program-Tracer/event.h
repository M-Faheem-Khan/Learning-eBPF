#ifndef _MAPS_H
#define _MAPS_H

#define TASK_COMM_LEN 16
#define PATH_MAX 128

struct event {
    char comm[TASK_COMM_LEN];
    char fpath[PATH_MAX];
    pid_t pid;
    uid_t uid;
};

#endif
