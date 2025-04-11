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
#include "reveal.h"
#include "input.h"
#include "log.h"
#include "proclore.h"


extern char prevdir[4096];
extern char homedir[4096];
extern int lasttime ;
extern char *lastcomm;
extern int backcnt ;
extern char filedirs[2000][5000];
extern int filecnt;
extern char logarr[15][4096];
extern int log_cnt ;
extern int asd;


void load_log()
{
    FILE *file = fopen("history.log", "r");
    char l[4096];
    while (fgets(l, sizeof(l), file))
    {
        l[strcspn(l, "\n")] = '\0';
        strcpy(logarr[log_cnt], l);
        log_cnt++;
    }
    fclose(file);
}

void save_log(char *comm, char *s)
{
    if (strcmp(logarr[log_cnt - 1], comm) == 0)
    {
        return;
    }
    if (log_cnt >= 15)
    {
        // strcpy(logarr[0],comm);
        for (int i = 0; i < 14; i++)
        {
            strcpy(logarr[i], logarr[i + 1]);
        }
        strcpy(logarr[14], comm);
        chdir(homedir);
        FILE *file = fopen("history.log", "w");

        for (int i = 0; i < 15; i++)
        {
            fprintf(file, "%s\n", logarr[i]);
        }
        fclose(file);
        
        chdir(s);
    }
    else
    {
        strcpy(logarr[log_cnt++], comm);
        chdir(homedir);
        FILE *file = fopen("history.log", "w");

        for (int i = 0; i < log_cnt; i++)
        {
            fprintf(file, "%s\n", logarr[i]);
        }
        fclose(file);
        
        chdir(s);
    }
}

void print_log()
{
    // printf("asdasfSF");
    for (int i = 0; i < log_cnt; i++)
    {
        printf("%s\n", logarr[i]);
    }
}


void log_purge(char *s)
{
    chdir(homedir);
    FILE *file = fopen("history.log", "w");
    fclose(file);
    // char s[4096];
    // getcwd(s, 4096);
    chdir(s);
    log_cnt = 0;
}

void log_execute(int n)
{
    if (n > 15)
    {
        printf("NOT SAVED\n");
    }
    else
    {
        if (n > log_cnt)
        {
            printf("NOT SAVED\n");
        }
        else
        {
            process_input(logarr[log_cnt - n], 1);
        }
    }
}