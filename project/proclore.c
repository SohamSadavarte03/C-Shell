#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <fcntl.h>

#include <grp.h>
#include <time.h>
#include <sys/time.h>

#include "hop.h"
#include "iman.h"

#include "reveal.h"
#include "input.h"
#include "log.h"
#include "proclore.h"

extern char prevdir[4096];
extern char homedir[4096];
extern int lasttime;
extern char *lastcomm;
extern int backcnt;
extern char filedirs[2000][5000];
extern int filecnt;
extern char logarr[15][4096];
extern int log_cnt;
extern int asd;

void proclore(pid_t pid)
{
    char path[256], buffer[1024], *p;
    int fd;
    sprintf(path, "/proc/%d/stat", pid);
    if ((fd = open(path, O_RDONLY)) < 0)
    {
        perror("Error opening stat file");
        // printf("asdh");
        return;
    }

    if (read(fd, buffer, sizeof(buffer)) < 0)
    {
        perror("Error reading stat file");
        close(fd);
        return;
    }

    close(fd);

    char comm[256], state;
    int ppid, pgrp, session, tty_nr, tpgid;
    long long int vsize = 0;
    sscanf(buffer, "%d %s %c %d %d %d %d %d", &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid);

    sprintf(path, "/proc/%d/exe", pid);
    char exe_path[1024];
    ssize_t len = readlink(path, exe_path, sizeof(exe_path) - 1);
    if (len != -1)
    {
        exe_path[len] = '\0';
    }
    else
    {
        strcpy(exe_path, "Unknown");
    }
    sprintf(path, "/proc/%d/status", pid);
    FILE *file = fopen(path, "r");
    char s23[5000];

    while (fgets(s23, sizeof(s23), file))
    {
        if (strncmp(s23, "VmSize:", 7) == 0)
        {
            break;
        }
    }
    fclose(file);
    char *s24=s23+8;
    vsize=atol(s24);
    
    

    char state_str[3];
    state_str[0] = state;
    state_str[1] = '\0';
    if (tpgid == pgrp)
    {
        strcat(state_str, "+");
    }

    printf("pid : %d\n", pid);
    printf("process status : %s\n", state_str);
    printf("Process Group : %d\n", pgrp);
    printf("Virtual memory : %lu KB\n", vsize);
    printf("executable path : %s\n", exe_path);
}
