#include  <stdio.h>
#include  <sys/types.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <stdlib.h>

//"							//commented quotes
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char *server_ip = NULL;
char *server_port = NULL;

void error(char *msg)
{
    perror(msg);
}


void child_handler(int sig){
	int *status;
	int pid = waitpid(-1,status,WNOHANG);
	if(pid<0){
		error("Error in waitpid ");
	}
	else if(pid==0){
		printf("Child  not yet terminated\n");
	}
	else
    printf("Reaping child : %d\n",pid);

}

char **tokenize(char *line, int *token_size)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  *token_size = tokenNo;
  return tokens;
}


void server(char **tokens,int token_size){
  if(token_size > 3){
    printf("Arguments more than expected. cd Takes only one arrgument\n");
  }
  else{
    server_ip = (char*)malloc(sizeof(tokens[1]));
    strcpy(server_ip , tokens[1]);
    server_port = (char*)malloc(sizeof(tokens[2]));
    strcpy(server_port, tokens[2]);
    printf("Got server and port : %s, %s\n",server_ip,server_port);
    }
  }

void cd(char **tokens,int token_size){
  if(token_size > 2){
    printf("Arguments more than expected. cd Takes only one arrgument\n");
  }
  else{
    int status = chdir(tokens[1]);
    if(status == -1){
      error("Error"); 
    }
    else{
    	printf("In directory : %s\n",tokens[1]);
    }
  }
}

void getfl(char **tokens,int token_size,char *mode){
  pid_t pid;

  if(server_ip == NULL || server_port == NULL){
    printf("Error : Server details not set");
    return;
  }

  else 
  {
    // for single file download without redirection.
    if(token_size == 2){
      pid = fork();
      if (pid == -1) error("Error in fork ");
      else if (pid > 0)
      {
          int status;
          if(waitpid(pid,&status,0)<0){
          	error("Child already reaped  by handler\n");
          }
          else{
          	printf("Child reaped, in function\n");
          }
      }
      else 
      {      
        if(execl("get-one-file-sig","get-one-file-sig",tokens[1],server_ip,server_port,mode,(char*)NULL)<0){
        	error("Error in exec ");
        	exit(1);  // exec never returns
        }
      }
    }

    else{
      // For single file download with io redirection.
      if(strcmp(tokens[2],">") == 0){
        // Error handling. Few or more arguments.
        if(token_size > 4 || tokens[3] == NULL){
          printf("Error : Illegal command format\n");
          return;
        }
        // Input format is correct
        else{
          pid = fork();
          if (pid == -1) error("Error in fork ");
          else if (pid > 0)
          {
	          int status;
	          if(waitpid(pid,&status,0)<0){
	          	error("Child already reaped  by handler\n");
	          }
	          else{
	          	printf("Child reaped, in function\n");
	          }


          }
          else 
          {      
            close(1);
            open(tokens[3],O_RDONLY);
            if(execl("get-one-file-sig","get-one-file-sig",tokens[1],server_ip,server_port,mode,(char*)NULL)<0){
		    	error("Error in exec ");
		    	exit(1);  // exec never returns    
	        }
          }	
        }
      }

      // For single file download with piping.
    else if(strcmp(tokens[2],"|") == 0){
        // Error handling. Less arguments.
        if(tokens[3] == NULL){
          printf("Error : Illegal command format\n");
          return;
        }
        // Input format is correct
        else{
          pid = fork();
          if (pid == -1) error("Error in fork ");
          else if (pid > 0)
          {
	          int status;
	          if(waitpid(pid,&status,0)<0){
	          	error("Child already reaped  by handler\n");
	          }
	          else{
	          	printf("Child reaped, in function\n");
	          }

          }
          else 
          { 
            int p[2];
            if( pipe(p)<0)
            {
                fprintf(stderr, "Pipe Initiation Error");
                return;
            }
            //writer process

            if(fork()==0)
            { 
              close(1);
              dup(p[1]);
              close(p[0]);
              close(p[1]);
              execl("get-one-file-sig","get-one-file-sig",tokens[1],server_ip,server_port,mode,(char*)NULL);
              
            }
            //reader process
            if(fork()==0)
            { 
              close(0);
              dup(p[0]);
              close(p[0]);
              close(p[1]);
              execv(strcat("../bin/",tokens[3]), tokens+3);
              
            }
            close(p[0]);
            close(p[1]);
            wait(NULL);
            wait(NULL);
          }
        }

      

      }
      else{
        printf("Error : Illegal command format (Takes only one file as argument)\n");
        return;
      }
    }
  }
}

void getbg(char **tokens,int token_size,char *mode){
  if(token_size == 2){
    int pid = fork();
    if (pid == -1) error("Error in fork ");
    else if (pid > 0)
    {
    	;
    }
    else 
    { 
	    if(execl("get-one-file-sig","get-one-file-sig",tokens[1],server_ip,server_port,mode,(char*)NULL)<0){
	    	error("Error in exec ");
	    	exit(1);  // exec never returns
	  	}
    }
  }
  else{
    printf("Error : Too many or too few arguments\n");
    return;
  }
}



void getsq(char **tokens,int token_size){
  char **args = malloc(2 * sizeof *args + (2 * (MAX_TOKEN_SIZE * sizeof **args)));
  strcpy(args[0],"getfl");
  if(token_size == 1){
    printf("Error : Too few arguments\n");
    return;
  }
  else{
    int i=1;

    while(tokens[i] != NULL){
      strcpy(args[1],tokens[i]);
      getfl(args,2,"nodislay");
    }
  }
  free (args);
}


void getpl(char **tokens,int token_size){
  char **args = malloc(2 * sizeof *args + (2 * (MAX_TOKEN_SIZE * sizeof **args)));
  strcpy(args[0],"getfl");
  if(token_size == 1){
    printf("Error : Too few arguments\n");
    return;
  }
  else{
    int i=1;
    int pid = fork();
    if (pid == -1) error("Error in fork ");
    else if (pid > 0)
    {
          int status;
          if(waitpid(pid,&status,0)<0){
          	error("Child already reaped  by handler\n");
          }
          else{
          	printf("Child reaped, in function\n");
          }
    }
    else 
    { 
      while(tokens[i] != NULL){
        strcpy(args[1],tokens[i]);
        getbg(args,2,"nodislay");
      }
      free (args);
      exit(1);
    }

  free (args);
  }
}


void  main(void)
{

	struct sigaction sa;
	sa.sa_handler = child_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGCHLD, &sa, 0) == -1) {
	  error("Error in sigaction ");
	  exit(1);
	}

     char  line[MAX_INPUT_SIZE];            
     char  **tokens;              
     int i;
     int *token_size = (int*)malloc(sizeof(int));

     while (1) {           
       
       printf("Hello>");     
       bzero(line, MAX_INPUT_SIZE);
       gets(line);           
       printf("Got command %s\n", line);
       line[strlen(line)] = '\n'; //terminate with new line
       tokens = tokenize(line,token_size);
   
        if(strcmp(tokens[0],"cd") == 0){
          cd(tokens,*token_size);
        }

        else if(strcmp(tokens[0],"server") == 0){
          server(tokens,*token_size);
        }
        

        else if(strcmp(tokens[0],"getfl") == 0){
          getfl(tokens,*token_size,"display");
        }

        else if(strcmp(tokens[0],"getsq") == 0){
          getsq(tokens,*token_size);
        }

        // else if(strcmp(tokens[0],"getpl") == 0){
        //   getpl(tokens,*token_size);
        // }

        // else if(strcmp(tokens[0],"getbg") == 0){
        //   getbg(tokens,*token_size);
        // }
       // Freeing the allocated memory	
       for(i=0;tokens[i]!=NULL;i++){
	       free(tokens[i]);
       }
       free(tokens);
     }
   free(server_port);
   free(server_ip);
}

                





// if( pipe(p)<0)
//       {
//           fprintf(stderr, "Pipe Initiation Error");
//           return;x`
//       }
//       //writer process

//       if(fork()==0)
//       { 
//         close(1);
//         dup(p[1]);
//         close(p[0]);
//         close(p[1]);
//         execv("client", pcmd->left->argv);
        
//       }
//       //reader process
//       if(fork()==0)
//       { 
//         close(0);
//         dup(p[0]);
//         close(p[0]);
//         close(p[1]);
//         execvp(pcmd->right->argv[0], pcmd->right->argv);
        
//       }
//       close(p[0]);
//       close(p[1]);
//       wait(NULL);
//       wait(NULL);
//       break;  