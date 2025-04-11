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
#include <termios.h>
#include <sys/time.h>
#include "hop.h"
#include "reveal.h"
#include "input.h"
#include "log.h"
#include "iman.h"
#include "proclore.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
int x;
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
typedef struct Process
{
    pid_t pid;
    char command[256];
    char state[100];
} Process;
extern Process processes[1000];
extern int process_count;
extern int fore_id;
typedef struct
{
    char alias[5000];
    char command[5000];
} Alias;

Alias aliases[1000];
int alias_count = 0;

void load_myshrc()
{
    char myshrc_path[5000];
    sprintf(myshrc_path, "%s/project/.myshrc", homedir);
    FILE *file = fopen(myshrc_path, "r");
    if (!file)
    {
        perror("Could not open .myshrc file");
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\n")] = 0;

        if (strstr(line, "alias") != NULL)
        {
            char *alias = strtok(line, "=");
            alias += 6;
            char *command = strtok(NULL, "=");
            if (alias && command)
            {
                strcpy(aliases[alias_count].alias, alias);
                strcpy(aliases[alias_count].command, command);
                alias_count++;
            }
        }
    }
    fclose(file);
}

void replace_aliases(char *input)
{
    char temp[5000];
    char *pos = input;
    char *found;

    temp[0] = '\0';

    for (int i = 0; i < alias_count; i++)
    {
        pos = input;

        while ((found = strstr(pos, aliases[i].alias)) != NULL)
        {
            strncat(temp, pos, found - pos);
            strcat(temp, aliases[i].command);
            pos = found + strlen(aliases[i].alias);
        }

        strcat(temp, pos);

        strcpy(input, temp);

        temp[0] = '\0';
    }
}

void add_process(pid_t pid, char *command, char *state)
{
    processes[process_count].pid = pid;
    strcpy(processes[process_count].command, command);
    strcpy(processes[process_count].state, state);
    process_count++;
}

void update_process(int pid)
{
    for (int i = 0; i < process_count; i++)
    {
        if (processes[i].pid == pid)
        {
            char path[256], buffer[1024];
            int fd;
            sleep(0.5);
            sprintf(path, "/proc/%d/stat", pid);
            if ((fd = open(path, O_RDONLY)) < 0)
            {
                strcpy(processes[i].state, "Puss");
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
            sscanf(buffer, "%d %s %c", &pid, comm, &state);
            if (state == 'S')
            {
                strcpy(processes[i].state, "Running");
            }
            else if (state == 'R')
            {
                strcpy(processes[i].state, "Running");
            }
            else if (state == 'T')
            {
                strcpy(processes[i].state, "Stopped");
            }
            else if (state == 'Z')
            {
                strcpy(processes[i].state, "Zombie");
            }
            else
            {
                strcpy(processes[i].state, "Puss");
            }
        }
    }
}
int compare(const void *a, const void *b)
{
    Process *processA = (Process *)a;
    Process *processB = (Process *)b;
    return strcmp(processA->command, processB->command);
}

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
        if (strcmp(tok, "<") != 0)
            argv[c++] = tok;

        // printf("%s\n",argv[c-1]);

        tok = strtok(NULL, " ");
    }
    argv[c] = NULL;

    if (c == 0)
        return;

    if (execvp(argv[0], argv) == -1)
    {
        printf("ERROR: '%s' is not a valid command\n", argv[0]);
        exit(EXIT_FAILURE);
    }
}

void handle_pipes(char *comm)
{
    load_myshrc();
    replace_aliases(comm);
    char *commands[100];
    int num_commands = 0;
    char *saveptr;
    if (comm[strlen(comm) - 1] == '|' || comm[0] == '|')
    {
        printf("Error: Missing command after pipe\n");
        return;
    }

    int nn = strlen(comm);
    int j = -1;
    int cc = 0;
    int flg = 0;
    char s34[5000];
    for (int i = 0; i < nn - 1; i++)
    {
        if (comm[i] == '|' && comm[i + 1] == '|')
        {
            j = i;
            break;
        }
        if (comm[i] != ' ' && comm[i] != '`')
        {
            s34[cc++] = comm[i];
        }
        if ((comm[i] == '|' && comm[i + 1] == ' '))
        {
            // flg = 1;
            for (int k = i + 1; k < nn - 1; k++)
            {

                if (comm[k] == '|')
                {
                    flg = 1;
                    break;
                }
                else if (comm[k] != '|' && comm[k] != ' ')
                {
                    break;
                }
            }
        }
    }

    if (comm[nn - 1] != ' ' || comm[nn - 1] != '`')
    {
        s34[cc++] = comm[nn - 1];
    }
    if (j != -1)
        comm[j - 1] = '\0';
    nn = strlen(s34);
    for (int i = 0; i < nn - 1; i++)
    {
        if (s34[i] == '&' && s34[i + 1] == '|')
        {
            printf("Invalid Command\n");
            return;
        }
        if (s34[i] == '|' && s34[i + 1] == '&')
        {
            printf("Invalid Command\n");
            return;
        }
    }
    if (flg == 1)
    {
        printf("Invalid Command\n");
        return;
    }

    while ((commands[num_commands] = strtok_r(comm, "|", &saveptr)) != NULL)
    {

        comm = NULL;
        num_commands++;
    }

    if (commands[num_commands - 1] != NULL && strlen(commands[num_commands - 1]) == 0)
    {
        fprintf(stderr, "Error: Trailing pipe without command\n");
        return;
    }

    int pipefd[2], fd = 0;
    for (int i = 0; i < num_commands; i++)
    {
        char *saveptr2;
        char *sub_command;
        int status;
        pipe(pipefd);
        pid_t pid = fork();
        if (pid == 0)
        {
            setpgid(0, 0);
            dup2(fd, STDIN_FILENO);
            if (i < num_commands - 1)
            {
                dup2(pipefd[1], STDOUT_FILENO);
            }
            close(pipefd[0]);
            close(pipefd[1]);

            if (strstr(commands[i], ">") != NULL || strstr(commands[i], ">>") != NULL || strstr(commands[i], "<") != NULL)
            {
                int a_ind = -1;
                strcat(commands[i], "  ");
                int len = strlen(commands[i]);
                for (int j = 0; j < len; j++)
                {
                    if (commands[i][j] == '&')
                    {
                        a_ind = i;
                        break;
                    }
                }

                if (a_ind != -1)
                {
                    for (int j = a_ind; j < len - 1; j++)
                    {
                        commands[i][j] = commands[i][j + 1];
                    }
                    commands[i][len - 1] = '&';
                }

                a_ind = -1;
                for (int j = 0; j < len; j++)
                {
                    if (commands[i][j] == '`')
                    {
                        a_ind = j;
                        break;
                    }
                }

                if (a_ind != -1)
                {
                    for (int j = a_ind; j < len - 1; j++)
                    {
                        commands[i][j] = commands[i][j + 1];
                    }

                    commands[i][len - 1] = '`';
                }
            }

            while ((sub_command = strtok_r(commands[i], "`", &saveptr2)) != NULL)
            {
                commands[i] = NULL;

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

                int input_fd = -1, output_fd = -1;
                int append_mode = 0; // Flag for '>>'
                char *input_file = NULL;
                char *output_file = NULL;
                char sub_command1[5000];
                char sub_command2[5000];
                char sub_command3[5000];

                strcpy(sub_command1, sub_command);
                strcpy(sub_command2, sub_command);
                strcpy(sub_command3, sub_command);

                int save1;
                char *input_redir = strstr(sub_command2, "<");
                char input_redir1[5000];
                char output_redir1[5000];
                char append_redir1[5000];

                if (input_redir != NULL)
                {
                    strcpy(input_redir1, input_redir);
                    input_file = strtok(input_redir + 1, " \n");

                    input_fd = open(input_file, O_RDONLY);
                    if (input_fd == -1)
                    {
                        perror("Error opening input file");
                    }
                    *input_redir = '\0';

                    save1 = dup(0);
                    dup2(input_fd, 0);
                }

                char *append_redir = strstr(sub_command1, ">>");
                char *output_redir = strstr(sub_command1, ">");

                if (append_redir != NULL)
                {
                    strcpy(output_redir1, append_redir);
                    output_file = strtok(append_redir + 2, " \n");
                    append_mode = 1;
                }
                else
                {

                    if (output_redir != NULL)
                    {
                        strcpy(output_redir1, output_redir);

                        output_file = strtok(output_redir + 1, " \n");
                    }
                }
                // printf("%s sub2\n", sub_command);
                int save;
                if (output_file)
                {
                    if (append_mode)
                    {
                        output_fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    }
                    else
                    {
                        output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    }
                    if (output_fd == -1)
                    {
                        perror("Error opening output file");
                    }
                    int len1;
                    if (output_redir)
                    {

                        *output_redir = '\0';
                    }
                    else if (append_redir)
                    {
                        *append_redir = '\0';
                    }

                    save = dup(1);
                    dup2(output_fd, 1);
                }

                char cleaned_command[5000] = "";
                char *kot;

                kot = strtok(sub_command3, " ");

                if (kot != NULL)
                {
                    strcat(cleaned_command, kot);
                    strcat(cleaned_command, " ");
                    kot = strtok(NULL, " ");
                }

                while (kot != NULL)
                {
                    if (strcmp(kot, "<") == 0 || strcmp(kot, ">") == 0 || strcmp(kot, ">>") == 0)
                    {
                        kot = strtok(NULL, " ");
                    }
                    else
                    {
                        strcat(cleaned_command, kot);
                        strcat(cleaned_command, " ");
                    }

                    kot = strtok(NULL, " ");
                }

                int len = strlen(cleaned_command);
                if (len > 0 && cleaned_command[len - 1] == ' ' && cleaned_command[len - 1] == '\n')
                {
                    cleaned_command[len - 1] = '\0';
                }

                j = strlen(cleaned_command) - 1;
                if (j > 0)
                    while (cleaned_command[j] == ' ')
                    {
                        j--;
                    }

                cleaned_command[j + 1] = '\0';

                // printf("%s subcommand2\n", cleaned_command);

                if (strchr(cleaned_command, '&') != NULL)
                {
                    // printf("321\n");
                    cleaned_command[strlen(cleaned_command) - 1] = '\0';
                    pid_t pid = fork();
                    if (pid < 0)
                    {
                        printf("ERROR: Unable to fork\n");
                    }
                    else if (pid == 0)
                    {
                        setpgid(0, 0);

                        execute_command(cleaned_command);
                    }
                    else
                    {
                        add_process(pid, cleaned_command, "Running");
                        fprintf(stderr, "%d\n", pid);
                    }
                }
                else
                {

                    if (strncmp(cleaned_command, "reveal", 6) == 0 || strncmp(cleaned_command, "hop", 3) == 0 || strncmp(cleaned_command, "log", 3) == 0 || strncmp(cleaned_command, "proclore", 8) == 0 || strncmp(cleaned_command, "seek", 4) == 0 || strncmp(cleaned_command, "activities", 10) == 0 || strncmp(cleaned_command, "fg", 2) == 0 || strncmp(cleaned_command, "ping", 4) == 0 || strncmp(cleaned_command, "bg", 2) == 0 || strncmp(cleaned_command, "neonate -n", 10) == 0 || strncmp(cleaned_command, "iMan", 4) == 0 || strncmp(cleaned_command, "hop_seek", 8) == 0 || strncmp(cleaned_command, "mk_hop", 6) == 0)
                    {
                        main_command(cleaned_command);
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
                            setpgid(0, 0);

                            execute_command(cleaned_command);

                            exit(EXIT_SUCCESS);
                        }
                        else
                        {

                            struct timeval st, end;
                            gettimeofday(&st, NULL);
                            fore_id = pid;

                            add_process(pid, cleaned_command, "Running");
                            waitpid(fore_id, NULL, WUNTRACED);

                            update_process(pid);
                            fore_id = -1;
                            gettimeofday(&end, NULL);
                            int secs = end.tv_sec - st.tv_sec;
                            if (secs > 2)
                            {
                                lasttime = secs;
                                lastcomm = strtok(cleaned_command, " ");
                            }
                        }
                    }
                }
                if (output_file)
                {
                    dup2(save, 1);
                    close(save);
                    close(output_fd);
                }
                if (input_file)
                {
                    dup2(save1, 0);
                    close(save1);
                    close(input_fd);
                }
            }

            fflush(stdout);
            exit(EXIT_SUCCESS);
        }
        else
        {

            wait(NULL);
            close(pipefd[1]);
            fd = pipefd[0];
        }
    }
}

void process_input(char *input, int fl)
{
    load_myshrc();
    char *command;
    char *rest = input;
    char *saveptr1;
    char s2[4096];
    getcwd(s2, 4096);
    if (strstr(rest, "log") == NULL && fl == 0 && strcmp(rest, "\n") != 0)
    {
        save_log(rest, s2);
    }
    replace_aliases(rest);

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
        // printf("%s comm\n",command);

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

        char result1[5000];
        char *result2 = (char *)malloc(5000);

        strcpy(result1, result);

        if (strstr(result1, "|") != NULL)
        {
            handle_pipes(result1);
            continue;
        }

        if (strstr(result, ">") != NULL || strstr(result, ">>") != NULL || strstr(result, "<") != NULL)
        {
            int a_ind = -1;
            strcat(result, "  ");
            int len = strlen(result);
            for (int i = 0; i < len; i++)
            {
                if (result[i] == '&')
                {
                    a_ind = i;
                    break;
                }
            }

            if (a_ind != -1)
            {
                for (int i = a_ind; i < len - 1; i++)
                {
                    result[i] = result[i + 1];
                }
                result[len - 1] = '&';
            }

            a_ind = -1;
            for (int i = 0; i < len; i++)
            {
                if (result[i] == '`')
                {
                    a_ind = i;
                    break;
                }
            }

            if (a_ind != -1)
            {
                for (int i = a_ind; i < len - 1; i++)
                {
                    result[i] = result[i + 1];
                }

                result[len - 1] = '`';
            }
        }
        strcpy(result2, result);

        while ((sub_command = strtok_r(result2, "`", &saveptr2)) != NULL)
        {
            result2 = NULL;

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

            int input_fd = -1, output_fd = -1;
            int append_mode = 0;
            char *input_file = NULL;
            char *output_file = NULL;
            char sub_command1[5000];
            char sub_command2[5000];
            char sub_command3[5000];

            strcpy(sub_command1, sub_command);
            strcpy(sub_command3, sub_command);

            strcpy(sub_command2, sub_command);

            int save1;
            char *input_redir = strstr(sub_command2, "<");
            char input_redir1[5000];
            char output_redir1[5000];
            char append_redir1[5000];

            if (input_redir != NULL)
            {
                strcpy(input_redir1, input_redir);
                input_file = strtok(input_redir + 1, " \n");

                input_fd = open(input_file, O_RDONLY);

                *input_redir = '\0';

                save1 = dup(0);
                dup2(input_fd, 0);
            }

            char *append_redir = strstr(sub_command1, ">>");
            char *output_redir = strstr(sub_command1, ">");

            if (append_redir != NULL)
            {
                strcpy(output_redir1, append_redir);
                output_file = strtok(append_redir + 2, " \n");
                append_mode = 1;
            }
            else
            {

                if (output_redir != NULL)
                {
                    strcpy(output_redir1, output_redir);

                    output_file = strtok(output_redir + 1, " \n");
                }
            }

            int save;
            if (output_file)
            {
                if (append_mode)
                {
                    output_fd = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
                }
                else
                {
                    output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                }
                if (output_fd == -1)
                {
                    perror("Error opening output file");
                }
                int len1;
                if (output_redir)
                {

                    *output_redir = '\0';
                }
                else if (append_redir)
                {
                    *append_redir = '\0';
                }

                save = dup(1);
                dup2(output_fd, 1);
            }

            char cleaned_command[5000] = "";
            char *kot;

            kot = strtok(sub_command3, " ");

            if (kot != NULL)
            {
                strcat(cleaned_command, kot);
                strcat(cleaned_command, " ");
                kot = strtok(NULL, " ");
            }

            while (kot != NULL)
            {
                if (strcmp(kot, "<") == 0 || strcmp(kot, ">") == 0 || strcmp(kot, ">>") == 0)
                {
                    kot = strtok(NULL, " ");
                }
                else
                {
                    strcat(cleaned_command, kot);
                    strcat(cleaned_command, " ");
                }

                kot = strtok(NULL, " ");
            }

            int len = strlen(cleaned_command);
            if (len > 0 && cleaned_command[len - 1] == ' ' && cleaned_command[len - 1] == '\n')
            {
                cleaned_command[len - 1] = '\0';
            }

            j = strlen(cleaned_command) - 1;
            if (j > 0)
                while (cleaned_command[j] == ' ')
                {
                    j--;
                }

            cleaned_command[j + 1] = '\0';

            if (strchr(cleaned_command, '&') != NULL)
            {
                // printf("321\n");
                cleaned_command[strlen(cleaned_command) - 1] = '\0';
                pid_t pid = fork();
                if (pid < 0)
                {
                    printf("ERROR: Unable to fork\n");
                }
                else if (pid == 0)
                {
                    setpgid(0, 0);

                    execute_command(cleaned_command);
                }
                else
                {
                    add_process(pid, cleaned_command, "Running");
                    fprintf(stderr, "%d\n", pid);
                }
            }
            else
            {

                if (strncmp(cleaned_command, "reveal", 6) == 0 || strncmp(cleaned_command, "hop", 3) == 0 || strncmp(cleaned_command, "log", 3) == 0 || strncmp(cleaned_command, "proclore", 8) == 0 || strncmp(cleaned_command, "seek", 4) == 0 || strncmp(cleaned_command, "activities", 10) == 0 || strncmp(cleaned_command, "fg", 2) == 0 || strncmp(cleaned_command, "ping", 4) == 0 || strncmp(cleaned_command, "bg", 2) == 0 || strncmp(cleaned_command, "neonate -n", 10) == 0 || strncmp(cleaned_command, "iMan", 4) == 0 || strncmp(cleaned_command, "hop_seek", 8) == 0 || strncmp(cleaned_command, "mk_hop", 6) == 0)
                {
                    main_command(cleaned_command);
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
                        setpgid(0, 0);

                        execute_command(cleaned_command);
                    }
                    else
                    {

                        struct timeval st, end;
                        gettimeofday(&st, NULL);
                        fore_id = pid;

                        add_process(pid, cleaned_command, "Running");
                        waitpid(fore_id, NULL, WUNTRACED);

                        update_process(pid);
                        fore_id = -1;
                        gettimeofday(&end, NULL);
                        int secs = end.tv_sec - st.tv_sec;
                        if (secs > 2)
                        {
                            lasttime = secs;
                            lastcomm = strtok(cleaned_command, " ");
                        }
                    }
                }
            }
            if (output_file)
            {
                dup2(save, 1);
                close(save);
                close(output_fd);
                output_file = NULL;
            }
            if (input_file)
            {
                dup2(save1, 0);
                close(save1);
                close(input_fd);
                input_file = NULL;
            }
        }

        sub_command = NULL;
    }
}

void main_command(char *s)
{

    char s2[4096];
    getcwd(s2, 4096);

    if (strncmp(s, "hop", 3) == 0 && strncmp(s, "hop_seek", 8) != 0)
    {

        char prev1[4096];
        getcwd(prev1, 4096);
        char *s1 = strtok(s + 4, " ");
        if (strncmp(s, "hop", 3) == 0 && s1 == NULL)
        {

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
        strcpy(prevdir, prev1);
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
        char *s3 = s;
        int exe = s[strlen(s) - 1] - '0';
        int exe1 = 0;
        if (s[strlen(s) - 2] != ' ')
        {
            exe1 = s[strlen(s) - 2] - '0';
            exe1 *= 10;
        }
        exe1 += exe;

        // printf("%d\n",exe1);
        char *s1 = strtok(s + 4, " ");
        if (strcmp(s3, "log") == 0)
        {

            print_log();
        }
        else
        {
            char oth[5000];
            int fl = 0;
            while (s1 != NULL)
            {

                if (strcmp(s1, "purge") == 0)
                {
                    log_purge(s2);
                }
                else if (strcmp(s1, "execute") == 0)
                {
                    fl = 1;
                }
                else
                {
                    strcat(oth, s1);
                    strcat(oth, " ");
                }
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
        while (s1 != NULL)
        {

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

                if (s1[0] == '-')
                    strcpy(target_directory, prevdir);
                else if (s1[0] == '~')
                {
                    strcpy(target_directory, homedir);
                    strcat(target_directory, s1 + 1);
                    // printf("%s\n",target_directory);
                }
                else if (s1[0] == '/')
                {
                    strcpy(target_directory, s1);
                }
                else
                {
                    char s9[4096];
                    getcwd(s9, 4096);
                    strcat(s9, s1 + 1);
                    strcpy(target_directory, s9);
                }
                // printf("%s\n",target_directory);
            }
            s1 = strtok(NULL, " ");
        }

        if (strlen(s5) != 4)
        {

            seek(src, target_directory, d_flag, f_flag, e_flag, target_directory);
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

                    strcat(s11, s12 + strlen(target_directory));
                    strcpy(filedirs[i], s11);
                    if (S_ISDIR(statbuf.st_mode))
                    {
                        printf("\033[34m%s/\033[0m\n", filedirs[i]);

                        if (filecnt == 1 && e_flag == 1 && access(s13, X_OK) == 0)
                        {
                            char s45[5000];
                            getcwd(s45, 5000);
                            chdir(s13);
                            strcpy(prevdir, s45);
                        }
                        else if (access(s13, X_OK) != 0)
                        {
                            printf("Missing permissions for task\n");
                        }
                    }
                    else
                    {
                        printf("\033[32m%s\033[0m\n", filedirs[i]);
                        if (filecnt == 1 && e_flag == 1 && access(s13, R_OK) == 0)
                        {

                            FILE *file = fopen(s13, "r");
                            char ch;
                            while ((ch = fgetc(file)) != EOF)
                            {
                                printf("%c", ch);
                            }

                            fclose(file);
                        }
                        else if (access(s13, R_OK) != 0)
                        {
                            printf("Missing permissions for task\n");
                        }
                    }
                }

                filecnt = 0;
            }
        }

        else
        {
            printf("ERROR: Command not recognized\n");
        }

        filecnt = 0;
        // return;
    }
    else if (strncmp(s, "activities", 10) == 0)
    {
        qsort(processes, process_count, sizeof(Process), compare);
        // printf("%d\n", process_count);
        for (int i = 0; i < process_count; i++)
        {
            if (strcmp(processes[i].state, "Puss") != 0)
                printf("[%d] : %s - %s\n", processes[i].pid, processes[i].command, processes[i].state);
        }
    }
    else if (strncmp(s, "fg", 2) == 0)
    {
        char *str = strtok(s + 3, " ");
        int pid = atoi(str);
        if (kill(pid, 0) == -1)
        {
            perror("No such process found");
            return;
        }

        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        tcsetpgrp(STDIN_FILENO, pid);

        if (kill(pid, SIGCONT) == -1)
        {
            perror("Failed to continue process");
            tcsetpgrp(STDIN_FILENO, getpid());
            signal(SIGTTOU, SIG_DFL);
            signal(SIGTTIN, SIG_DFL);
            return;
        }
        fore_id = pid;
        int status;
        waitpid(pid, &status, WUNTRACED);

        tcsetpgrp(STDIN_FILENO, getpid());

        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);

        if (WIFSTOPPED(status))
        {
            printf("Process [%d] was stopped by signal %d\n", pid, WSTOPSIG(status));
        }
        else if (WIFEXITED(status))
        {
            printf("Process [%d] exited with status %d\n", pid, WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status))
        {
            printf("Process [%d] was killed by signal %d\n", pid, WTERMSIG(status));
        }
        else if (WIFCONTINUED(status))
        {
            printf("Process [%d] continued\n", pid);
        }
        update_process(pid);
        fore_id = -1;
    }
    else if (strncmp(s, "bg", 2) == 0)
    {
        char *str = strtok(s + 3, " ");
        int pid = atoi(str);
        int flag = 0;
        for (int i = 0; i < process_count; i++)
        {
            if (processes[i].pid == pid && strcmp(processes[i].state, "Stopped") == 0)
            {
                printf("Resuming [%d] %s in background\n", pid, processes[i].command);
                kill(pid, SIGCONT);
                update_process(pid);
                flag = 1;
                break;
            }
        }

        if (flag == 0)
        {
            printf("No such process found\n");
        }
    }
    else if (strncmp(s, "ping", 4) == 0)
    {
        char *str = strtok(s + 5, " ");
        char *s_str = strtok(NULL, " ");

        if (str != NULL && s_str != NULL)
        {
            pid_t pid = atoi(str);
            int signal_num = atoi(s_str);

            signal_num = signal_num % 32;

            if (kill(pid, 0) == 0)
            {
                if (kill(pid, signal_num) == 0)
                {
                    printf("Sent signal %d to process with pid %d\n", signal_num, pid);
                    update_process(pid);
                }
                else
                {
                    perror("Error sending signal");
                }
            }
            else
            {
                printf("No such process found\n");
            }
        }
        else
        {
            printf("ERROR: Please provide valid PID and signal number\n");
        }
    }
    else if (strncmp(s, "neonate -n", 10) == 0)
    {
        char s2[100];
        strcpy(s2, s + 11);
        int n1 = atoi(s2);

        while (1)
        {
            DIR *proc_dir;
            struct dirent *entry;
            int recent_pid = -1;

            proc_dir = opendir("/proc");
            if (!proc_dir)
            {
                perror("Error opening /proc directory");
                return -1;
            }

            while ((entry = readdir(proc_dir)) != NULL)
            {
                int pid = atoi(entry->d_name);
                if (pid > recent_pid)
                {
                    recent_pid = pid;
                }
            }

            closedir(proc_dir);

            if (recent_pid != -1)
            {
                printf("%d\n", recent_pid);
            }

            sleep(n1);

            struct termios oldt, newt;
            int ch;
            int ret = 0;
            int oldf;

            tcgetattr(STDIN_FILENO, &oldt);
            newt = oldt;
            newt.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
            fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

            ch = getchar();

            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            fcntl(STDIN_FILENO, F_SETFL, oldf);

            if (ch != EOF)
            {
                ungetc(ch, stdin);
                ret = 1;
            }

            if (ret && getchar() == 'x')
            {
                break;
            }
        }
    }
    else if (strncmp(s, "iMan", 4) == 0)
    {
        char *s1 = strtok(s + 5, " ");
        iman(s1);
    }
    else if (strncmp(s, "hop_seek", 8) == 0)
    {
        char myshrc_path[5000];
        sprintf(myshrc_path, "%s/project/.myshrc", homedir);
        FILE *file = fopen(myshrc_path, "r");
        char line[256];
        int flg = 0;
        while (fgets(line, sizeof(line), file))
        {
            if (strcmp(line, "hop_seek()\n") == 0)
            {
                flg = 1;
                break;
            }
        }
        fclose(file);
        if (flg == 1)
        {

            char *s1 = strtok(s + 9, " ");
            char s2[5000] = "hop ";
            strcat(s2, s1);
            strcat(s2, "; ");
            strcat(s2, "seek ");
            strcat(s2, s1);
            // printf("%s \n",s2);
            process_input(s2, 1);
        }
        else
        {
            printf("ERROR: Command not recognized\n");
        }
    }
    else if (strncmp(s, "mk_hop", 6) == 0)
    {
        char myshrc_path[5000];
        sprintf(myshrc_path, "%s/project/.myshrc", homedir);
        FILE *file = fopen(myshrc_path, "r");
        char line[256];
        int flg = 0;
        while (fgets(line, sizeof(line), file))
        {
            if (strcmp(line, "mk_hop()\n") == 0)
            {
                flg = 1;
                break;
            }
        }

        fclose(file);
        if (flg == 1)
        {

            char *s1 = strtok(s + 7, " ");
            char s2[5000] = "mkdir ";
            strcat(s2, s1);
            strcat(s2, "; ");
            strcat(s2, "hop ");
            strcat(s2, s1);
            // printf("%s \n",s2);
            process_input(s2, 1);
        }
        else
        {
            printf("ERROR: Command not recognized\n");
        }
    }
    else
    {
        printf("ERROR: Command not recognized\n");
    }
}
