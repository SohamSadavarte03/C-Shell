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
#include <sys/socket.h>
#include <netdb.h>
#include <sys/time.h>
#include "hop.h"
#include "reveal.h"
#include "input.h"

#include "iman.h"

#include "log.h"
#include "proclore.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

void iman(char *comm)
{
    char *command_name = comm;

    const char *host = "man.he.net";
    const char *path_format = "/?topic=%s&section=all";
    char path[256];
    snprintf(path, sizeof(path), path_format, command_name);

    char request[512];
    snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, "80", &hints, &res) != 0)
    {
        perror("getaddrinfo");
        return 1;
    }

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0)
    {
        perror("socket");
        freeaddrinfo(res);
        return 1;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
    {
        perror("connect");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    if (send(sockfd, request, strlen(request), 0) < 0)
    {
        perror("send");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }

    char buffer[5000];
    int received;
    int header_skipped = 0;
    received = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

    buffer[received] = '\0';
    char *s1 = strstr(buffer, "<title>");
    // printf("%s s1\n",s1);
    strcpy(buffer, s1);

    if (!header_skipped)
    {
        char *body = strstr(buffer, "\r\n\r\n");
        if (body)
        {
            body += 4;
            int in_tag = 0;
            for (int i = 0; body[i] != '\0'; i++)
            {

                if (body[i] == '<')
                {
                    in_tag = 1;
                }
                else if (body[i] == '>')
                {
                    in_tag = 0;
                }
                else if (!in_tag)
                {
                    putchar(body[i]);
                }
            }
        }
        else
        {
            int in_tag = 0;
            for (int i = 0; buffer[i] != '\0'; i++)
            {

                if (buffer[i] == '<')
                {
                    in_tag = 1;
                }
                else if (buffer[i] == '>')
                {
                    in_tag = 0;
                }
                else if (!in_tag)
                {
                    putchar(buffer[i]);
                }
            }
        }
        header_skipped = 1;
    }
    else
    {
        int in_tag = 0;
        for (int i = 0; buffer[i] != '\0'; i++)
        {

            if (buffer[i] == '<')
            {
                in_tag = 1;
            }
            else if (buffer[i] == '>')
            {
                in_tag = 0;
            }
            else if (!in_tag)
            {

                putchar(buffer[i]);
            }
        }
    }

    if (received < 0)
    {
        perror("recv");
    }

    close(sockfd);
    freeaddrinfo(res);
}