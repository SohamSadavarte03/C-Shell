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

char homedir[4096];
char prevdir[4096];
int lasttime = -10;
char *lastcomm;
int backcnt = 0;
char filedirs[2000][5000];
int filecnt = 0;
char logarr[15][4096];
int log_cnt = 0;
int asd=-1;

int main()
{
    getcwd(homedir, sizeof(homedir));
    prevdir[0]='\0';
    char s[10000];
    FILE *file = fopen("history.log", "a+");
    fclose(file);
    load_log();

    while (1)
    {
        display();
        if (fgets(s, sizeof(s), stdin) == NULL)
        {
            // break;
        }
        if (strcmp(s, "\n") == 0)
            continue;
        lasttime = -10;
        if(asd==0){
            prevdir[0]='\0';
        }
        if (s[strlen(s) - 1] == '\n')
            s[strlen(s) - 1] = '\0';


        process_input(s, 0);
    }

    return 0;
}