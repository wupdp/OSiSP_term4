#define MAX_LENGTH 256
#define CHILD_NAME_LENGTH 9    // максимальная длина
#define ENV_COUNT 11        // SHELL, HOME, HOSTNAME, LOGNAME, LANG, TERM, USER, LC_COLLATE, PATH, CHILD_PATH, NULL
#define _GNU_SOURCE

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

void sortStrings(char ***strings, int rows);
char** newStrings(size_t rows, size_t columns);
void printStrings(char **strings, size_t rows);
char* findEnv(char **env, char *k, size_t rows);

extern char **environ;

int main(int argc, char *argv[], char *envp[])
{
    if (argc<2)
    {
        fprintf(stderr, "Error! child_file is not provided\n");
        exit(1);
    }
    size_t sizeOfEnv = 0 ;

    while(environ[sizeOfEnv])
        sizeOfEnv++;

    sortStrings(&environ, sizeOfEnv);
    printStrings(environ, sizeOfEnv);

    char *childEnvPath = argv[1];
    size_t childIndex = 0;
    while(1)
    {
        setenv("LC_COLLATE", "C", 1);
        printf("input char: + * & q\n");
        rewind(stdin);
        char opt;
        while(1)
        {
            scanf("%c", &opt);
            if (opt == '+' || opt == '*' || opt == '&' || opt == 'q')
            {
                break;
            }
        }
        char  childName[CHILD_NAME_LENGTH];
        sprintf(childName, "%s%02d", "child_", (int) childIndex);
        char *childArgs[] = {childName, childEnvPath, (char *) 0};
        int statusChild = 0;
        switch (opt)
        {
            case '+':
            {
                pid_t pid = fork();
                if (pid==-1)
                {
                    printf("Error! Error occured, error code - %d\n", errno);
                    exit(errno);
                }
                if (pid == 0)
                {
                    char *CHILD_PATH = getenv("CHILD_PATH");
                    printf("%s\n", CHILD_PATH);
                    printf("Child process created. Please, wait...\n");
                    execve(CHILD_PATH, childArgs, environ);
                }
                wait(&statusChild);
                break;
            }
            case '*':
            {
                pid_t pid = fork();
                if (pid==-1)
                {
                    printf("Error! Error occured, error code - %d\n", errno);
                    exit(errno);
                }
                if (pid == 0)
                {
                    char *CHILD_PATH = findEnv(envp, "CHILD_PATH", sizeOfEnv);
                    printf("Child process created. Please, wait...\n");
                    execve(CHILD_PATH, childArgs, environ);
                }
                wait(&statusChild);
                break;
            }
            case '&':
            {
                pid_t pid = fork();
                if (pid==-1)
                {
                    printf("Error! Error occured, error code - %d\n", errno);
                    exit(errno);
                }
                if (pid == 0)
                {
                    char *CHILD_PATH = findEnv(environ, "CHILD_PATH", sizeOfEnv);
                    printf("Child process created. Please, wait...\n");
                    execve(CHILD_PATH, childArgs, environ);
                }
                wait(&statusChild);
                break;
            }
            case 'q':
            {
                exit(0);
                break;
            }
            default:
                break;
        }
        childIndex++;
        if (childIndex > 99) childIndex = 0;
    }
    return 0;
}

void printStrings(char **strings, size_t rows)
{
    for(size_t i = 0; i < rows; i++)
        fprintf(stdout, "%s\n", strings[i]);
}

void swap(char **s1, char** s2)
{
    char* tmp = (*s1);
    (*s1) = (*s2);
    (*s2) = tmp;
}

void sortStrings(char ***strings, int rows)
{
    for (int i = 0; i < rows - 1; i++)
    {
        for (int j = 0; j < rows - i - 1; j++)
        {
            if (strcoll((*strings)[j],(*strings)[j+1]) > 0)
                swap(&((*strings)[j]),&((*strings)[j+1]));
        }
    }
}

char* findEnv(char **env, char *k, size_t rows)
{
    char* result =(char*)calloc(MAX_LENGTH, sizeof(char));
    for(size_t i = 0; i < rows; i++)
    {
        if(strstr(env[i], k))
            strncpy(result, env[i] + strlen(k) + 1, MAX_LENGTH);
    }
    return result;
}