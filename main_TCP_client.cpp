#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include<arpa/inet.h>
#include<regex>
#include <netdb.h>
#include <ctime>
#include <unistd.h>
using namespace std;

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

int main(int arg_Num, char *args[]) {//){//
    int clientSocket, rcvd_Data, server_port, no_files;
    int bufsize = 256 + 128;
    int totalRcvdB;
    char buffer[bufsize];
    const char *server_host, *connection_type, *connection_order, *filename;// = malloc(bufsize);
    struct sockaddr_in srvr_Addr, clnt_Addr;
    struct hostent *hst_Name;
    socklen_t clnt_Len = sizeof(clnt_Addr);

    server_host = args[1];//"192.168.2.223";//
    server_port = atoi(args[2]);//15010;//
    connection_type = args[3];//"non-persistant";//
    filename = args[4];//"text.txt";//HelloWorld.html";//
    no_files = atoi(args[5]);

    //server_host = "149.160.202.17";//192.168.2.223";//args[1];//
    //server_port = 15010;//atoi(args[2]);//
    //connection_type = "persistent";//args[3];//
    //filename = "text_1MB.txt";//HelloWorld.html";//args[4];//
    //no_files = 10;//atoi(args[5]);

    if (strcmp(connection_type,"persistent") == 0) connection_order = "keep-alive";
    else if (strcmp(connection_type, "non-persistent") == 0) connection_order = "close";
    else{
        perror("Entered connection type is no valid!");
        exit(1);
    }
    stringstream stream;
    string sending_Text;
    char message[60];
    int rcving_Cntr = 0;
    clock_t begin = clock();
    //non-Persistent
    do {
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) > 0) {
            printf("The socket was created\n");
        }

        srvr_Addr.sin_family = AF_INET;
        srvr_Addr.sin_addr.s_addr = inet_addr(server_host);
        srvr_Addr.sin_port = htons(server_port);

        if (connect(clientSocket, (struct sockaddr *) &srvr_Addr, sizeof(srvr_Addr)) < 0) {
            perror("Cannot connect to the server!");
            exit(1);
        }

        printf("The Server is connected...\n");
        //Persistent
        do {
            totalRcvdB = 0;
            sprintf(message, "GET /%s HTTP/1.1\n", filename);
            stream << message;
            sprintf(message, "Host: %s\n", server_host);
            stream << message;
            sprintf(message, "Connection:%s\n\n", connection_order);
            stream << message;
            add_Date(&stream);
            sending_Text = stream.str();
            //printf(sending_Text.c_str());
            write(clientSocket, sending_Text.c_str(), sending_Text.length());
            bzero(buffer, bufsize);
            rcvd_Data = recv(clientSocket, buffer, bufsize, 0);
            regex okay_Pttrn("HTTP/.+ 200 OK");
            regex notFnd_Pttrn("HTTP/.+ 404 Not Found");
            regex pat_connct_len("Content-length:.+\n");
            smatch m;
            if (rcvd_Data < 0) perror("ERROR reading from socket");
            //printf("Here is the server message:\n %s\n\n", buffer);
            string str(buffer);
            string connection_len;
            if (regex_search(str, m, okay_Pttrn)) {
                rcving_Cntr++;
                if (regex_search(str, m, pat_connct_len)) {
                    connection_len = m.str();
                    connection_len = connection_len.substr(16);
                    connection_len.erase(remove(connection_len.begin(), connection_len.end(), '\n'),
                                         connection_len.end());
                } else connection_len = "1000";
                bzero(buffer, bufsize);
                //printf("Here is the received file:\n");
                while ((totalRcvdB < atoi(connection_len.c_str())) &&
                       (recv(clientSocket, buffer, bufsize - 128, 0) > 0)) {
                    totalRcvdB += bufsize - 128;//strlen(buffer);
                    //cout << buffer;
                }
            } else if (regex_search(str, m, notFnd_Pttrn)) {
                printf("Server could not find the requested file!\n");
            }
            if (strcmp(connection_order, "keep-alive") == 0) printf("\nClient left the TCP connection open.\n");
            else {
                printf("\nClient closed the TCP connection.\n");
                close(clientSocket);
            }
            printf("number of file: %d Bytes received: %d\n", rcving_Cntr, totalRcvdB);
        } while (strcmp(connection_order, "keep-alive") == 0 & rcving_Cntr < no_files);
        //printf("Sleeping for 100 msec to make sure server is running again!");
        //usleep(500000);
    }while (strcmp(connection_order, "close") == 0 & rcving_Cntr < no_files);
    clock_t end = clock();
    printf("Files recieved and elapsed time(sec) is: %0.3f\n", double(end - begin) / CLOCKS_PER_SEC);
    return 0;
}