#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

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

            if (strstr(options, "d") != NULL || strstr(options, "a") != NULL)
                printf("%s/\n", fullpath);

            dirwalk(fullpath, options);
        } else if (S_ISLNK(statbuf.st_mode)) {
            if (strstr(options, "l") != NULL || strstr(options, "a") != NULL)
                printf("%s@\n", fullpath);
        } else if (S_ISREG(statbuf.st_mode)) {
            if (strstr(options, "f") != NULL || strstr(options, "a") != NULL)
                printf("%s\n", fullpath);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";
    const char *options = (argc > 2) ? argv[2] : "a";

    dirwalk(path, options);

    return 0;
}
