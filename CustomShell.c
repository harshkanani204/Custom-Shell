#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define GREEN   "\x1b[32m"
#define RED     "\x1b[31m"
#define BLUE    "\x1b[34m"
#define WHITE   "\x1b[0m"

void removeSpace(char* buf)
{
	if(buf[0]==' ') 
    {
        memmove(buf, buf+1, strlen(buf));
    }

    if(buf[0] == '\n')
    {
        memmove(buf, buf+1, strlen(buf));
    }

	if(buf[strlen(buf)-1]=='\n')
    {
        buf[strlen(buf)-1]='\0';
    }

    if(buf[strlen(buf)-1]==' ')
    {
        buf[strlen(buf)-1]='\0';
    }
}

void bufferTokenization(char** param,int *nr,char *buf,const char *c)
{
	int pc = -1;
	char *token;
	token = strtok(buf,c);
	
    while(1)
    {
        if(!(token))    break;

		param[++pc] = malloc(sizeof(token)+1);
		strcpy(param[pc],token);
		removeSpace(param[pc]);
		token = strtok(NULL,c);
	}

	param[++pc] = NULL;
	*nr = pc;
}

void exeBase(char** argv)
{
	if(fork()<=0)
    {
		execvp(argv[0],argv);
		perror(RED   "invalid input"   WHITE "\n");
		exit(1);
	}

	else
    {
		wait(NULL);
	}
}

void exePipe(char** buf,int nr)
{
	if(nr>20)
    {
        perror("Can't use more than 20 piped commands\n");
        return;
    }
	
	char *argv[100];
	int fd[20][2],i,pc;

	for(i=0;i<nr;i++)
    {
		bufferTokenization(argv,&pc,buf[i]," ");
		if(i != nr-1 && (pipe(fd[i]) < 0))
        {
            perror("pipe creating was not successfull\n");
            return;
		}

		if(fork() == 0)
        {
			if(i != nr-1)
            {
				dup2(fd[i][1],1);
				close(fd[i][0]);
				close(fd[i][1]);
			}

			if(i != 0)
            {
				dup2(fd[i-1][0],0);
				close(fd[i-1][1]);
				close(fd[i-1][0]);
			}

			execvp(argv[0],argv);
			perror("invalid input ");
			exit(1);    //if exec is not successful...
		}

		if(i != 0)
        {
			close(fd[i-1][0]);
			close(fd[i-1][1]);
		}

		wait(NULL);
	}
}

void exeAsync(char** buf,int nr)
{
	char *argv[100];
	int i,pc;

	for(i=0;i<nr;i++)
    {
		bufferTokenization(argv,&pc,buf[i]," ");
		if(fork()==0)
        {
			execvp(argv[0],argv);
			perror("invalid input ");
			exit(1);    //if exec is not successful
		}
	}

}

void exeRedirect(char** buf,int nr,int mode)
{
	int pc,fd;
	char *argv[100];
	removeSpace(buf[1]);
	bufferTokenization(argv,&pc,buf[0]," ");

	if(fork()==0)
    {

		switch(mode){
		case 0:  fd=open(buf[1],O_RDONLY); break;
		case 1: fd=open(buf[1],O_WRONLY); break;
		case 2: fd=open(buf[1],O_WRONLY | O_APPEND); break;
		default: return;
		}

		if(fd<0)
        {
			perror("cannot open file\n");
			return;
		}

		switch(mode){
		case 0:  		dup2(fd,0); break;
		case 1: 		dup2(fd,1); break;
		case 2: 		dup2(fd,1); break;
		default: return;
		}
		
        execvp(argv[0],argv);
		perror("invalid input ");
		exit(1);//in case exec is not successfull, exit
	}

	wait(NULL);
}

void handle_sigint(int sig)
{
    printf("\nCaught Signal %d\n", sig);
    exit(1);
}

int main(char** argv,int argc)
{
	int nr=0;
	char buf[500],*buffer[100],buf2[500],buf3[500], *params1[100],*params2[100],*token,cwd[1024];
	printf(GREEN   "    ==========SHELL==========    "   WHITE "\n");

	while(1)
    {
        signal(SIGINT, handle_sigint);
        signal(SIGTSTP, handle_sigint);
		if(getcwd(cwd, sizeof(cwd)) == NULL)
        {
         	perror("getcwd failed\n");
        }    

		else
        {
            printf(RED "%s" WHITE, cwd);
            printf(BLUE "$  " WHITE);
        }

	    fgets(buf, 500, stdin);

		if(strchr(buf,'|'))
        {
			bufferTokenization(buffer,&nr,buf,"|");
			exePipe(buffer,nr);
		}

		else if(strchr(buf,'&'))
        {   
			bufferTokenization(buffer,&nr,buf,"&");
			exeAsync(buffer,nr);
		}

		else if(strstr(buf,">>>"))
        {
			bufferTokenization(buffer,&nr,buf,">>>");
			if(nr==2)
            {
                exeRedirect(buffer,nr,2);
            }
			
            else 
            {
                printf("Incorrect output redirection!(has to to be in this form: command >> file)");
            }
        }

		else if(strchr(buf,'>'))
        {
			bufferTokenization(buffer,&nr,buf,">");
			if(nr==2)
            {
                exeRedirect(buffer,nr, 1);
            }
            else
            {
                printf("Incorrect output redirection!(has to to be in this form: command > file)");
            }
        }

		else if(strstr(buf,"<<<"))
        {
			bufferTokenization(buffer,&nr,buf,"<<<");
			
            if(nr==2)
            {
                exeRedirect(buffer,nr, 0);
            }
            
            else 
            {
                printf("Incorrect input redirection!(has to to be in this form: command < file)");
            }
        }

		else
        {
			bufferTokenization(params1,&nr,buf," ");
			
            if(strstr(params1[0],"exit"))
            {
				exit(0);
			}

			else if(strstr(params1[0],"cd"))
            {
				chdir(params1[1]);
			}

			else exeBase(params1);
		}
	}

	return 0;
}
