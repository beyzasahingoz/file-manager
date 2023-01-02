#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>

#define MAX_BUF 128
//defined variables
char file_List[10][100];
char response[128];
char c;
int count = 0, id = 0;
pthread_t threads[4];
//found file_list size and assigned to the file_length.
int file_length = sizeof(file_List) / sizeof(file_List[0]);
//defined lock and cond.
pthread_mutex_t lock;
pthread_cond_t cond;
//defined struct params
struct params
{
    char *arg1;
    char *arg2;
};
//initialized functions.
char **generateMatrix(int columns, int rows);
char **splitIntoWords(char *str);
void *createFile(char *args);
void *deleteFile(char *args);
void *readFile(char *args);
void *writeFile(char *args);


int main(){

    char **command;
    void *status;
    int file_read;
    int returnResponse = 0;
    char *myfifo = "/tmp/myfifo";
    char buf[MAX_BUF];
    //file_list is filled with '/0'
    memset(file_List, '\0', sizeof(file_List));
    //initialized mutex and assigned lock as first parameter.
    pthread_mutex_init(&lock, NULL);
    //initialized cond and assigned cond as first parameter.
    pthread_cond_init(&cond, NULL);

    while (1)
    {
        //read myfifo and myfifo opened as read only
        file_read = open(myfifo, O_RDONLY);
        read(file_read, buf, MAX_BUF);
        //command is splited into words.
        command = splitIntoWords(buf);

        struct params params;
        //assigned command elements to params variables.
        params.arg1 = command[1];
        params.arg2 = command[2];
        //checked command's first element.
        if (strcmp(command[0], "init") == 0){
            count++;
            printf("%d. client is created\n", count);
        }
        //if first element equals to create, created pthread and called createfile function.
        else if (strcmp(command[0], "create") == 0)
        {
            pthread_create(&threads[0], NULL, createFile, &params);
            returnResponse = 1;
        }
        //if first element equals to delete, created pthread and called deletefile function.
        else if (strcmp(command[0], "delete") == 0)
        {
            pthread_create(&threads[1], NULL, deleteFile, &params);
            returnResponse = 1;
        }
        //if first element equals to write, created pthread and called writefile function.
        else if (strcmp(command[0], "write") == 0)
        {
            pthread_create(&threads[2], NULL, writeFile, &params);
            returnResponse = 1;
        }
        //if first element equals to read, created pthread and called readfile function.
        else if (strcmp(command[0], "read") == 0)
        {
            pthread_create(&threads[3], NULL, readFile, &params);
            returnResponse = 1;
        }
        //if first element equals to exit, program gave a response and exit that client.
        else if (strcmp(command[0], "exit") == 0)
        {
            printf("Exit\n");
            strcpy(response, "Program has finished\n");
            returnResponse = 1;
            count--;
            //if client number (count) = 0, exited file manager too.
            if(count==0){
                file_read = open(myfifo, O_WRONLY);
                write(file_read, response, sizeof(response));
                close(file_read);
                exit(0);
            } 
            //printf("%s\n", myArray[0]);
            printf("%d\n", count);
        }
        //waits till the threads are finished
        for (int i = 0; i < 4; i++)
        {
            pthread_join(threads[i], &status);
        }

        if(returnResponse == 1){
            file_read = open(myfifo, O_WRONLY);
            write(file_read, response, sizeof(response));
            close(file_read);
        }
        
    }
    //destroyed lock and cond variables.
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    exit(0);
}
//creates 2D matrix
char **generateMatrix(int columns, int rows)
{
    //creates array as dynamic in memory.
    char **matrix = malloc(rows * sizeof(char*));
    for (int i = 0; i < rows; i++)
    {
        matrix[i] = malloc(columns * sizeof(char));
    }

    return matrix;
}
//splits a input into words.
char **splitIntoWords(char *str)
{
    int i = 0;
    //splited according to  " " and assigned to token.
    char *token = strtok(str, " ");
    char **words;
    //generated 10,10 2D matris for words.
    words = generateMatrix(10, 10);
    // continues until token is not null
    while (token != NULL)
    {
        //assigned to words[i]
        *(words + i) = token;
        i++;
        //splited according to  " " and assigned to token.
        token = strtok(NULL, " ");
    }

    return words;
}

void *createFile(char *args)
{
    //locked thread.
    pthread_mutex_lock(&lock);
    struct params *params = args;
    //arg1 is assigned to file_name
    char *file_name = params->arg1;
    printf("filename : %s\n", file_name);
    //checks if file is created.
    int idx = -1;
    //if file_length[i] is not null, compares with i and if they are equal i assigned to idx.
    for (int i = 0; i < file_length; i++)
    {
        if (file_List[i] != NULL)
        {
            if (strcmp(file_List[i], file_name) == 0)
            {
                idx = i;
            }
        }
    }
    //if idx = -1, file has never been created. So file is created.
    if (idx == -1)
    {
        for (int i = 0; i < 10; i++)
        {
            if (file_List[i][0] == '\0')
            {
                //created a file empty index of file_list.
                strcpy(file_List[i], file_name);
                FILE *file = fopen(file_name, "w");
                fclose(file);
                strcpy(response, "File Created\n");

                break;
            }
        }
    }
    //if idx!=-1, gave a response message.
    else
    {
        strcpy(response, "File has already created.\n");
    }
    //unlocked the thread.
    pthread_mutex_unlock(&lock);
}

void *deleteFile(char *args)
{
    //locked thread.
    pthread_mutex_lock(&lock);
    struct params *params = args;
    //arg1 is assigned to file_name
    char *file_name = params->arg1;
    printf("filename : %s\n", file_name);
    //checks if file is created.
    int idx = -1;
    //if file_length[i] is not null, compares with i and if they are equal i assigned to idx.
    for (int i = 0; i < file_length; i++)
    {
        if (file_List[i] != NULL)
        {
            if (strcmp(file_List[i], file_name) == 0)
            {
                idx = i;
                break;
            }
        }
    }
    //if idx != -1, file has never been deleted. So file is deleted.
    if (idx != -1)
    {
        file_List[idx][0] = '\0';
        remove(file_name);
        strcpy(response, "File Deleted!\n");
    }
    //if idx=-1, gave a response message.
    else
    {
        strcpy(response, "File Cannot Find.\n");
    }
    //unlocked the thread.
    pthread_mutex_unlock(&lock);
}

void *readFile(char *args)
{
    pthread_mutex_lock(&lock);
    struct params *params = args;
    char *file_name = params->arg1;
    printf("filename : %s\n", file_name);
    int idx = -1;
    for (int i = 0; i < file_length; i++)
    {
        if (file_List[i] != NULL)
        {
            if (strcmp(file_List[i], file_name) == 0)
            {
                idx = i;
                break;
            }
        }
    }
    //if idx != -1, file is read.
    if (idx != -1)
    {
        FILE *fptr = fopen(file_name, "r");
        while ((c = fgetc(fptr)) != EOF)
        {
            printf("%c", c);
        }
        fclose(fptr);

        strcpy(response, "File Readed!\n");
    }
    else
    {
        strcpy(response, "File Cannot Find.\n");
    }
    pthread_mutex_unlock(&lock);
}

void *writeFile(char *args)
{
    pthread_mutex_lock(&lock);
    struct params *params = args;
    //arg1 is assigned to file_name
    char *file_name = params->arg1;
    //arg2 is assigned to data as text.
    char *data = params->arg2;
    printf("filename : %s\n", file_name);
    printf("data : %s\n", data);
    int idx = -1;
    for (int i = 0; i < 10; i++)
    {
        if (file_List[i] != NULL)
        {
            if (strcmp(file_List[i], file_name) == 0)
            {
                idx = i;
                break;
            }
        }
    }
    //printed the data.
    if (idx != -1)
    {

        FILE *file = fopen(file_name, "a+");
        if (file == NULL)
        {
            perror("fopen failed");
            // return;
        }

        fprintf(file, "%s\n", data);

        if (fclose(file) == EOF)
        {
            perror("fclose failed");
            // return;
        }
        strcpy(response, "File Writed!\n");
    }
    else
    {
        strcpy(response, "File Cannot Find.\n");
    }
    pthread_mutex_unlock(&lock);
}

