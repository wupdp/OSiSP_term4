#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <locale.h>
#include <getopt.h>

#define PATH_MAX 4096

int compare_strings(const void *a, const void *b) {
    return strcoll(*(const char **)a, *(const char **)b);
}

void dirwalk(const char *path, const char *options) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    char **files = NULL;
    int files_count = 0;

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
            char *filename = strdup(entry->d_name);
            files = realloc(files, (files_count + 1) * sizeof(char *));
            files[files_count++] = filename;
        }
    }

    closedir(dir);

    if (strstr(options, "s") != NULL) {
        qsort(files, files_count, sizeof(char *), compare_strings);
    }

    for (int i = 0; i < files_count; ++i) {
        printf("%s/%s\n", path + 2, files[i]);
        free(files[i]);
    }
    free(files);
}

int main(int argc, char *argv[]) {
    //setlocale(LC_COLLATE, "C");

    const char *path = (argc > 1) ? argv[1] : ".";
    char options[10] = "";

    int opt;
    while ((opt = getopt(argc, argv, "ldfs")) != -1) {
        switch (opt) {
            case 'l':
            case 'd':
            case 'f':
            case 's':
                strcat(options, (char[]){opt, '\0'});
                break;
            default:
                break;
        }
    }

    dirwalk(path, options);

    return 0;
}

