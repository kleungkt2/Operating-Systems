
#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"
char *
fmtname(char *path)
{
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    int i = 0;
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
    {
        i++;
    }
    p++;
    i--;
    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    buf[i] = '\0';
    return buf;
}
char *my_strcat(char *destination, const char *source)
{
    // make `ptr` point to the end of the destination string
    char *ptr = destination + strlen(destination);

    // appends characters of the source to the destination string
    while (*source != '\0')
    {
        *ptr++ = *source++;
    }

    // null terminate destination string
    *ptr = '\0';

    // the destination is returned by standard `strcat()`
    return destination;
}
void printtree(char *path, int level)
{
    if (level == 0)
    {
        printf("%s\n", path);
        return;
    }
    char *first_line = (char *)malloc(50);
    char *second_line = (char *)malloc(50);
    first_line[0] = '\0';
    second_line[0] = '\0';
    my_strcat(first_line, "|");
    for (int i = 1; i < level; i++)
    {
        my_strcat(first_line, "   |");
    }
    if (level == 1)
    {
        my_strcat(second_line, "+-- ");
    }
    else if (level > 1)
    {

        my_strcat(second_line, "|   ");
        for (int i = 1; i < level; i++)
        {
            my_strcat(second_line, "+-- ");
        }
    }
    if (level > 0)
    {
        printf("%s\n", first_line);
        printf("%s%s\n", second_line, fmtname(path));
        free(first_line);
        free(second_line);
    }
}
int tree(char *path, int level, int *dir_num, int *file_num)
{

    int fd;
    struct stat st;
    char buf[128], *p;
    struct dirent de;
    if ((fd = open(path, 0)) < 0)
    {
        printf("%s [error opening dir]\n", path);
        printf("\n");
        printf("0 directories, 0 files\n");
        exit(1);
        return -1;
    }
    if (fstat(fd, &st) < 0)
    {
        printf("%s [error opening dir]\n", path);
        printf("\n");
        printf("0 directories, 0 files\n");
        return -1;
    }

    switch (st.type)
    {
    case T_FILE:

        if (level == 0)
        {
            printf("%s [error opening dir]\n", path);
            printf("\n");
            printf("0 directories, 0 files\n");
            return -1;
        }
        else
        {
            *file_num = *file_num + 1;
            printtree(path, level);
        }

        break;
    case T_DIR:
        if (level > 0)
        {
            *dir_num = *dir_num + 1;
        }
        printtree(path, level);
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            if (de.inum == 0)
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0)
            {
                printf("ERROR %s\n", buf);
                continue;
            }

            tree(buf, level + 1, dir_num, file_num);
        }
        break;
    }
    close(fd);
    return 1;
}

int main(int argc, char *argv[])
{
    int dir_num[2];
    int file_num[2];
    pipe(dir_num);
    pipe(file_num);
    int pid;
    pid = fork();
    int success = 1;
    if (pid < 0)
    {
        printf("Error: failed to create fork\n");
    }
    else if (pid == 0)
    {
        int dir_child = 0;
        int file_child = 0;
        int success = tree(argv[1], 0, &dir_child, &file_child);
        if (success != -1)
        {
            write(dir_num[1], &dir_child, 1);
            write(file_num[1], &file_child, 1);
            close(dir_num[1]);
            close(file_num[1]);
        }

    }
    else if (pid > 0 && success != -1)
    {
        int dir_parent = 0, file_parent = 0;
        read(dir_num[0], &dir_parent, 1);
        read(file_num[0], &file_parent, 1);
        printf("\n%d directories, %d files\n", dir_parent, file_parent);
        close(dir_num[0]);
        close(file_num[0]);
    }
    exit(0);
}
