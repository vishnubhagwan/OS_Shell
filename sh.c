#include<stdio.h>
#include<string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <fcntl.h>

#define MAX_LINE 100

extern int errno;

int main(void)
{
	char inputBuffer[MAX_LINE];
	int bg=0;
	char *args[MAX_LINE+1];
	while(1)
	{
		fflush(0);
		char hostname[1024];
		hostname[1023] = '\0';
		gethostname(hostname, 1023);
 		printf("%s@", hostname);
		struct utsname unameData;
		uname(&unameData);
		printf("%s ", unameData.sysname);
		long size;
		char *buf;
		char *ptr;
		int cnt = 0, cnt1 = 0, redir = 0, fdIn, fdOut, err;
		size = pathconf(".", _PC_PATH_MAX);
		if ((buf = (char *)malloc((size_t)size)) != NULL)
			ptr = getcwd(buf, (size_t)size);
		printf("%s $ ",ptr);
		char str[MAX_LINE];
		strcpy(str,"");
		scanf("%[^\n]",str);
		getchar();
		if(strcmp(str,"") == 0) continue;
		bg = 0;
		int l = strlen(str);
		if(str[l-1] == '&')
		{
			str[l-1] = '\0';
			bg = 1;
		}
		char* token = strtok(str, " ");
		args[cnt] = token;
		while(token != NULL)
		{
			cnt++;
			token = strtok(NULL, " ");
			args[cnt] = token;
			if(token && (!strcmp(token, "<") || !strcmp(token, ">")))
				redir = 1;
		}
		args[cnt+1] = 0;
		pid_t pid; //Setting up PID
		int status = 0, wpid = 0;
		if(strcmp(args[0],"cd") == 0)
		{
			if (args[1] == NULL)
				chdir(getenv("HOME"));
			else if (chdir(args[1]) == -1)
				perror("Error");
			continue;
		}
		pid = fork();
		if(pid == -1)
			perror("Fork Failed\n");
		else if(pid == 0)
		{
			cnt1 = 0;
			while(args[cnt1] != NULL)
			{
				if(!strcmp(args[cnt1], "<"))
				{
					if(args[cnt1+1] != NULL)
					{
						fdIn = open(args[cnt1+1], O_RDONLY);
						if(fdIn == -1)
						{
							perror("Error");
							break;
						}
						err = dup2(fdIn, STDIN_FILENO);
						if(err == -1)
						{
							perror("Error");
							break;
						}
					}
					else
					{
						printf("No output file specified");
						break;
					}
					args[cnt1] = 0;
				}
				else if(!strcmp(args[cnt1], ">"))
				{
					if(args[cnt1] != NULL)
					{
						fdOut = open(args[cnt1+1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
						if(fdOut == -1)
						{
							perror("Error");
							break;
						}
						err = dup2(fdOut, STDOUT_FILENO);
						if(err == -1)
						{
							perror("Error");
							break;
						}
					}
					else
					{
						printf("No output file specified");
						break;
					}
					args[cnt1] = 0;
				}
				else if(!strcmp(args[cnt1], "|"))
				{
					int in = 0, fd[2];
					if(args[cnt1] != NULL)
					{
						pipe(fd);
						pid_t pid1;
						if (in != 0)
			        	{
       						dup2 (in, 0);
							close (in);
						}

						if (fd[1] != 1)
						{
							dup2 (fd[1], 1);
							close (fd[1]);
						}
					}
					args[cnt1] = 0;
				}
				cnt1++;
			}
			execvp(args[0],args);
			perror("Error");
		}
		else
		{
			wpid = waitpid(pid, &status,bg==1?WNOHANG:0);
			if(wpid == 0)
				printf("     **************Background Child process Done**************\n");
		}
	}
	return 0;
}
