#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include<regex>
#include <string>
#include<time.h>
#include<pthread.h>
using namespace std;

struct arg_struct{
    const char *str;
    int* skt;
    struct sockaddr_in* clnt_Addr ;
    socklen_t* clnt_Len;
};
void add_Date(stringstream* stream){
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[25];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 25, "%a, %d %h %Y %H:%M:%S", timeinfo);
    char crrnt_Time[35];
    sprintf(crrnt_Time, "Date: %s GMT\n", buffer);
    *stream << crrnt_Time;
}
void send_File(void *pnt_Args, const char* pntFilePath) {
    struct arg_struct *args =(struct arg_struct *) pnt_Args;
    int* skt_Connection =  args -> skt;// pnt_Socket;
    const char *connection_type = args -> str;
    struct sockaddr_in* clnt_Addr = args -> clnt_Addr ;
    socklen_t* clnt_Len = args -> clnt_Len;
    string sending_Text;
    stringstream stream;
    //stringstream& refStream = stream;
    if (strcmp(pntFilePath, "404") == 0) {
        stream << "HTTP/1.1 404 Not Found\n";
        char msg[60];
        sprintf(msg, "Connection: %s\n", connection_type);
        stream << msg;
        add_Date(&stream);
        stream << "Content-length: 39\n";
        //write(connectionSocket, "Server: pydb.py\n", 16);
        //write(connectionSocket, "Transfer-Encoding: chunked\n", 27);
        stream << "Content-Type: text/html;\n\n";
        stream << "<html><body>404 Not Found</body></html>";
        sending_Text = stream.str();
        sendto(*skt_Connection, sending_Text.c_str(), (int)sizeof(sending_Text), 0, (struct sockaddr *) clnt_Addr, *clnt_Len);
    }
    else{
        //sending header of the HTTP message
        stream << "HTTP/1.1 200 OK\n";
        char msg[60];
        sprintf(msg, "Connection: %s\n", connection_type);
        stream << msg;
        stream << "Content-length: ";
        FILE *sending_File = fopen(pntFilePath, "r");
        if (sending_File == NULL) return;

        fseek(sending_File, 0L, SEEK_END);
        stream << ftell(sending_File) << "\n";
        fseek(sending_File, 0L, SEEK_SET);
        add_Date(&stream);
        stream << "Content-Type: text/html\n\n";

        sending_Text = stream.str();
        sendto(*skt_Connection, sending_Text.c_str(), sending_Text.length(), 0, (struct sockaddr *) clnt_Addr, *clnt_Len);

        // sending the file
        while (feof(sending_File) == 0) {
            int read_Size;
            char read_Buffer[512];

            read_Size = fread(read_Buffer, sizeof(unsigned char), 512, sending_File);
            if (read_Size > 0) {
                char *ptr_Read_Buffer = read_Buffer;
                do {
                    fd_set fdSet;
                    timeval tm_Eval;

                    FD_ZERO(&fdSet);
                    FD_SET(*skt_Connection, &fdSet);

                    tm_Eval.tv_sec = 10;
                    tm_Eval.tv_usec = 0;

                    if (select(1 + *skt_Connection, NULL, &fdSet, NULL, &tm_Eval) > 0) {
                        int sent_Size;
                        sent_Size = sendto(*skt_Connection, ptr_Read_Buffer, read_Size, 0, (struct sockaddr *) clnt_Addr, *clnt_Len);
                        if (sent_Size == -1) return;
                        ptr_Read_Buffer += sent_Size;
                        read_Size -= sent_Size;
                    }
                } while (read_Size > 0);
            }
        }
        fclose(sending_File);
        printf("The requested file was transmitted!\n");
    }
}

void *crt_new_Connect(void *pnt_Args) {
    struct arg_struct *args = (struct arg_struct *) pnt_Args;
    int *serverSocket = args->skt;// pnt_Socket;
    const char *connection_type = args->str;
    struct sockaddr_in *clnt_Addr = args->clnt_Addr;
    socklen_t *clnt_Len = args->clnt_Len;
    int rcvd_Data;
    int bufsize = 1024;
    char buffer[bufsize];// = malloc(bufsize);

    bzero(buffer, bufsize);
    rcvd_Data = recvfrom(*serverSocket, buffer, bufsize, 0, (struct sockaddr *) clnt_Addr, clnt_Len);

    regex pattern("/.+ HTTP");
    smatch m;
    if (rcvd_Data < 0) perror("ERROR reading from socket");
    //printf("Here is the client message:\n %s\n", buffer);
    string str(buffer);
    if (regex_search(str, m, pattern)) {
        string file_Path = m.str();
        file_Path = file_Path.substr(1, file_Path.size() - 6);
        if (access(file_Path.c_str(), F_OK) != -1) {
            send_File(pnt_Args, file_Path.c_str());
        } else {
            const char *fndState = "404";
            send_File(pnt_Args, fndState);
        }
    } else {
        string text_message;
        stringstream stream;
        stream << "HTTP/1.1 200 OK\n";
        char msg[60];
        sprintf(msg, "Connection: %s\n", connection_type);
        stream << msg;
        stream << "Content-length: 54\n";
        stream << "Content-Type: text/html\n\n";
        stream << "<html><body>You can request a HTML page!</body></html>";
        text_message = stream.str();
        sendto(*serverSocket, text_message.c_str(), text_message.length(), 0, (struct sockaddr *) clnt_Addr, *clnt_Len);
    }
}
int main(int arg_Num, char *args[]) {//){//
    int serverSocket, connectionSocket;
    struct sockaddr_in srvr_Addr, clnt_Addr;
    const char *connection_type, *connection_order;
    socklen_t clnt_Len = sizeof(clnt_Addr);

    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) > 0){
        printf("The socket was created\n");
    }

    connection_type = "non-persistent";//args[2];//
    srvr_Addr.sin_family = AF_INET;
    srvr_Addr.sin_addr.s_addr = INADDR_ANY;
    srvr_Addr.sin_port = htons(atoi(args[1]));//"15010"));//
    if (strcmp(connection_type,"persistent") == 0) connection_order = "keep-alive";
    else if (strcmp(connection_type, "non-persistent") == 0) connection_order = "close";
    else{
        perror("Entered connection type is no valid!");
        exit(1);
    }

    if (bind(serverSocket, (struct sockaddr *) &srvr_Addr, sizeof(srvr_Addr)) == 0){
        printf("Binding Socket\n");
    }

    if (serverSocket < 0 ) {
        perror("server cannot connect to client");
        exit(1);
    }
    printf("\nNew Client connection...\n");

    struct arg_struct thrd_Args;
    thrd_Args.str = connection_order;
    thrd_Args.skt = &serverSocket;
    thrd_Args.clnt_Addr = &clnt_Addr;
    thrd_Args.clnt_Len = &clnt_Len;
    crt_new_Connect((void*) &thrd_Args);

    //Now join the thread , so that we dont terminate before the thread
    //pthread_join( thread_id , NULL);
    puts("Handler assigned");
    close(serverSocket);
    return 0;
}

