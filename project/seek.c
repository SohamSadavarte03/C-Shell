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
#include "iman.h"

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

void seek(char *src, char *target_directory, int d_flag, int f_flag, int e_flag, char *target_directory2)
{
    if (d_flag && f_flag)
    {
        return;
    }

    struct dirent *entry;
    struct stat statbuf;
    DIR *dp = opendir(target_directory);
    if (!dp)
    {
        return 0;
    }

    char path[1024];

    while ((entry = readdir(dp)) != NULL)
    {
        snprintf(path, sizeof(path), "%s/%s", target_directory, entry->d_name);

        if (stat(path, &statbuf) == -1)
        {
            continue;
        }

        if ((d_flag && S_ISDIR(statbuf.st_mode)) || (f_flag && S_ISREG(statbuf.st_mode)) || (!d_flag && !f_flag))
        {

            if (strncmp(entry->d_name, src, strlen(src)) == 0)
            {
               
                strcpy(filedirs[filecnt], path);

                filecnt++;
            }
        }

        if (S_ISDIR(statbuf.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            
            seek(src, path, d_flag, f_flag, e_flag, target_directory2);
        }
    }

    closedir(dp);

}