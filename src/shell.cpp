#include  <stdio.h>
#include  <sys/types.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <unordered_set>
#include <stdlib.h>
#include <bits/stdc++.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include <string>
#include <map>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <string.h>
#include <string>
#include <fcntl.h>
#include <iostream>
#include <set>
#include <vector>
using namespace std;

//"							//commented quotes
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char *server_ip = NULL;
char *server_port = NULL;


unordered_set<int> bg_process;
unordered_set<int> bg_process_exit;


void error(char *msg)
{
    perror(msg);
}



void handler(int sig){
	if(sig==SIGCHLD){
		int *status;
		int pid = waitpid(-1,status,WNOHANG);
		if(pid<0){
			;
			//error("Error in waitpid ");
		}
		else if(pid==0){
			printf("Child  not yet terminated\n");
		}
		else{
		    printf("Reaping background child  proess : %d\n",pid);
		    if(bg_process.find(pid)!=bg_process.end() && bg_process_exit.find(pid)==bg_process_exit.end()){
		    	printf("background process completed\n");
		    }
		    else{
		    	printf("background process terminated by exit\n");
		    }
		}
	}
	else if (sig==SIGINT){
		printf("SIGINT received in shell process");				//dont close the shell on SIGINT
	}
	else if (sig==SIGTERM){
		printf("Ignore SIGTERM in shell process\n");		//ignore SIGTERM when exit is called
    int status;
		while(1){
	    	int m=waitpid(-1,&status,WNOHANG);
	    	if(m==0||m==-1)break;
	    	printf("Cleaned up foreground child process in exit : %d\n",m);	    	
	    }
	    free(server_port);
   		free(server_ip);
	    exit(0);			//exit the shell

	}
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
  // out<<token_size<<end;
  if(token_size > 3){
    printf("Arguments more than expected. server Takes only two arrgument\n");
  }
  else if(token_size <3){
    printf("Arguments less than expected. server Takes only two arrgument\n");
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
    // for single file download withthout redirection.
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

void getbg(char **tokens,int token_size,char *mode){			//print to shell when completed
  if(token_size == 2){
    int pid = fork();
    if (pid == -1) error("Error in fork ");
    else if (pid > 0)
    {
        if(setpgid(pid,pid)<0){
          error("ERROR in setting PGID of background process");
        }
    	     bg_process.insert(pid);
            for(unordered_set<int>::iterator it = bg_process.begin();it!=bg_process.end();it++){ 
              cout<<*it<<endl;
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
    printf("Error : Too many or too few arguments\n");
    return;
  }
}



void getsq(char **tokens,int token_size){

  // char args[2][MAX_TOKEN_SIZE];

  char **args = (char **)malloc(2 * sizeof(char *));
  args[0] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
  args[1] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));

  // cout<<"hi"<<endl;
  // int *size;
  // *size=2;
  strcpy(args[0],"getfl");
  if(token_size == 1){
    printf("Error : Too few arguments\n");
    return;
  }
  else{
    int i=1;

    while(tokens[i] != NULL){
      strcpy(args[1],tokens[i]);
      getfl(args,2,"no display");
      i++;
    }
  }
  free (args);
}


void getpl(char **tokens,int token_size){
  char **args = (char **)malloc(2 * sizeof(char *));
  args[0] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
  args[1] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));

  strcpy(args[0],"getfl");

  vector<int>pids;
  int status;

  if(token_size == 1){
    printf("Error : Too few arguments\n");
    return;
  }
  else{
    int i=1;
    while(tokens[i] != NULL){
        int pid = fork();
        if (pid == -1) error("Error in fork ");
        else if (pid == 0)
        {
          strcpy(args[1],tokens[i]);
          getfl(args,2,"no display");
        }
        else{
          pids.push_back(pid);
        }
        i++;        
    }

    for(int i=0;i<pids.size();i++){
      waitpid(pids[i],&status,0);
    }

  free (args);
  }
}


int  main(void)
{

	struct sigaction sa;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	  error("Error in sigaction SIGCHILD");
	  exit(1);
	}
	if (sigaction(SIGINT,&sa, NULL) == -1) {
	  error("Error in sigaction SIGINT ");
	  exit(1);
	}
	if (sigaction(SIGTERM,&sa, NULL) == -1) {
	  error("Error in sigaction SIGTERM ");
	  exit(1);
	}	
    

     char  line[MAX_INPUT_SIZE];            
     char  **tokens;              
     int i;
     int *token_size = (int*)malloc(sizeof(int));

     while (1) {           
       
       bzero(line, MAX_INPUT_SIZE);
       gets(line);           
       printf("Got command %s\n", line);
       line[strlen(line)] = '\n'; //terminate with new line
       tokens = tokenize(line,token_size);
   
        if(strcmp(tokens[0],"cd") == 0){
          cd(tokens,*token_size);
        }

        else if(strcmp(tokens[0],"server") == 0){
          cout<<"hello"<<endl;
          server(tokens,*token_size);
        }
        

        else if(strcmp(tokens[0],"getfl") == 0){
          getfl(tokens,*token_size,"display");
        }

        else if(strcmp(tokens[0],"getsq") == 0){
          getsq(tokens,*token_size);
        }

        else if(strcmp(tokens[0],"getpl") == 0){
          getpl(tokens,*token_size);
        }

        else if(strcmp(tokens[0],"getbg") == 0){
          getbg(tokens,*token_size,"no display");
        }

        else if(strcmp(tokens[0],"exit") == 0){
        	for(unordered_set<int>::iterator it = bg_process.begin();it!=bg_process.end();it++){						//empty the bg_process so that they dont print completed
        		bg_process_exit.insert(*it);
        		kill(*it,SIGINT);		//send SIGINT to background process
        	}
        	kill(0,SIGTERM);			//send to the parent and the foreground processes
        }
        else{
          // cout<<"was"<<endl;
        	// Default system call
        }

       // Freeing the allocated memory	
       for(i=0;tokens[i]!=NULL;i++){
	       free(tokens[i]);
       }
       free(tokens);
     }
   free(server_port);
   free(server_ip);
   return 0;
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