#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "commfifo"
#define LOG_FILE_NAME "log.txt"
#define SIGNAL_FILE_NAME "signal.txt"
#define BUFFER_SIZE 300

#define HEADER_SIZE 5

void removeHeader(uint8_t * buffer, uint32_t bytesRead);

int main(void)
{
	uint8_t inputBuffer[BUFFER_SIZE];
	int32_t bytesRead, returnCode, fd;

    int32_t fd_log,fd_signals;
    char header[HEADER_SIZE];
    
    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }
    if ( (returnCode = mknod(LOG_FILE_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }
    if ( (returnCode = mknod(SIGNAL_FILE_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }
    
    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for writers...\n");
	if ( (fd = open(FIFO_NAME, O_RDONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a writer\n");


    
    /*Abro los archivos para log y signals*/
    if ( (fd_log = open(LOG_FILE_NAME, O_WRONLY) ) < 0 )
        {
            printf("Error opening text file: %d\n", fd);
            exit(1);
        }
    if ( (fd_signals = open(SIGNAL_FILE_NAME, O_WRONLY) ) < 0 )
    {
        printf("Error opening signal file: %d\n", fd);
        exit(1);
    }

    /* Loop until read syscall returns a value <= 0 */
	do
	{
        /* read data into local buffer */
		if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
        {
			perror("read");
        }
        else
		{
			inputBuffer[bytesRead] = '\0';

            /* Extraigo el header*/
            for (int i = 0; i < HEADER_SIZE; i++){
                header[i]   = inputBuffer[i];
            }
            removeHeader(inputBuffer,bytesRead);
            if (header[0]== 'D' && header[1]== 'A' && header[2]=='T' && header[3] =='A' && header[4]==':'){
               uint32_t bytesWrote;
               if ((bytesWrote = write(fd_log, inputBuffer, strlen((char *)inputBuffer))) == -1)
                    {
                        perror("write on log");
                    } 
            }
            if (header[0]== 'S' && header[1]== 'I' && header[2]=='G' && header[3] =='N' && header[4]==':'){
               uint32_t bytesWrote;
               if ((bytesWrote = write(fd_signals, inputBuffer, strlen(inputBuffer))) == -1)
                    {
                        perror("write on signals");
                    } 
            }
			printf("reader: read %d bytes: \"%s\"\n", bytesRead, inputBuffer);
		}
	}
	while (bytesRead > 0);

	return 0;
}

void removeHeader(uint8_t * buffer, uint32_t bytesRead){

    if (bytesRead >HEADER_SIZE){
        for (int i=HEADER_SIZE;i< BUFFER_SIZE - 2;i++){  
            if (buffer[i]== '\0'){  //busco el terminador y agrego un enter para escribir en el archivo
                buffer[i-HEADER_SIZE] = '\n';
                buffer[i-HEADER_SIZE + 1] = '\r';
                buffer[i-HEADER_SIZE + 2 ] = buffer[i];
                break;
            }
            buffer[i-HEADER_SIZE] = buffer[i];
        }
    }
}