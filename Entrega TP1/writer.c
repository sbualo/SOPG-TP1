#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FIFO_NAME "commfifo"
#define BUFFER_SIZE 300
#define HEADER_SIZE 5 

#define HEADER_TYPE_DATA -1
#define HEADER_TYPE_SIGNAL 1

char outputBuffer[BUFFER_SIZE];
uint32_t bytesWrote;
int32_t returnCode, fd;

char s[200];
struct sigaction sa_user1, sa_user2;

void siguserone_handler(int sig);
void sigusertwo_handler(int sig);
void addHeader(char *,uint32_t* bufferSize, int isSignal);

int main(void)
{
   

    sa_user1.sa_handler = siguserone_handler;
    sa_user1.sa_flags = SA_RESTART; //SA_RESTART;
    sigemptyset(&sa_user1.sa_mask);
    if (sigaction(SIGUSR1, &sa_user1, NULL) == -1) { 
        perror("sigaction");
        exit(1); 
    }


    sa_user2.sa_handler = sigusertwo_handler;
    sa_user2.sa_flags = SA_RESTART; //SA_RESTART;
    sigemptyset(&sa_user2.sa_mask);
    if (sigaction(SIGUSR2, &sa_user2, NULL) == -1) { 
        perror("sigaction");
        exit(1); 
    }

    /* Create fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for readers...\n");
	if ( (fd = open(FIFO_NAME, O_WRONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a reader--type some stuff\n");

    /* Loop forever */
	while (1)
	{
        /* Get some text from console */
		fgets(outputBuffer, BUFFER_SIZE, stdin);
        
        uint32_t bufferSize = strlen(outputBuffer);
        addHeader(outputBuffer,&bufferSize,HEADER_TYPE_DATA);

        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
		if ((bytesWrote = write(fd, outputBuffer, bufferSize)) == -1)
        {
			perror("write");
        }
        else
        {
			printf("writer: wrote %d bytes\n", bytesWrote);
        }
	}
	return 0;
}



void siguserone_handler(int sig){

    printf("siguserone detected \n");
    char * outputStr = "SIGUSR1 Detected -> logging activity";

    
    uint32_t strSize = strlen(outputStr);
    
        for (int i = 0; i <strSize;i++){
        outputBuffer[i] = *(outputStr+i);
    }

    addHeader(outputBuffer,&strSize,HEADER_TYPE_SIGNAL);

    if ((bytesWrote = write(fd, outputBuffer, strSize)) == -1)
    {
        perror("write");
    }
    else
    {
        printf("Signal Logged\n\r");
    }
}

void sigusertwo_handler(int sig){
    char * outputStr = "SIGUSR2 Detected -> logging activity";

    
    uint32_t strSize = strlen(outputStr);
    
        for (int i = 0; i <strSize;i++){
        outputBuffer[i] = *(outputStr+i);
    }

    addHeader(outputBuffer,&strSize,HEADER_TYPE_SIGNAL);

    if ((bytesWrote = write(fd, outputBuffer, strSize)) == -1)
    {
        perror("write");
    }
    else
    {
        printf("Signal Logged");
    }
}

void addHeader(char * buffer,uint32_t * bufferSize, int isSignal){

uint32_t buffSize = *bufferSize;

if (bufferSize>0 ){
    for(int j=buffSize ; j>=0;j--){
        buffer[j+HEADER_SIZE] = buffer[j];
    }

if (isSignal<0){
    buffer[0]='D';
    buffer[1]='A';
    buffer[2]='T';
    buffer[3]='A';
    buffer[4]=':';
} else if (isSignal>0){
    buffer[0]='S';
    buffer[1]='I';
    buffer[2]='G';
    buffer[3]='N';
    buffer[4]=':';
}
}

*bufferSize = buffSize + HEADER_SIZE;
printf("%s",buffer);
}
