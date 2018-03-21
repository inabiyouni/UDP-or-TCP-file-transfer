#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include<sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include<regex>
#include <string>
#include<time.h>
#include<pthread.h>
#include <ctime>
using namespace std;

struct arg_struct{
    const char *str;
    int skt;
};
void add_Date(stringstream *stream){
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
void send_File(int* skt_Connection, const char* pntFilePath, const char *connection_type) {
    string sending_Text;
    stringstream stream;
    //stringstream& refStream = stream;
    if (strcmp(pntFilePath, "404") == 0) {
        stream << "HTTP/1.1 404 Not Found\n";
        char msg[60];
        sprintf(msg, "Connection:%s\n", connection_type);
        stream << msg;
        add_Date(&stream);
        stream << "Content-length: 39\n";
        //write(connectionSocket, "Server: pydb.py\n", 16);
        //write(connectionSocket, "Transfer-Encoding: chunked\n", 27);
        stream << "Content-Type: text/html;\n\n";
        stream << "<html><body>404 Not Found</body></html>";
        sending_Text = stream.str();
        send(*skt_Connection, sending_Text.c_str(), sending_Text.length(), 0);
    }
    else{
        //sending header of the HTTP message
        stream << "HTTP/1.1 200 OK\n";
        char msg[60];
        sprintf(msg, "Connection:%s\n", connection_type);
        stream << msg;
        stream << "Content-length: ";
        FILE *sending_File = fopen(pntFilePath, "r");
        if (sending_File == NULL) return;

        //fseek(sending_File, 0L, SEEK_END);
        struct stat st;
        stat(pntFilePath, &st);
        int size = st.st_size;
        stream << size<< "\n";  //ftell(sending_File)
        //fseek(sending_File, 0L, SEEK_SET);
        add_Date(&stream);
        stream << "Content-Type: text/html\n\n";

        sending_Text = stream.str();
        send(*skt_Connection, sending_Text.c_str(), sending_Text.length(), 0);

        // sending the file
        while (feof(sending_File) == 0) {
            int read_Size;
            int bugger_Size = 512;
            char read_Buffer[bugger_Size];

            read_Size = fread(read_Buffer, sizeof(unsigned char), bugger_Size, sending_File);
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
                        sent_Size = send(*skt_Connection, ptr_Read_Buffer, read_Size, 0);
                        if (sent_Size == -1) return;
                        ptr_Read_Buffer += sent_Size;
                        read_Size -= sent_Size;
                    }
                } while (read_Size > 0);
            }
        }
        fclose(sending_File);
    }
}

void *crt_new_Connect(void *pnt_Args) {
    struct arg_struct *args =(struct arg_struct *) pnt_Args;
    int socket =  args -> skt;// pnt_Socket;
    string connection_type;//const char *connection_type = args -> str;
    int read_size, rcvd_Data;
    int bufsize = 1024;
    char buffer[bufsize];// = malloc(bufsize);
    int sending_Cntr = 0;
    do {
        bzero(buffer, 256);
        rcvd_Data = recv(socket, buffer, bufsize, 0);
        regex pattern("/.+ HTTP");
        regex pat_connct_type("Connection:.+\n");
        smatch m;
        if (rcvd_Data < 0) perror("ERROR reading from socket");
        //printf("Here is the client message:\n %s\n", buffer);
        string str(buffer);
        if (regex_search(str, m, pat_connct_type)) {
            connection_type = m.str();
            int len = connection_type.size();
            connection_type = connection_type.substr(11);
            connection_type.erase(remove(connection_type.begin(), connection_type.end(), '\n'), connection_type.end());
        } else connection_type = "close";
        if (regex_search(str, m, pattern)) {
            string file_Path = m.str();
            file_Path = file_Path.substr(1, file_Path.size() - 6);
            if (access(file_Path.c_str(), F_OK) != -1) {
                sending_Cntr++;
                send_File(&socket, file_Path.c_str(), connection_type.c_str());
            } else {
                const char *fndState = "404";
                send_File(&socket, fndState, connection_type.c_str());
            }
        } else {
            string text_message;
            stringstream stream;
            stream << "HTTP/1.1 200 OK\n";
            char msg[60];
            sprintf(msg, "Connection:%s\n", connection_type);
            stream << msg;
            stream << "Content-length: 54\n";
            stream << "Content-Type: text/html\n\n";
            stream << "<html><body>You can request a HTML page!</body></html>";
            text_message = stream.str();
            send(socket, text_message.c_str(), text_message.length(), 0);
        }
        if (connection_type.compare("keep-alive") == 0) printf("\nServer left the TCP Connection open!\n");
    } while(connection_type.compare("keep-alive") == 0);
    printf("\nServer closed the TCP Connection.\n");
    close(socket);
}

int main(int arg_Num, char *args[]) {//){//){//){//
    int serverSocket, connectionSocket;
    struct sockaddr_in srvr_Addr, clnt_Addr;
    const char *connection_type;
    socklen_t clnt_Len = sizeof(clnt_Addr);

    while (true){
        if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) > 0) {
            printf("The socket was created\n");
        }

        srvr_Addr.sin_family = AF_INET;
        srvr_Addr.sin_addr.s_addr = INADDR_ANY;
        srvr_Addr.sin_port = htons(atoi(args[1]));//"15010"));//

        if (bind(serverSocket, (struct sockaddr *) &srvr_Addr, sizeof(srvr_Addr)) == 0) {
            printf("Binding Socket\n");
        }

        if (listen(serverSocket, 10) < 0) {
            perror("server is listening");
            exit(1);
        }

        pthread_t thread_id;

        connectionSocket = accept(serverSocket, (struct sockaddr *) &clnt_Addr, &clnt_Len);
        if (connectionSocket < 0) {
            perror("server cannot connect to client");
            exit(1);
        }
        printf("\nClient connection...\n");

        struct arg_struct thrd_Args;
        thrd_Args.str = connection_type;
        thrd_Args.skt = connectionSocket;
        crt_new_Connect((void *) &thrd_Args);
        close(serverSocket);
        close(connectionSocket);
    }

    return 0;
}

