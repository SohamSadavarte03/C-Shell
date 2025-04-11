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
#include <signal.h>
#include <grp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include "iman.h"

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
int asd = -1;
int fore_id = -1;

typedef struct Process
{
    pid_t pid;
    char command[256];
    char state[100];
} Process;

Process processes[1000];
int process_count = 0;

void handleBackgroundCompletion(int sig)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WIFEXITED(status))
        {
            printf("Background process %d completed\n", pid);
            if (pid == fore_id)
            {
                fore_id = -1;
            }
        }
        else if (WIFSIGNALED(status))
        {
            printf("Background process %d terminated\n", pid);
        }
        else if (WIFSTOPPED(status))
        {
            printf("Background process %d stopped\n", pid);
        }
        update_process(pid);
    }
}

void handle_sigint(int sig)
{
    if (fore_id != -1)
    {
        printf("%d\n", fore_id);
        kill(-fore_id, SIGINT);
    }
}

void handle_ctrl_d()
{

    for (int i = 0; i < process_count; i++)
    {
        if (strcmp(processes[i].state, "Running") == 0)
        {
            kill(processes[i].pid, SIGKILL);
        }
    }
    exit(0);
}

void sigint_handler(int signal)
{

    printf("%d\n", fore_id);
    kill(fore_id, SIGTSTP);
    update_process(fore_id);

    fore_id = -1;

}

int main()
{

    signal(SIGCHLD, handleBackgroundCompletion);
    signal(SIGINT, handle_sigint);
    getcwd(homedir, sizeof(homedir));
    prevdir[0] = '\0';
    char s[10000];
    FILE *file = fopen("history.log", "a+");
    fclose(file);
    load_log();
    signal(SIGTSTP, sigint_handler);
    while (1)
    {
        display();
        if (fgets(s, sizeof(s), stdin) == NULL)
        {
            handle_ctrl_d();
        }
        if (strcmp(s, "\n") == 0)
            continue;
        lasttime = -10;
        if (asd == 0)
        {
            prevdir[0] = '\0';
        }
        if (s[strlen(s) - 1] == '\n')
            s[strlen(s) - 1] = '\0';


        process_input(s, 0);
    }
    

    return 0;
}