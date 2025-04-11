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

extern char logarr[15][4096];
extern int log_cnt ;
extern char filedirs[2000][5000];
extern int filecnt;
extern int asd;
extern int alphasort(const struct dirent **a,const struct dirent **b);


void print_permissions(mode_t mode)
{
    char permissions[11];
    permissions[0] = S_ISDIR(mode) ? 'd' : '-';
    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';
    permissions[10] = '\0';

    printf("%s ", permissions);
}


void reveal(int show_hidden, int show_long, char *path)
{
    char target_directory[4096];
    struct stat statbuf;
    
    if (stat(path, &statbuf) == -1)
    {
        perror("stat");
        return;
    }

    if (S_ISREG(statbuf.st_mode)) 
    {
        if (show_long)
        {
            print_permissions(statbuf.st_mode);
            printf("%ld ", statbuf.st_nlink);
            printf("%s ", getpwuid(statbuf.st_uid)->pw_name);
            printf("%s ", getgrgid(statbuf.st_gid)->gr_name);
            printf("%ld ", statbuf.st_size);
            char timed[80];
            strftime(timed, sizeof(timed), "%b %d %H:%M", localtime(&statbuf.st_mtime));
            printf("%s ", timed);
        }
        printf("%s\n", path);
        return;
    }

    if (S_ISDIR(statbuf.st_mode))
    {
        if (strcmp(path, ".") == 0)
        {
            getcwd(target_directory, sizeof(target_directory));
        }
        else if (strcmp(path, "..") == 0)
        {
            getcwd(target_directory, sizeof(target_directory));
            strcat(target_directory, "/..");
        }
        else if (strcmp(path, "~") == 0)
        {
            strcpy(target_directory, homedir);
        }
        else if (strcmp(path, "-") == 0)
        {
            
            strcpy(target_directory, prevdir);

        }
        else if (path[0] == '~')
        {
            snprintf(target_directory, sizeof(target_directory), "%s%s", homedir, path + 1);
        }
        else
        {
            strcpy(target_directory, path);
        }

        struct dirent **namelist;
        int n = scandir(target_directory, &namelist, NULL, alphasort);
        if (n < 0)
        {
            perror("scandir");
            return;
        }

        long long int sum = 0;

        if (show_long)
        {
            for (int i = 0; i < n; i++)
            {
                struct dirent *entry = namelist[i];

                if (!show_hidden && entry->d_name[0] == '.')
                {
                    free(entry);
                    continue;
                }

                char filepath[4096];
                snprintf(filepath, sizeof(filepath), "%s/%s", target_directory, entry->d_name);
                struct stat entry_statbuf;
                if (stat(filepath, &entry_statbuf) == -1)
                {
                    perror("stat");
                    free(entry);
                    continue;
                }
                sum += entry_statbuf.st_size;
            }
            sum /= 1000;
            printf("total %lld\n", sum);
        }

        for (int i = 0; i < n; i++)
        {
            struct dirent *entry = namelist[i];

            if (!show_hidden && entry->d_name[0] == '.')
            {
                continue;
            }

            char filepath[4096];
            snprintf(filepath, sizeof(filepath), "%s/%s", target_directory, entry->d_name);
            struct stat entry_statbuf;
            if (stat(filepath, &entry_statbuf) == -1)
            {
                perror("stat");
                continue;
            }

            if (show_long)
            {
                print_permissions(entry_statbuf.st_mode);
                printf("%ld ", entry_statbuf.st_nlink);
                printf("%s ", getpwuid(entry_statbuf.st_uid)->pw_name);
                printf("%s ", getgrgid(entry_statbuf.st_gid)->gr_name);
                printf("%ld ", entry_statbuf.st_size);
                char timed[80];
                strftime(timed, sizeof(timed), "%b %d %H:%M", localtime(&entry_statbuf.st_mtime));
                printf("%s ", timed);
            }

            if (S_ISDIR(entry_statbuf.st_mode))
            {
                printf("\033[1;34m%s\033[0m\n", entry->d_name);
            }
            else if (S_ISREG(entry_statbuf.st_mode) && (entry_statbuf.st_mode & S_IXUSR))
            {
                printf("\033[1;32m%s\033[0m\n", entry->d_name);
            }
            else
            {
                printf("%s\n", entry->d_name);
            }
        }

        free(namelist);
    }
}
