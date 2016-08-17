#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>
#include <signal.h>


int bytes_downloaded=0;


void download_handler(int sig){

    printf("Received SIGINT; downloaded %d  bytes so far.\n",bytes_downloaded);
    exit(0);

}
void error(char *msg)                       
{
    perror(msg);
    exit(0);
}
   
int main(int argc, char *argv[])
{
    struct sigaction new_action;                        //use sigaction() instead of signal()
    new_action.sa_handler=download_handler;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT,&new_action,NULL);
    sigaction(SIGTERM,&new_action,NULL);

    if (argc < 5) {                     //check if user aruguments are correct
       fprintf(stderr,"usage %s filename hostname port mode ", argv[0]);
       exit(0);
    }

    int display = 0;
    if(strcmp("display",argv[4])==0){
        display=1;
    }
    else if(strcmp("no display",argv[4])!=0){
        fprintf(stderr,"ERROR, no such mode\n");   
        exit(1);             
    }    

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

        /* create socket, get sockfd handle */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0){
            error("ERROR opening socket");
            exit(1);
        }
        /* fill in server address in sockaddr_in datastructure */

        server = gethostbyname(argv[2]);
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            close(sockfd);
            exit(1);
        }
        bzero((char *) &serv_addr, sizeof(serv_addr));          //clear serv_addr
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
             (char *)&serv_addr.sin_addr.s_addr,
             server->h_length);          
       serv_addr.sin_port = htons(atoi(argv[3]));

        /* connect to server */

        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
            error("ERROR connecting");
            close(sockfd);
        }
        // printf("Connected to server \n");

        /* send user message to server */


        char buffer[256];
        strcpy(buffer,argv[1]);

        // printf("Writing : %s to socket \n",buffer);

        n = write(sockfd,buffer,strlen(buffer));                //write the request message to server
        if (n < 0) {
            error("ERROR writing to socket");
            close(sockfd);
        }


        char read_buffer[1024];
        /* read reply from server */
        while(1){
            n = read(sockfd,read_buffer,1023);
            if (n < 0) {
                 error("ERROR reading from socket");
                 break;
            }
            bytes_downloaded+=n;
            if(n==0){                           
                // printf("File read completely \n");
                break;
            }
            if(display==1){
                // printf("Displaying %d read bytes\n",n);
                read_buffer[n]='\0';                            //add termintaion chartacter to string to print on terminal
                printf("%s",read_buffer);                                          
            }
        }
        close(sockfd);

    return 0;
}
