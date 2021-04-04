#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <arpa/inet.h>
#include <errno.h>

#define FILE_NAME_MAX_SIZE  512  

void error(const char *msg) {
    perror(msg);
    exit(1);
}


int main(int argc, char const *argv[])
{
	if(argc<5||argc>6) {
		fprintf(stderr,"Error format of input, please check the input.\n");
		exit(1);
	}

	if(strcmp(argv[1], "tcp")==0) {

		if(strcmp(argv[2], "send")==0) {
			int sockfd, newsockfd, portno;
		    socklen_t clilen;
		    char buffer[256];
		    struct sockaddr_in serv_addr, cli_addr;
		    int n;
		    
		    /*
		     * 建立socket
		     * AF_INET => internet IPv4協議
		     * SOCK_STREAM => TCP , SOCK_DGRAM => UDP
		     */
		    sockfd = socket(AF_INET, SOCK_STREAM, 0);

		    if (sockfd < 0) {
		       error("ERROR opening socket");
		    }

		    bzero((char *) &serv_addr, sizeof(serv_addr));
		    portno = atoi(argv[4]);

		    /*
		     * sin_family：表示該 Socket 的通訊協定家族系列，可選擇：AF_UNIX（Unix 作業系統）、AF_INET（Internet 網路）..etc
		     * sin_port：表示該 Socket 所連接的埠口位址，如通訊家族為 AF_INET，則表示 TCP/UDP 埠口號碼
		     * sin_addr：表示該 Socket 所連接之 IP 位址
		     */
		    serv_addr.sin_family = AF_INET;
		    serv_addr.sin_addr.s_addr = INADDR_ANY;
		    serv_addr.sin_port = htons(portno);

		    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		    	error("ERROR on binding");
		    }
		    /*
		     * 透過listen()進入被動監聽狀態
		     */
		    listen(sockfd,20);

		    //while(1) {

		    	clilen = sizeof(cli_addr);
		    	/*
		    	 * 接受一個從client端到達server端的連線請求，將客戶端的資訊儲存在client_addr中
		    	 * 如果沒有連線請求，則一直等待直到有連線請求為止
		    	 * accpet返回一個新的socket，這個socket用來與此次連線到server的client進行通訊 
		    	 * newsockfd代表了這個通訊通道
		    	 */
			    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			    if (newsockfd < 0) {
			        error("ERROR on accept");
			    }

			    /** 紀錄file檔名*/
			    char file_name[FILE_NAME_MAX_SIZE + 1];
			    bzero(file_name, sizeof(file_name));

			    if(strlen(argv[5]) <= FILE_NAME_MAX_SIZE) {
                    strncpy(file_name, argv[5],strlen(argv[5]));
                } else {
                    error("Name of file is too long cause error");
                }

                /** 傳送檔名給client */
                send(newsockfd, file_name, strlen(file_name), 0);

			    FILE *fp = fopen(file_name, "r");
			    if(fp==NULL) {
			    	error("ERROR openning file.");
			    } else {
			    	/** 尋找檔案大小並且傳送至client */
			    	int fileNum;
			    	fseek(fp, 0, SEEK_END);
			    	fileNum = ftell(fp);
			    	send(newsockfd, (void *)&fileNum, sizeof(fileNum), 0);
			    	fseek(fp, 0, SEEK_SET);

			    	bzero(buffer,256);
			    	int file_block_length = 0;  
			    	while( (file_block_length = fread(buffer, sizeof(char), 256, fp)) > 0) {
			    		// printf("file_block_length = %d\n", file_block_length);  
			    		if (send(newsockfd, buffer, file_block_length, 0) < 0) {  
		                    printf("Send File:\t%s Failed!\n", file_name);  
		                    break;  
		                } 
		                bzero(buffer, sizeof(buffer));   
			    	}
			    	fclose(fp);
			    	printf("File:\t%s Transfer Finished!\n", file_name);  
			    }
			    close(newsockfd);
		    //}

		    // printf("Here is the message: %s\n",buffer);
		    // n = write(newsockfd,"I got your message",18);
		    // if (n < 0) error("ERROR writing to socket");

		    close(sockfd);
		    return 0;

		} else if(strcmp(argv[2], "recv")==0) {
			int sockfd, portno, n;
		    struct sockaddr_in serv_addr;
		    struct hostent *server;

		    char buffer[256];
		    
		    portno = atoi(argv[4]);
		    sockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if (sockfd < 0) 
		        error("ERROR opening socket");
		    server = gethostbyname(argv[3]);
		    if (server == NULL) {
		        fprintf(stderr,"ERROR, no such host\n");
		        exit(0);
		    }
		    bzero((char *) &serv_addr, sizeof(serv_addr));
		    serv_addr.sin_family = AF_INET;
		    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
		    serv_addr.sin_port = htons(portno);

		    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		        error("ERROR connecting");

		    char file_name[FILE_NAME_MAX_SIZE + 1];  
    		bzero(file_name, sizeof(file_name));
    		bzero(buffer, sizeof(buffer));
    		/** 接收server傳送檔案名稱 */
    		int message_length;
    		if((message_length = recv(sockfd, buffer, 256, 0))) {
    			strncpy(file_name, buffer, strlen(buffer));
                file_name[strlen(buffer)] = '\0';
    		}

    		/** 接收檔案大小資訊 */
    		int fileNum;
    		double fileSize;
    		recv(sockfd, (void *) &fileNum, sizeof(fileNum), 0);
    		//printf("File Byte %d Bytes\n",fileNum);
            fileSize = fileNum/(1024*1024);
            //printf("File size %f MB\n",fileSize);

    		FILE *fp = fopen(file_name, "w");  
		    if (fp==NULL) {  
		        error("ERROR openning file.");  
		    }  

		    bzero(buffer, sizeof(buffer));  
		    n=0;
		    clock_t t1, t2;
		    t1 = clock();

		    int total_length = 0;
		    int index = 0;
		    time_t now;
		    struct tm * timeinfo;
		    while((n=recv(sockfd, buffer, 256, 0))) {  
		        if (n < 0) {  
		            printf("Recieve Data From Server Failed!\n");  
		            break;  
		        }  
		  
		        int write_length = fwrite(buffer, sizeof(char), n, fp);  
		        if (write_length < n) {  
		            printf("File:\t%s Write Failed!\n", file_name);  
		            break;  
		        }  
		        bzero(buffer, 256);

		        /** 計算傳送進度 */
		        total_length += n;
		        if(total_length >= (fileNum/4)*index) {
		        	time (&now);
					timeinfo = localtime (&now);
                    printf("%d%%",index*25);
                    printf(" %d/%d/%d %02d:%02d:%02d\n", timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                    index++;
		        }
		    }  
		    t2 = clock();
		    //printf("Finished.\n");
		    printf("Total trans time: %lf ms\n", ((t2-t1)/(double)(CLOCKS_PER_SEC))*1000);
		    printf("File size %f MB\n",fileSize);
		    fclose(fp);
		    close(sockfd);
		    return 0;
		} else {
			fprintf(stderr,"Error, the second input must be send or recv.\n");
			exit(1);
		}
	} else if(strcmp(argv[1], "udp")==0) {

		if(strcmp(argv[2], "send")==0) {

			int sock;
    		if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        		error("ERROR opening socket");
    		}

    		int portno = atoi(argv[4]);
    		struct sockaddr_in serv_addr;
    		bzero((char *) &serv_addr, sizeof(serv_addr));
		    serv_addr.sin_family = AF_INET;
		    serv_addr.sin_port = htons(portno);
		    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		    if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        		error("ERROR on binding");
		    }

		    //while(1) {
		    	char buffer[256];
			    bzero(buffer,256);
			    struct sockaddr_in peeraddr;
			    socklen_t peerlen;
			    peerlen = sizeof(peeraddr);

			    /** 等待取得client訊息 */
			    int n;
			    n = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, &peerlen);

			    /** 紀錄file檔名 */
				char file_name[FILE_NAME_MAX_SIZE + 1];
				bzero(file_name, sizeof(file_name));

				if(strlen(argv[5]) <= FILE_NAME_MAX_SIZE) {
	                strncpy(file_name, argv[5],strlen(argv[5]));
	            } else {
	                error("Name of file is too long cause error");
	            }
	            // 傳送檔名給client
	            sendto(sock, file_name, sizeof(file_name), 0, (struct sockaddr *)&peeraddr, peerlen);

	            FILE *fp = fopen(file_name, "r");
	            if(fp==NULL) {
			    	error("ERROR openning file.");
			    } else {
			    	/** 尋找檔案大小並且傳送至client */
			    	int fileNum;
			    	fseek(fp, 0, SEEK_END);
			    	fileNum = ftell(fp);
			    	sendto(sock, (void *)&fileNum, sizeof(fileNum), 0, (struct sockaddr *)&peeraddr, peerlen);
			    	fseek(fp, 0, SEEK_SET);

			    	bzero(buffer,256);
			    	int file_block_length = 0;  
			    	while( (file_block_length = fread(buffer, sizeof(char), 256, fp)) > 0) {
			    		// printf("file_block_length = %d\n", file_block_length);  
			    		if (sendto(sock, buffer, file_block_length, 0, (struct sockaddr *)&peeraddr, peerlen) < 0) {  
		                    printf("Send File:\t%s Failed!\n", file_name);  
		                    break;  
		                } 
		                bzero(buffer, sizeof(buffer));   
			    	}
			    	strncpy(buffer, "UDP_file_send_ended", 19);
			    	
			    	for(int i=0;i<20;i++) {
			    		sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&peeraddr, peerlen);
			    	}
			    	
			    	fclose(fp);
			    	printf("File:\t%s Transfer Finished!\n", file_name); 
			    }

			    close(sock);
		    //}
            return 0;
		} else if(strcmp(argv[2], "recv")==0) {
			int sock;
			if((sock = socket(PF_INET, SOCK_DGRAM, 0))<0) {
        		error("ERROR opening socket");
			}

			int portno = atoi(argv[4]);
			struct hostent *server;
			server = gethostbyname(argv[3]);
		    if (server == NULL) {
		        fprintf(stderr,"ERROR, no such host\n");
		        exit(0);
		    }
			struct sockaddr_in serv_addr;
		    memset(&serv_addr, 0, sizeof(serv_addr));

		    serv_addr.sin_family = AF_INET;
		    serv_addr.sin_port = htons(portno);
		    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

		    char buffer[256];
		    bzero(buffer, 256);

		    char file_name[FILE_NAME_MAX_SIZE + 1];  
    		bzero(file_name, sizeof(file_name));
    		bzero(buffer, sizeof(buffer));

    		/** 傳送client訊息給server */
    		sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    		/** 接收server傳送檔案名稱 */
    		int message_length;
    		if((message_length = recvfrom(sock, buffer, 256, 0, NULL, NULL))) {
    			strncpy(file_name, buffer, strlen(buffer));
                file_name[strlen(buffer)] = '\0';
    		}

    		/** 接收檔案大小資訊 */
    		int fileNum;
    		double fileSize;
    		recvfrom(sock, (void *) &fileNum, sizeof(fileNum), 0, NULL, NULL);
    		//printf("File Byte %d Bytes\n",fileNum);
            fileSize = fileNum/(1024*1024);
            //printf("File size %f MB\n",fileSize);

            FILE *fp = fopen(file_name, "w");  
		    if (fp==NULL) {  
		        error("ERROR openning file.");  
		    }

		    bzero(buffer, sizeof(buffer));    

            int n=0;
            clock_t t1, t2;
		    t1 = clock();

		    int total_length = 0;
		    int index = 0;
		    time_t now;
		    struct tm * timeinfo;
            while(1) {  
            	n=recvfrom(sock, buffer, 256, 0, NULL, NULL);
            	//printf("%d\n", n);
            	//printf("%s\n", buffer);
		        if(n < 0) {  
		            printf("Recieve Data From Server Failed!\n");  
		            break;  
		        }
		        /** 判斷檔案結尾的特殊string，收到後會判斷檔案已傳送完畢 */
		        if(strncmp(buffer, "UDP_file_send_ended", 19)==0) {
		        	printf("%d%%",100);
                    printf(" %d/%d/%d %02d:%02d:%02d\n", timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
		        	break;
		        }
		  
		        int write_length = fwrite(buffer, sizeof(char), n, fp);  
		        if (write_length < n) {  
		            printf("File:\t%s Write Failed!\n", file_name);  
		            break;  
		        }  
		        bzero(buffer, 256);

		        /** 計算傳送進度 */
		        total_length += n;
		        if(total_length >= (fileNum/4)*index) {
		        	time (&now);
					timeinfo = localtime (&now);
                    printf("%d%%",index*25);
                    printf(" %d/%d/%d %02d:%02d:%02d\n", timeinfo->tm_year+1900, timeinfo->tm_mon+1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
                    index++;
		        }
		    }
		    t2 = clock();
		    printf("Total trans time: %lf ms\n", ((t2-t1)/(double)(CLOCKS_PER_SEC))*1000);
		    /** 計算收到檔案大小 */
		    int recvFileNum;
		    double recvFileSize;
			fseek(fp, 0, SEEK_END);
			recvFileNum = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			recvFileSize = recvFileNum/(1024*1024);
			//printf("File size %f MB\n",recvFileSize);
			//printf("%d\n", fileNum);
			//printf("%d\n", recvFileNum);
			double result = (double)(fileNum-recvFileNum)/fileNum;

			printf("File size %f MB\n",fileSize);
			printf("packet lost rate: %f%%\n", result*100);

    		fclose(fp);
		    close(sock);
		    return 0;
		} else {
			fprintf(stderr,"Error, the second input must be send or recv.\n");
			exit(1);
		} 
	} else {
		fprintf(stderr,"Error, the first input must be tcp or udp.\n");
		exit(1);
	}
	return 0;
}