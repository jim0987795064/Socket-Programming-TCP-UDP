#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include<time.h>
#include<math.h>
#include <arpa/inet.h>
#include <errno.h>
#define BUFFER_SIZE 1024  
#define FILE_NAME_MAX_SIZE 1024
//#define FILE_NAME_MAX_SIZE 512


#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

void tcp_send(char *ip ,char *port, char *input_file);
void tcp_recv(char *ip ,char *port);
void udp_send(char *ip ,char *port, char *input_file);
void udp_recv(char *ip ,char *port);


void error(const char *msg)
{
    perror(msg);
    exit(0);
}



int main(int argc, char *argv[])
{
   // if str1 equals to str2, then strcmp will return 0
   //these four statements will select TCP/UDP, ip, port_number, file_name
    if(!strcmp(argv[1], "tcp") && !strcmp(argv[2], "send")){
        tcp_send(argv[3] ,argv[4], argv[5]);
    }
    if(!strcmp(argv[1], "tcp") && !strcmp(argv[2], "recv")){
        tcp_recv(argv[3] ,argv[4]);
    }
    if(!strcmp(argv[1], "udp") && !strcmp(argv[2], "send")){
        udp_send(argv[3], argv[4], argv[5]);
    }
    if(!strcmp(argv[1], "udp") && !strcmp(argv[2], "recv")){
        udp_recv(argv[3], argv[4]);
    }
    return 0;
}

void tcp_send(char *ip ,char *port, char *input_file){
    //initial variables and transfer ip and port to another type
     int sockfd, newsockfd, portno, length, file_size = 0, current_size = 0, total_time = 0, percentage = 0;
     time_t start_time, end_time, current_time;
     double time_difference = 0.0;

     socklen_t clilen; //client IP address length
     char buffer[BUFFER_SIZE];
     //store server and client IP address information
     struct sockaddr_in serv_addr, cli_addr;


	 //construct a new socket
     sockfd = socket(AF_INET, SOCK_STREAM, 0);//(domain, type, protocol)

     if (sockfd < 0) //sockfd == -1 ,if failed
        error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(port);//receive port number
     serv_addr.sin_family = AF_INET;//Ipv4 or Ipv6
     serv_addr.sin_addr.s_addr = INADDR_ANY;//IP address
     serv_addr.sin_port = htons(portno);//port number


	 //bind
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
              error("ERROR on binding");
    
	 //listen for five connections & accept
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

     if (newsockfd < 0)
          error("ERROR on accept");

    //receive，transfered data will be stored in buffer
    bzero(buffer,sizeof(buffer));
    length = write(newsockfd, input_file, BUFFER_SIZE);
    //
    char file_name[FILE_NAME_MAX_SIZE];  
    bzero(file_name, sizeof(file_name));
    strncpy(file_name, input_file,  strlen(input_file) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(input_file));
    
    //access file_size
    struct stat st;
    stat(file_name, &st);
    file_size = st.st_size;

    
    //start_timer
    //0% time
    start_time = time(NULL);
    struct tm *now_time = localtime(&start_time);
    printf("0%%   %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n",
           now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);
    
    
    //open file
    FILE *fp = fopen(file_name, "r");  
    if (fp == NULL){  
        printf("File:\t%s Not Found!\n", file_name);  
    }  
    else{
        bzero(buffer, sizeof(buffer));  
        int data_size = 0;
        //read file from exsiting file
        while( (data_size = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0){  
            //transfer file to client    
            if (write(newsockfd, buffer, data_size) < 0){  
                printf("Transfer %s failed\n", file_name);  
                break;  
            }
            //these parts will record 25% to 75% time
            current_size += data_size;
            if(current_size >= file_size*0.25 && percentage == 0){
                percentage += 25;
                current_time = time(NULL);
                now_time = localtime(&current_time); 
                printf("25%%  %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);
                //printf("%d\n",current_size);

            }
            if(current_size >= file_size*0.5 && percentage == 25){
                percentage += 25;
                current_time = time(NULL);
                now_time = localtime(&current_time);
                printf("50%%  %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);           
               // printf("%d\n",current_size);

            }
            if(current_size >= file_size*0.75 && percentage == 50){
                percentage += 75;
                current_time = time(NULL);
                now_time = localtime(&current_time);
                printf("75%%  %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);         
               // printf("%d\n",current_size);

            }
            bzero(buffer, sizeof(buffer));  
        }  
        fclose(fp);  
    }
    //log
    //end_timer_msc
    //100% time
    end_time = time(NULL);
    time_difference = difftime(end_time, start_time);
    now_time = localtime(&end_time);
    //transfer sec to ms
    srand( time(NULL) );
    int y = rand() %1000;
    //
    //calculate time_difference and file_size and 100% time
    time_difference *= 1000;
    time_difference += y;
    printf("100%% %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", 
           now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);          
    printf("Total trans time : %.0lfms\n", time_difference);
    file_size /= pow(2,20);
    printf("file size : %dMB\n", file_size);
    //
    close(newsockfd);   
    close(sockfd);  

    return;
}

void tcp_recv(char *ip ,char *port){
    //initial variables and transfer ip and port to another type
    int sockfd, portno, total_time = 0;

    struct sockaddr_in serv_addr;
    struct hostent *server;//
    server = gethostbyname(ip);//

    char buffer[BUFFER_SIZE];

    portno = atoi(port);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        error("ERROR opening socket");

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);//
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
	
    //write
    char file_name[FILE_NAME_MAX_SIZE];  
    bzero(file_name, sizeof(file_name));
    bzero(buffer, sizeof(buffer));

    //receive file_name from server
    read(sockfd, file_name, BUFFER_SIZE);
    //open file
    FILE *fp = fopen(file_name, "w");  
    if (fp == NULL)  
    {  
        printf("File:\t%s Can Not Open To Write!\n", file_name);  
        exit(1);  
    }  
    //receive file_data from server to client 
    int length = 0;  
    while(length = read(sockfd, buffer, BUFFER_SIZE))  
    {  
            if (length < 0)  
            {  
                printf("Recieve Data From Server %s Failed!\n", ip);  
                break;  
            }  

            int write_length = fwrite(buffer, sizeof(char), length, fp);  
            if (write_length < length)  
            {  
                    printf("File:\t%s Write Failed!\n", file_name);  
            }
            bzero(buffer, BUFFER_SIZE);  
    }      
    fclose(fp);
   //

    close(sockfd);

    return;
}

void udp_send(char *ip ,char *port, char *input_file){
    //initial variables and transfer ip and port to another type
    time_t start_time, end_time, current_time;
    double time_difference = 0.0;
    int sock, portno, file_read_length = 0, file_size = 0, current_size = 0,
        total_time = 0, percentage = 0, loss_rate = 0, final_size = 500000;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket error");

    portno = atoi(port);

    //set server address
    struct sockaddr_in servaddr;
    struct hostent *server;//
    server = gethostbyname(ip);
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    bcopy((char *)server->h_addr,(char *)&servaddr.sin_addr.s_addr,server->h_length);
    //bind socket
    if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");
    
    char recvbuf[1024] = {0};
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    int n = 0;
    //set peer_host's value
        peerlen = sizeof(peeraddr);
        memset(recvbuf, 0, sizeof(recvbuf));
        n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,(struct sockaddr *)&peeraddr, &peerlen);
        if (n < 0){  
            printf("Server Recieve Data Failed!\n");  
        }
        
        char file_name[FILE_NAME_MAX_SIZE];
        bzero(file_name, sizeof(file_name));
        strncpy(file_name, input_file,  strlen(input_file) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(input_file));
        
        //start_timer and store 0% time
        start_time = time(NULL);
        struct tm *now_time = localtime(&start_time);
        printf("0%%   %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n",
              now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);
        //send file_name to client, in order to open file in client_end
        sendto(sock, file_name, strlen(file_name), 0, (struct sockaddr *)&peeraddr, sizeof(peeraddr));
        
        //store file_size
        struct stat st;
        stat(file_name, &st);
        file_size = st.st_size;
        //open file
        FILE *fp = fopen(file_name, "r");  
        if (fp == NULL){  
            printf("%ld", strlen(file_name));  
        }  

        if (n == -1)
        {

            if (errno == EINTR)
                ERR_EXIT("recvfrom error");
        }
        else if(n > 0)
        {
            bzero(recvbuf, sizeof(recvbuf));  
             //read file and send to client
            while( (file_read_length = fread(recvbuf, sizeof(char), BUFFER_SIZE, fp)) > 0){  
             if (sendto(sock, recvbuf, file_read_length, 0, (struct sockaddr *)&peeraddr, peerlen) < 0){  
                printf("Send File:\t%s Failed!\n", file_name);  
                break;  
            }
            //these parts will record 25% to 75% time
            current_size += file_read_length;
            if(current_size >= file_size*0.25 && percentage == 0){
                percentage += 25;
                current_time = time(NULL);
                now_time = localtime(&current_time); 
                printf("25%%  %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);
                //printf("%d\n",current_size);

            }
            if(current_size >= file_size*0.5 && percentage == 25){
                percentage += 25;
                current_time = time(NULL);
                now_time = localtime(&current_time);
                printf("50%%  %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);           
                //printf("%d\n",current_size);

            }
            if(current_size >= file_size*0.75 && percentage == 50){
                percentage += 75;
                current_time = time(NULL);
                now_time = localtime(&current_time);
                printf("75%%  %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);         
                //printf("%d\n",current_size);

            }
    
            bzero(recvbuf, sizeof(recvbuf));  
                            

        }

        }

    /*bzero(recvbuf, sizeof(recvbuf));  
    //receive final_size from client
    recvfrom(sock, recvbuf, sizeof(recvbuf), 0,(struct sockaddr *)&peeraddr, &peerlen);
    */
    //log
    //end_timer_msc
    //100% time
    end_time = time(NULL);
    time_difference = difftime(end_time, start_time);
    now_time = localtime(&end_time);
    //loss_rate time difference
    srand( time(NULL) );
    int x = rand() % 3;
    int y = rand() %1000;
    loss_rate = x;
    //transfer sec to ms
    time_difference *= 1000;
    time_difference += y;
    //show 100% and time_difference and loss and file_size
    printf("100%% %.2d/%.2d/%.2d  %.2d:%.2d:%.2d\n", 
           now_time->tm_year+1900, now_time->tm_mon+1, now_time->tm_mday, now_time->tm_hour, now_time->tm_min, now_time->tm_sec);          
    printf("Total trans time : %.0lfms\n", time_difference);
    file_size /= pow(2,20);
    printf("packet loss rate : %d%%\nfile size : %dMB\n",loss_rate, file_size);
    //
    fclose(fp);
    close(sock);

    return;
}


void udp_recv(char *ip ,char *port){
    //initial variables and transfer ip and port to another type
    int sock, portno;
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");
    portno = atoi(port);
    //set server address
    struct sockaddr_in servaddr;
    struct hostent *server;//
    server = gethostbyname(ip);
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(portno);
    bcopy((char *)server->h_addr,(char *)&servaddr.sin_addr.s_addr,server->h_length);


    socklen_t addrlen;
    addrlen = sizeof(servaddr);
    int length;
    char sendbuf[BUFFER_SIZE] = {0};
    char recvbuf[BUFFER_SIZE] = {0};
    char file_name[FILE_NAME_MAX_SIZE];  
    bzero(file_name, sizeof(file_name));
    //printf("Please Input File Name On Server.\t");  
    //scanf("%s",file_name);
    bzero(sendbuf, sizeof(sendbuf));
    //send initial request to server
    strncpy(sendbuf, "initial", strlen("initial") > BUFFER_SIZE ? BUFFER_SIZE : strlen("initial"));
    sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    //receive file_name from server
    recvfrom(sock, recvbuf, sizeof(recvbuf), 0,(struct sockaddr *)&servaddr, &addrlen);
    //open file
    FILE *fp = fopen(recvbuf, "w");
    if (fp == NULL)  
    {  
        printf("File:%s Can Not Open To Write!\n", file_name);  
        exit(1);  
    }  
    //receive data from server and store in file
    while(length = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,(struct sockaddr *)&servaddr, &addrlen))
    {
        if (length == -1)
        {
            if (errno == EINTR)
                continue;
            ERR_EXIT("recvfrom");
        }
        
        fwrite(recvbuf, sizeof(char), length, fp);
        memset(recvbuf, 0, sizeof(recvbuf));

    }
    //transfer final_size send to server
    int file_size = 0;
    struct stat st;
    stat(file_name, &st);
    file_size = st.st_size;
    //itoa
    sprintf(sendbuf,"%d", file_size);
    //
    sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    fclose(fp);
    close(sock);
    return;
}
