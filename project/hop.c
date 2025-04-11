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
#include "iman.h"

#include "proclore.h"

extern char prevdir[4096];
extern char homedir[4096];
extern int lasttime;
extern char *lastcomm;
extern int backcnt;

extern char logarr[15][4096];
extern int log_cnt;
extern char filedirs[2000][5000];
extern int filecnt;
extern int asd;

void hop(char *comm)
{
    char t_dir[4096];
    strcpy(t_dir, prevdir);
    // printf("%s ppppp\n", prevdir);
    // getcwd(prevdir, sizeof(prevdir));

    if (strcmp(comm, ".") == 0)
    {
        // getcwd(t_dir,4096);
        getcwd(t_dir, 4096);
        // printf("%s\n", t_dir);

        if (strcmp(t_dir, homedir) == 0)
        {
            // strcpy(path, "~");
            printf("~\n");
        }
        else if (strcmp(t_dir, homedir) > 0)
        {

            printf("~%s\n", t_dir + strlen(homedir));
        }
        else
        {
            printf("%s\n", t_dir);
        }
    }
    else if (strcmp(comm, "..") == 0)
    {
        // chdir("..");

        if (chdir("..") == 0)
        {
            getcwd(t_dir, 4096);
            printf("%s\n", t_dir);
        }
        else
        {
            printf("ERROR: Unable to change directory to '%s'\n", comm);
        }
    }
    else if (strcmp(comm, "-") == 0)
    {
        char s[4096];
        getcwd(s, 4096);
        if (t_dir[0] == '\0')
        {
            printf("No previous directory\n");
            asd=0;
        }
        else
        {
            if (strcmp(s, t_dir) == 0)
            {
                // printf("OLDPWD not set\n");
                printf("%s\n", t_dir);
            }
            else
            {
                if (chdir(t_dir) == 0)
                {
                    printf("%s\n", t_dir);
                }
                else
                {
                    printf("ERROR: Unable to change directory to '%s'\n", comm);
                }
            }
        }
    }
    else if (strcmp(comm, "~") == 0)
    {
        // ;
        // printf("%s\n", homedir);
        if (chdir(homedir) == 0)
        {
            printf("%s\n", homedir);
        }
        else
        {
            printf("ERROR: Unable to change directory to '%s'\n", comm);
        }
    }
    else if (strncmp(comm, "~/", 2) == 0)
    {
        if (comm[1] == '/')
        {
            snprintf(t_dir, sizeof(t_dir), "%s%s", homedir, comm + 1);
            // printf("%s\n",t_dir);
            // printf("%s\n", t_dir);
            if (chdir(t_dir) == 0)
            {
                getcwd(t_dir, 4096);
                printf("%s\n", t_dir);
            }
            else
            {
                printf("ERROR: Unable to change directory to '%s'\n", comm);
            }
        }
    }
    else
    {
        if (comm[0] == '/')
        {

            // chdir(comm);
            // printf("%s\n", comm);
            if (chdir(comm) == 0)
            {
                // getcwd(t_dir, 4096);
                printf("%s\n", comm);
            }
            else
            {
                printf("ERROR: Unable to change directory to '%s'\n", comm);
            }
        }
        else
        {
            getcwd(t_dir, 4096);
            // strcpy(t_dir, homedir);
            strcat(t_dir, "/");
            strcat(t_dir, comm);
            // chdir(t_dir);
            // printf("%s\n", t_dir);
            if (chdir(t_dir) == 0)
            {
                // getcwd(t_dir, 4096);
                printf("%s\n", t_dir);
            }
            else
            {
                printf("ERROR: Unable to change directory to '%s'\n", comm);
            }
        }
    }
}