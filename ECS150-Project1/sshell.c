#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define CMDLINE_MAX 512
#define ARG_MAX 16
#define PIPE_MAX 3
struct pipeline
{
    char *cmds[PIPE_MAX + 1];
    int size;
    int eRedirect[2];
};
int execution(const char *cmd, char *const argv[], char *redirFile, struct pipeline *pipeObj);
void parsePipe(const char *cmd, struct pipeline *pipeObj);
int parseArgv(const char *cmd, char *argv[]);
int pwd();
int cd(const char *dir);
int sls();
void pipeline2(struct pipeline *pipeObj);
void pipeline3(struct pipeline *pipeObj);

int main(void)
{
    char cmd[CMDLINE_MAX];

    struct pipeline pipeObj;
    for (size_t i = 0; i < 2; i++)
    {
        pipeObj.eRedirect[i] = 0;
    }
    while (1)
    {
        char *argv[ARG_MAX + 1];
        char *nl;
        int status;

        /* Print prompt */
        printf("sshell@ucd$ ");
        fflush(stdout);

        /* Get command line */
        fgets(cmd, CMDLINE_MAX, stdin);

        /* Print command line if stdin is not provided by terminal */
        if (!isatty(STDIN_FILENO))
        {
            printf("%s", cmd);
            fflush(stdout);
        }

        /* Remove trailing newline from command line */
        nl = strchr(cmd, '\n');
        if (nl)
            *nl = '\0';

        /* Output redirection */
        char *outputFile;
        outputFile = strchr(cmd, '>');
        if (outputFile)
        {
            /* remove output file from command and put dest file name to outputFile */
            *outputFile = '\0';
            *outputFile++;
        }

        /* Read in argument */
        if (!strchr(cmd, '|'))
        {
            if (parseArgv(cmd, argv))
            {
                fprintf(stderr, "Error: too many process arguments\n");
                continue;
            }
        }

        /* Builtin command - exit */
        if (!strcmp(cmd, "exit"))
        {
            fprintf(stderr, "Bye...\n+ completed '%s' [%d]\n", cmd, 0);
            break;
        }
        /* Builtin command - pwd */
        if (!strcmp(cmd, "pwd"))
        {
            status = pwd();
        }
        /* Buildin command - cd */
        else if (!strcmp(argv[0], "cd"))
        {
            status = cd(argv[1]);
        }
        else if (!strcmp(argv[0], "sls"))
        {
            /* Custom command - sls */
            status = sls();
        }
        else
        {
            /* Regular command */
            parsePipe(cmd, &pipeObj);
            // parseArgv(cmd, argv);
            status = execution(argv[0], argv, outputFile, &pipeObj);
        }
        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, status);
    }

    return EXIT_SUCCESS;
}

int execution(const char *cmd, char *const argv[], char *redirFile, struct pipeline *pipeObj)
{
    pid_t pid = fork();
    int status;
    int special = 0;
    if (pid == 0)
    {
        /*child process*/
        if (redirFile)
        {
            if (redirFile[0] == '&')
            {
                special = 1;
                *redirFile++;
                fprintf(stderr, "%s \n", redirFile);
            }

            char *fullLine[CMDLINE_MAX];
            char *redcpy = strdup(redirFile);
            const char space[2] = " ";
            char *check;
            check = strtok(redcpy, space);
            fullLine[0] = check;
            // printf("fullLine[%d] = %s\n", 0, fullLine[0]);
            int i = 1;
            while (check != NULL)
            {
                check = strtok(NULL, space);
                fullLine[i] = check;
                // printf("fullLine[%d] = %s\n", i, fullLine[i]);
                i++;
            }

            int fd = open(fullLine[0], O_CREAT | O_WRONLY, S_IREAD | S_IWRITE);
            if (fd == -1)
            {
                fprintf(stderr, "Error: cannot open output file\n");
                return 1;
            }
            else
            {
                if (special)
                {
                    dup2(fd, STDERR_FILENO);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }
        }
        if (pipeObj->size == 2)
        {
            pipeline2(pipeObj);
        }
        else if (pipeObj->size == 3)
        {
            pipeline3(pipeObj);
        }
        else
        {
            execvp(cmd, argv);
        }
    }
    else
    {
        waitpid(pid, &status, 0);
    }
    return WEXITSTATUS(status);
}

void pipeline2(struct pipeline *pipeObj)
{
    char *process1 = pipeObj->cmds[0];
    char *process2 = pipeObj->cmds[1];
    pid_t pid = fork();
    int status;
    if (pid != 0)
    {
        waitpid(pid, &status, 0);
    }
    else
    {
        int fd[2];
        pipe(fd);
        if (fork() != 0)
        {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            char *argv2[ARG_MAX];
            parseArgv(process2, argv2);
            execvp(argv2[0], argv2);
        }
        else
        {
            close(fd[0]);
            dup2(fd[1], STDOUT_FILENO);
            if (pipeObj->eRedirect[0] == 1)
            {
                dup2(fd[1], STDERR_FILENO);
            }
            close(fd[1]);
            char *argv1[ARG_MAX];
            parseArgv(process1, argv1);
            execvp(argv1[0], argv1);
        }
    }
}

void pipeline3(struct pipeline *pipeObj)
{
    char *process1 = pipeObj->cmds[0];
    char *process2 = pipeObj->cmds[1];
    char *process3 = pipeObj->cmds[2];
    pid_t pid = fork();
    int status;
    if (pid != 0)
    {
        waitpid(pid, &status, 0);
    }
    else
    {
        int fd[2];
        pipe(fd);
        int fd2[2];
        pipe(fd2);
        // I can print out the pipe number and see which process is taking from what.
        if (fork() != 0)
        {
            close(fd[0]);
            close(fd[1]);
            close(fd2[1]);
            //            fprintf(stderr,"Process 3 read from %d\n",fd2[0]);
            dup2(fd2[0], STDIN_FILENO);
            close(fd2[0]);
            char *argv3[ARG_MAX];
            parseArgv(process3, argv3);
            execvp(argv3[0], argv3);
        }
        else
        {
            if (fork() != 0)
            {

                close(fd[0]);
                //                fprintf(stderr,"Process 1 output to %d\n",fd[1]);
                dup2(fd[1], STDOUT_FILENO);
                if (pipeObj->eRedirect[0] == 1)
                {
                    dup2(fd[1], STDERR_FILENO);
                }
                close(fd[1]);
                char *argv1[ARG_MAX];
                parseArgv(process1, argv1);
                execvp(argv1[0], argv1);
            }
            else
            {

                close(fd[1]);
                //                fprintf(stderr,"Process 2 read from %d\n",fd[0]);
                dup2(fd[0], STDIN_FILENO);
                close(fd[0]);
                close(fd2[0]);
                //                fprintf(stderr,"Process 2 output to %d\n",fd[1]);
                dup2(fd2[1], STDOUT_FILENO);
                if (pipeObj->eRedirect[1] == 1)
                {
                    dup2(fd2[1], STDERR_FILENO);
                }
                close(fd2[1]);

                char *argv2[ARG_MAX];
                parseArgv(process2, argv2);
                execvp(argv2[0], argv2);
            }
        }
    }
}

void parsePipe(const char *cmd, struct pipeline *pipeObj)
{
    int i = 0;
    char *cmd_cpy = strdup(cmd);
    char *cp = strtok(cmd_cpy, "|");
    pipeObj->cmds[i++] = cp;

    cp = strtok(NULL, "|");
    while (cp)
    {
        if (cp[0] == '&')
        {
            pipeObj->eRedirect[i - 1] = 1;
            *cp++;
        }
        pipeObj->cmds[i++] = cp;

        cp = strtok(NULL, "|");
    }
    pipeObj->cmds[i] = NULL;
    pipeObj->size = i;
}
int parseArgv(const char *cmd, char *argv[])
{
    int i = 0;
    char *cmd_cpy = strdup(cmd);
    char *cp = strtok(cmd_cpy, " ");
    argv[i++] = cp;

    cp = strtok(NULL, " ");
    while (cp)
    {
        if (i > 15)
        {
            return 1;
        }
        argv[i++] = cp;

        cp = strtok(NULL, " ");
    }
    argv[i] = NULL;
    return 0;
}

int sls()
{
    int status;
    size_t size = PATH_MAX + 1;
    char *buffer = (char *)malloc(size);
    char *current_directory = getcwd(buffer, size);
    DIR *dirp;
    struct dirent *dp;
    dirp = opendir(current_directory);
    if (dirp == NULL)
    {
        fprintf(stderr, "Error: cannot open directory]\n");
    }
    struct stat slsbuf;
    status = 1;
    while ((dp = readdir(dirp)) != NULL)
    {
        if (dp->d_name[0] == '.')
        {
            continue;
        }
        if (stat(dp->d_name, &slsbuf) == -1)
        {
            printf("Can not get stat");
            continue;
        }
        printf("%s (%ld bytes)\n", dp->d_name, slsbuf.st_size);
        status = 0;
    }
    return status;
}
int pwd()
{
    // can also use char cwd[PATH_MAX] instead of malloc
    char *buffer = (char *)malloc(PATH_MAX);
    if (getcwd(buffer, PATH_MAX))
    {
        printf("%s%c", buffer, '\n');
        free(buffer);
        return 0;
    }
    return 1;
}

int cd(const char *dir)
{
    if (chdir(dir))
    {
        fprintf(stderr, "Error: cannot cd into directory\n");
        return 1;
    }
    return 0;
}