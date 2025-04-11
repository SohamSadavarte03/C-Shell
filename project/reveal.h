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

void print_permissions(mode_t mode);
void reveal(int show_hidden, int show_long, char *path);