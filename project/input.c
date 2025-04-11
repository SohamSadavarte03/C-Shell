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

extern char homedir[4096];
extern char prevdir[4096];
extern int lasttime;
extern char *lastcomm;
extern int backcnt;
extern char logarr[15][4096];
extern int log_cnt;

extern char filedirs[2000][5000];
extern int filecnt;
extern int asd;

void display()
{
    struct passwd *pass = getpwuid(getuid());
    char *name = pass->pw_name;
    char systname[4096];
    gethostname(systname, 4096);
    char current[4096];
    getcwd(current, 4096);
    char path[4096];

    if (strcmp(current, homedir) == 0)
    {
        strcpy(path, "~");
    }
    else if (strstr(current, homedir) == current)
    {
        sprintf(path, "~%s", current + strlen(homedir));
    }
    else
    {
        strcpy(path, current);
    }
    if (lasttime < 0)
        printf("<%s@%s:%s> ", name, systname, path);
    else
        printf("<%s@%s:%s %s : %ds> ", name, systname, path, lastcomm, lasttime);
}

void execute_command(char *command)
{
    char *argv[100];
    char *tok;
    int c = 0;
    tok = strtok(command, " ");
    while (tok != NULL)
    {
        argv[c++] = tok;

        tok = strtok(NULL, " ");
    }
    argv[c] = NULL;

    if (c == 0)
        return;

    if (execvp(argv[0], argv) == -1)
    {
        // printf("%s\n",argv[0]);
        printf("ERROR: '%s' is not a valid command\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void process_input(char *input, int fl)
{
    char *command;
    char *rest = input;
    char *saveptr1;
    char s2[4096];
    getcwd(s2, 4096);
    if (strstr(rest, "log") == NULL && fl == 0 && strcmp(rest, "\n") != 0)
    {
        save_log(rest, s2);
    }
    while ((command = strtok_r(rest, ";", &saveptr1)) != NULL)
    {
        rest = NULL;
        char *sub_command;
        char *saveptr2;

        int j = 0;
        while (command[j] == ' ')
        {
            j++;
        }
        command += j;
        j = strlen(command) - 1;
        if (j > 0)
            while (command[j] == ' ')
            {
                j--;
            }

        command[j + 1] = '\0';

        int length = strlen(command);
        char *result = (char *)malloc(length * 2);
        j = 0;

        for (int i = 0; i < length; i++)
        {
            result[j++] = command[i];

            if (command[i] == '&')
            {
                result[j++] = '`';
            }
        }
        result[j] = '\0';
        // printf("%s command", result);
        while ((sub_command = strtok_r(result, "`", &saveptr2)) != NULL)
        {
            result = NULL;

            int j = 0;
            while (sub_command[j] == ' ')
            {
                j++;
            }
            sub_command += j;
            j = strlen(sub_command) - 1;
            if (j > 0)
                while (sub_command[j] == ' ')
                {
                    j--;
                }

            sub_command[j + 1] = '\0';

            if (*sub_command == '\0')
                continue;

            if (strchr(sub_command, '&') != NULL)
            {
                // printf("321\n");
                sub_command[strlen(sub_command) - 1] = '\0';
                pid_t pid = fork();
                if (pid < 0)
                {
                    printf("ERROR: Unable to fork\n");
                }
                else if (pid == 0)
                {
                    setpgid(0, 0);
                    pid_t pid1 = fork();
                    if (pid1 < 0)
                    {
                        printf("ERROR: Unable to fork\n");
                    }
                    else if (pid1 == 0)
                    {

                        printf("%d\n", getpid());
                        execute_command(sub_command);
                    }
                    else
                    {

                        // printf("[%d] %d\n", pid, pid);

                        wait(NULL);
                        printf("%s completed normally %d\n", sub_command, pid1);

                        exit(EXIT_SUCCESS);
                    }
                }
                else
                {
                    sleep(1);
                    // printf("[%d] %d\n", ++backcnt, pid);
                }
            }
            else
            {

                if (strncmp(sub_command, "reveal", 6) == 0 || strncmp(sub_command, "hop", 3) == 0 || strncmp(sub_command, "log", 3) == 0 || strncmp(sub_command, "proclore", 8) == 0 || strncmp(sub_command, "seek", 4) == 0)
                {
                    // printf("123\n");
                    main_command(sub_command);
                }
                else
                {
                    pid_t pid = fork();
                    if (pid < 0)
                    {
                        printf("ERROR: Unable to fork\n");
                    }
                    else if (pid == 0)
                    {

                        // printf("%s\n", sub_command);
                        execute_command(sub_command);
                        exit(EXIT_SUCCESS);
                    }
                    else
                    {
                        struct timeval st, end;
                        gettimeofday(&st, NULL);

                        waitpid(pid, NULL, 0);
                        gettimeofday(&end, NULL);
                        int secs = end.tv_sec - st.tv_sec;
                        if (secs > 2)
                        {
                            lasttime = secs;
                            lastcomm = strtok(sub_command, " ");
                        }
                    }
                }
            }
        }
    }
}

void main_command(char *s)
{

    char s2[4096];
    getcwd(s2, 4096);

    if (strncmp(s, "hop", 3) == 0)
    {
        // s+=4;
        // printf("%s ",s);
        char prev1[4096];
        getcwd(prev1, 4096);
        char *s1 = strtok(s + 4, " ");
        if (strcmp(s, "hop") == 0)
        {
            // printf("asdbu");
            hop("~");
        }
        else
        {
            while (s1 != NULL)
            {
                hop(s1);
                s1 = strtok(NULL, " ");
            }
        }
        // if (prevdir[0] == '\0')
        strcpy(prevdir, prev1);

        // char *comm = s + 4;
        // printf("%s ",s);
    }
    else if (strncmp(s, "reveal", 6) == 0)
    {
        int show_hidden = 0, show_long = 0;
        char *path = NULL;
        char *tok = strtok(s + 7, " ");
        while (tok != NULL)
        {
            if (tok[0] == '-')
            {
                for (int i = 1; tok[i] != '\0'; i++)
                {
                    if (tok[i] == 'a')
                        show_hidden = 1;
                    else if (tok[i] == 'l')
                        show_long = 1;
                }
            }
            else
            {
                if (strncmp(tok, "~", 1) == 0)
                {
                    char s22[5000];
                    strcpy(s22, homedir);
                    strcat(s22, tok + 1);
                    path = s22;
                    // strcpy(path,s22);
                }
                else
                {

                    path = tok;
                }
            }
            tok = strtok(NULL, " ");
        }

        if (!path)
        {
            path = ".";
        }
        reveal(show_hidden, show_long, path);
    }
    else if (strncmp(s, "log", 3) == 0)
    {
        // printf("%d\n",strlen(s));
        char *s3 = s;
        int exe = s[strlen(s) - 1] - '0';
        int exe1 = 0;
        if (s[strlen(s) - 2] != ' ')
        {
            exe1 = s[strlen(s) - 2] - '0';
            // exe1+=exe;
            exe1 *= 10;
            // exe1+=exe;
        }
        // exe=exe1;
        exe1 += exe;

        // printf("%d\n",exe1);
        // printf("%s", s);
        char *s1 = strtok(s + 4, " ");
        // printf("%s\n",s+4);
        if (strcmp(s3, "log") == 0)
        {
            // hop("~");
            // printf("asouasd");
            print_log();
        }
        else
        {
            int fl = 0;
            // printf("%s", s);
            while (s1 != NULL)
            {
                // printf("%s\n", s1);

                if (strcmp(s1, "purge") == 0)
                {
                    log_purge(s2);
                    // printf("asdad");
                }
                else if (strcmp(s1, "execute") == 0)
                {
                    fl = 1;
                    // printf("%d",exe);
                }
                // printf("%s\n", s1);
                // else if()
                s1 = strtok(NULL, " ");
            }
            if (fl == 1)
            {
                log_execute(exe1);
            }
        }
    }
    else if (strncmp(s, "proclore", 8) == 0)
    {

        if (strlen(s) == 8)
        {

            proclore(getpid());
        }
        else
        {
            // printf("ye");
            char *s1 = strtok(s + 9, " ");
            int x1 = atoi(s1);
            // printf("%d", x1);
            proclore(x1);
        }
    }
    else if (strncmp(s, "seek", 4) == 0)
    {
        char *s5 = s;
        int d_flag = 0, f_flag = 0, e_flag = 0;
        char *src = NULL;
        char target_directory[5000] = ".";
        char *s1 = strtok(s + 5, " ");
        // int f = 0;
        while (s1 != NULL)
        {
            // f++;
            // printf("%s\n",s1);
            if (strcmp(s1, "-d") == 0)
            {
                d_flag = 1;
            }
            else if (strcmp(s1, "-f") == 0)
            {
                f_flag = 1;
            }
            else if (strcmp(s1, "-e") == 0)
            {
                e_flag = 1;
            }
            else if (!src)
            {
                src = s1;
            }
            else
            {
                // strcat(target_directory,)
                // if (strcmp(s1, "."))
                // {
                //     target_directory = ".";
                // }
                // else
                if (s1[0] == '-')
                    // target_directory = prevdir;
                    strcpy(target_directory, prevdir);
                else if (s1[0] == '~')
                {
                    // target_directory = homedir;
                    strcpy(target_directory, homedir);
                    strcat(target_directory, s1 + 1);
                    // printf("%s\n",target_directory);
                }
                else
                {
                    char s9[4096];
                    getcwd(s9, 4096);
                    strcat(s9, s1 + 1);
                    // printf("%s\n",s9);
                    strcpy(target_directory, s9);
                    // target_directory = s9;
                }
                // printf("%s\n",target_directory);
            }
            s1 = strtok(NULL, " ");
        }
        // if(d_flag==f_flag){
        //     continue;
        // }
        if (strlen(s5) != 4)
        {

            seek(src, target_directory, d_flag, f_flag, e_flag, target_directory);
            // printf("%d\n",filecnt);
            if (filecnt == 0)
            {
                if ((d_flag == 0 && f_flag == 0))
                {
                    printf("No match found\n");
                }
                else if (d_flag == f_flag)
                {

                    printf("Invalid flags!\n");
                }
                else
                {
                    printf("No match found\n");
                }
            }

            else
            {
                for (int i = 0; i < filecnt; i++)
                {

                    struct stat statbuf;
                    stat(filedirs[i], &statbuf);
                    char *s12 = filedirs[i];
                    char s13[5000];
                    strcpy(s13, filedirs[i]);
                    char s11[5000] = ".";
                    // printf("%s\n", target_directory);
                    // char s13
                    strcat(s11, s12 + strlen(target_directory));
                    strcpy(filedirs[i], s11);
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        printf("\033[34m%s/\033[0m\n", filedirs[i]);
                        // printf("%s\n", filedirs[i]);

                        if (filecnt == 1 && e_flag == 1 && access(s12, X_OK) == 0)
                        {
                            // printf("%s s13\n", s13);
                            char s45[5000];
                            getcwd(s45, 5000);
                            chdir(s13);
                            strcpy(prevdir, s45);
                            // hop(s13);
                        }
                        else
                        {
                            printf("Missing permissions for task\n");
                        }
                    }
                    else
                    {
                        printf("\033[32m%s\033[0m\n", filedirs[i]);
                        if (filecnt == 1 && e_flag == 1 && access(s12, R_OK) == 0)
                        {

                            FILE *file = fopen(s13, "r");
                            char ch;
                            while ((ch = fgetc(file)) != EOF)
                            {
                                printf("%c", ch);
                            }

                            fclose(file);
                        }
                        else
                        {
                            printf("Missing permissions for task\n");
                        }
                    }
                }

                filecnt = 0;
            }

            // else if (filecnt == 1 && e_flag == 1 && d_flag == 1)
            // {
            //     char s20[4096];
            //     getcwd(s20, 4096);
            //     // printf("%sasjd\n",filedirs[0]);
            //     // hop(filedirs[0]);
            //     chdir(filedirs[0]);
            // }
            // else if (filecnt == 1 && e_flag == 1 && f_flag == 1)
            // {
            //     FILE *file = fopen(filedirs[0], "r");
            //     char ch;
            //     while ((ch = fgetc(file)) != EOF)
            //     {
            //         printf("%c", ch);
            //     }

            //     fclose(file);
            // }

            // else if (filecnt == 1 && e_flag == 1)
            // {
            //     // struct stat statbuf;
            //     // printf("%c asd\n", filedirs[0][strlen(filedirs[0]) - 1]);
            //     // printf("%s asd\n", filedirs[0]);

            //     // if (filedirs[0][strlen(filedirs[0]) - 1] == '/')
            //     if (asd == 1)
            //     {
            //         strcat(target_directory, filedirs[0] + 1);
            //         // target_directory[strlen(target_directory) - 1] = '\0';
            //         // printf("%s asd\n", target_directory);

            //         // chdir(target_directory);
            //         // hop(target_directory);
            //     }
            //     else
            //     {
            //         FILE *file = fopen(filedirs[0], "r");
            //         char ch;
            //         while ((ch = fgetc(file)) != EOF)
            //         {
            //             printf("%c", ch);
            //         }

            //         fclose(file);
            //     }
            // }
        }
        else
        {
            printf("ERROR: Command not recognized\n");
        }

        filecnt = 0;
        // return;
    }
    else
    {
        printf("ERROR: Command not recognized\n");
    }
    // int j=
}