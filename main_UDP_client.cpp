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
using namespace std;

void add_Date(stringstream *stream){
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[25];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 25, "%a, %d %h %Y %H:%M:%S", timeinfo);
    char crrnt_Time[35];
    sprintf(crrnt_Time, "Date: %s GMT\n\n", buffer);
    *stream << crrnt_Time;
}

int main(int arg_Num, char *args[]) {//){//
    int clientSocket, rcvd_Data, server_port;
    int bufsize = 512;
    char buffer[bufsize];
    const char *server_host, *connection_type, *connection_order, *filename;// = malloc(bufsize);
    struct sockaddr_in srvr_Addr, clnt_Addr;
    struct hostent *hst_Name;
    socklen_t clnt_Len = sizeof(clnt_Addr);
    socklen_t srvr_Len = sizeof(srvr_Addr);

    server_host = args[1];//"149.160.202.17";//192.168.2.223";//149.160.200.10";//
    server_port = atoi(args[2]);//15010;//
    connection_type = args[3];//"non-persistant";//
    filename = args[4];//"text.txt";//HelloWorld.html";//

    if (strcmp(connection_type,"persistent") == 0) connection_order = "keep-alive";
    else if (strcmp(connection_type, "non-persistent") == 0) connection_order = "close";
    else{
        perror("Entered connection type is no valid!");
        exit(1);
    }

    string zeroTo255 = "(\\d{1,2}|(0|1)\\d{2}|2[0-4]\\d|25[0-5])";
    string compPattern = zeroTo255 + "\\." + zeroTo255 + "\\." + zeroTo255 + "\\." + zeroTo255;
    regex pattern(compPattern);//"(\\d{1,3})\.(\\d{1,3})");

    clock_t begin = clock();
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) > 0){
        printf("The socket was created\n");
    }

    srvr_Addr.sin_family = AF_INET;
    srvr_Addr.sin_addr.s_addr =inet_addr(server_host);
    srvr_Addr.sin_port = htons(server_port);

    printf("The Server is connected...\n");
    stringstream stream;
    string sending_Text;
    char message[60];
    sprintf(message, "GET /%s HTTP/1.1\n", filename);
    stream << message;
    sprintf(message, "Host: %s\n", server_host);
    stream << message;
    sprintf(message, "Connection:%s\n", connection_order);
    stream << message;
    add_Date(&stream);
    sending_Text = stream.str();
    //printf(sending_Text.c_str());
    sendto(clientSocket, sending_Text.c_str(), sending_Text.length(),0, (struct sockaddr *)&srvr_Addr, srvr_Len);

    bzero(buffer,bufsize);
    rcvd_Data = recvfrom(clientSocket, buffer, bufsize - 128, 0, (struct sockaddr *)&srvr_Addr, &srvr_Len);
    regex okay_Pttrn("HTTP/.+ 200 OK");
    regex notFnd_Pttrn("HTTP/.+ 404 Not Found");
    regex pat_connct_len("Content-length:.+\n");
    smatch m;
    if (rcvd_Data < 0) perror("ERROR reading from socket");
    //printf("Here is the server message:\n %s\n", buffer);
    string str(buffer);
    string connection_len;
    if (regex_search(str, m, okay_Pttrn)) {
        regex dblReturn_Pttrn("\\n\\n");
        if (regex_search(str, m, dblReturn_Pttrn))
            memmove (buffer, buffer + m.position(), strlen (buffer));// = buffer(m.position(),str.length());
        if (regex_search(str, m, pat_connct_len)) {
            connection_len = m.str();
            connection_len = connection_len.substr(16);
            connection_len.erase(remove(connection_len.begin(), connection_len.end(), '\n'), connection_len.end());
        } else connection_len = "1000";
        //printf("Here is the received file:\n");

        int rcvd_Len = 0;
         do{
             rcvd_Len += bufsize;//strlen(buffer);
             //printf("\nNumber received bytes so far : %i\n", rcvd_Len);
             //printf(buffer);
             //
        }while ((rcvd_Len < atoi(connection_len.c_str())) && (rcvd_Data = recvfrom(clientSocket, buffer , bufsize, 0, (struct sockaddr *)&srvr_Addr, &srvr_Len) >= 0));
        clock_t end = clock();
        printf("number of received bytes: %d\n", rcvd_Len);
        printf("The file was received and elapsed time(sec)is: %0.3f", double(end - begin) / CLOCKS_PER_SEC);
    }
    else if (regex_search(str, m, notFnd_Pttrn)) {
        printf("Server could not find the requested file!\n");
    }
    close(clientSocket);
    return 0;
}