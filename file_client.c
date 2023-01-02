#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>

//finds data length
int findLength(char *data)
{
    int length = 0;
    int i=0;
    while(data[i]!='\0'){
        length++;
        i++;
    }
    return length;
}

main()
{
    int fd;
	int isExit = 1;
	char *myfifo = "/tmp/myfifo";
    //created myfifo and setted request 0666
	mkfifo(myfifo, 0666);
    char response2[128];
    char init[128] = "init";
    fd = open(myfifo, O_WRONLY);
    write(fd, init, sizeof(init));
    close(fd);

    while (isExit){
        char data[128];
        char response[128];
        //gets input from user.
        fgets(data, 128, stdin);
        //assign 0 to end of the line.
		if (data[findLength(data) - 1] == '\n'){
            data[findLength(data) - 1] = '\0';
        }
        //myfifo read only mood.
        fd = open(myfifo, O_WRONLY);
        write(fd, data, sizeof(data));
		close(fd);
        //if input is exit, made isexit=0.
        if (strcmp(data, "exit") == 0){
			isExit = 0;
		}
		fd = open(myfifo, O_RDONLY);
		read(fd, response, 128);
		printf("%s\n", response);
	}
	return 0;
}