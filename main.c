#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <locale.h>
#include <getopt.h>

#define PATH_MAX 4096

void dirwalk(const char *path, const char *options) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;

    if ((dir = opendir(path)) == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char fullpath[PATH_MAX];
        sprintf(fullpath, "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &statbuf) == -1) {
            perror("lstat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            dirwalk(fullpath, options);
        }

        if ((strstr(options, "l") != NULL && S_ISLNK(statbuf.st_mode)) ||
            (strstr(options, "d") != NULL && S_ISDIR(statbuf.st_mode)) ||
            (strstr(options, "f") != NULL && S_ISREG(statbuf.st_mode)) ||
            (strcmp(options, "") == 0 && (S_ISLNK(statbuf.st_mode) || S_ISDIR(statbuf.st_mode) || S_ISREG(statbuf.st_mode)))) {
            printf("%s\n", fullpath + 2);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    setlocale(LC_COLLATE, "");

    const char *path = (argc > 1) ? argv[1] : ".";
    char options[10] = ""; // Создаем буфер для опций

    int opt;
    while ((opt = getopt(argc, argv, "ldfs")) != -1) {
        switch (opt) {
            case 'l':
            case 'd':
            case 'f':
            case 's':
                strcat(options, (char[]){opt, '\0'}); // Копируем опцию в буфер
                break;
            default:
                break;
        }
    }

    dirwalk(path, options);

    return 0;
}

