#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h> /* for sockaddr_in and inet_addr() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#include "ProtocolTCP.h"

#include <arpa/inet.h>
void DieWithError(char *errorMessage); /* Error handling function */
void rcvEcho(int echoStringLen, int sock);
char* recvMsg(int sock);
void sendEcho(int sock);
void sendCmd(int sock, char cmd);
char* sendMsg(int sock, char *str, char *errorMsg);
void sendFile(int sock, char *servIP);
int getFileSize(FILE *fp);
void recvFile(int sock);

int main(int argc, char *argv[]) {

	int sock; /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
	char *servIP; /* Server IP address (dotted quad) */
	int bytesRcvd;//, totalBytesRcvd; /* Bytes read in single recv() and total bytes read */
	char *connString;
	unsigned short echoServPort;	
	
	servIP = (char *)malloc(sizeof(servIP));
	printf("Server IP: ");
	printf("127.0.0.1\n");
	
	//gets(servIP);
	printf("Port: ");
	//scanf("%hd",&echoServPort);
	printf("55555\n");
	strcpy(servIP,"127.0.0.1"); /* First arg: server IP address (dotted quad) */	
	echoServPort = 55555;
	connString = "hello";	
	
	/* Create a reliable, stream socket using TCP */
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		DieWithError("socket() failed");
	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
	echoServAddr.sin_family = AF_INET; /* Internet address family */
	echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
	echoServAddr.sin_port = htons(echoServPort); /* Server port */

	/* Establish the connection to the echo server */
	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
		DieWithError("connect() failed");	

	/* Send the string to the server */
	sendMsg(sock, connString, "send() connString send Error");
	recvMsg(sock);
	
	while(1){
		char cmd = 0;
		printf("command [  m)sg  p)ut  g)et  l)s  r)ls  e)xit  ] ->");
		scanf("%c",&cmd);

		getchar();
		sendCmd(sock, cmd);
		if (cmd == 'm'){
			sendEcho(sock);
			getchar();
		}
		else if (cmd == 'p'){
			sendFile(sock, servIP);
			getchar();
		}
		else if (cmd == 'g'){
			recvFile(sock);
		}
		else if (cmd == 'l'){

		}
		else if (cmd == 'r'){

		}
		else{
			break;
		}
		
	}

	free(servIP);
	close(sock);
	exit(0);
}

void recvFile(int sock){
	char* fName;//[FileName];
	int fSize = 0, recvSize = 0, bufSize = 0;
	char buf[RCVBUFSIZE];
	FILE *fp;
	char path[FileName] = "./";
	char* fsize;
	int n = 0;

	/*get file name and file size*/
	fName = recvMsg(sock);
	strcat(path, fName);

	fsize = recvMsg(sock);
	fSize = atoi(fsize);

	/* check whether file is exist */
	if (access(path, F_OK) == 0){
		strcpy(fName, "temp.txt");
	}

	fp = fopen(fName, "wb");
	if (fp == NULL)
		DieWithError("File Open Error");

	/* receive the file*/
	bufSize = RCVBUFSIZE;
	while (fSize != 0){
		if (fSize < RCVBUFSIZE)
			bufSize = fSize;

		recvSize = recv(sock, buf, bufSize, 0);
		fSize -= recvSize;
		fwrite(buf, sizeof(char), recvSize, fp);
		recvSize = 0;
	}

	fclose(fp);
}

void sendFile(int sock, char *servIP){
	FILE *fp;
	int fSize = 0, totalSize = 0, sendSize = 0;
	char fName[FileName];
	char buf[RCVBUFSIZE];

	/*enter file name*/
	printf("Filename to put to server -> ");
	scanf("%s", fName);	
	
	/* open file*/
	fp = fopen(fName, "rb");
	if (fp == NULL)
		DieWithError("File Open Error");

	
	/*send file name*/
	sendMsg(sock, fName, "FileName send Error");

	/*send file size as char* */
	fSize = getFileSize(fp);
	char fsize[RCVBUFSIZE];
	sprintf(fsize, "%d", fSize);	
	sendMsg(sock, fsize, "Send Size Error");

	printf("Sending => ###############\n");
	/*send file content*/
	while (totalSize != fSize){
		sendSize = fread(buf, 1, RCVBUFSIZE, fp);
		totalSize += sendSize;
		send(sock, buf, sendSize, 0);
	}
	printf("%s(%d bytes) uploading success to %s\n", fName, totalSize, servIP);

	fclose(fp);
}

/* get file size*/
int getFileSize(FILE *fp){
	int fsize = 0;
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	return fsize;
}

/*send message to server*/
char* sendMsg(int sock, char *str, char *errorMsg){
	printf("Send: ");
	if (str == NULL){
		str = (char *)malloc(sizeof(str));
		scanf("%s", str);
	}
	else{
		printf("%s \n", str);
	}

	if (send(sock, str, strlen(str), 0) <0)
		DieWithError(errorMsg);
	
	return str;
}

/*receieve message from server*/
char* recvMsg(int sock){
	char buff[RCVBUFSIZE];
	char *rtnBuf;
	int recvMsgSize; /* Size of received message */

	if ((recvMsgSize = recv(sock, buff, RCVBUFSIZE - 1, 0)) < 0)
		DieWithError("recv() failed");
	buff[recvMsgSize] = '\0';

	printf("Received: %s \n", buff);
	rtnBuf = buff;
	return rtnBuf;
}

/*send protocol to server*/
void sendCmd(int sock, char cmd){

	char *cmdStr = (char *)malloc(sizeof(cmdStr));
	if (cmd == 'm'){
		strcpy(cmdStr, EchoReq);
	}
	else if (cmd == 'p'){
		strcpy(cmdStr, FileUpReq);
	}
	else if (cmd == 'g'){
		strcpy(cmdStr, FileDnReq);
	}
	else if (cmd == 'l'){
		strcpy(cmdStr, LsReq);
	}
	else if (cmd == 'r'){
		strcpy(cmdStr, RlsReq);
	}
	else {
		strcpy(cmdStr, ExitReq);
	}

	sendMsg(sock, cmdStr, "sendCmdError");
	free(cmdStr);
}

/*send echo message to server*/
void sendEcho(int sock){

	unsigned int echoStringLen; /* Length of string to echo */
	char * echoString = sendMsg(sock, NULL, "send() sent a different number of bytes than expected");
	//printf("Send: %s\n", echoString);
	echoStringLen = strlen(echoString);
	rcvEcho(echoStringLen, sock);
	free(echoString);
}

/*receive echo message from server*/
void rcvEcho(int echoStringLen,int sock){
	int totalBytesRcvd = 0;
	int bytesRcvd = 0;
	char echoBuffer[RCVBUFSIZE];
	
	printf("Received: "); /*Setup to print the echoed string */
	while (totalBytesRcvd < echoStringLen) {
   /* Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender */
		if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0)
      		 DieWithError("recv() failed or connection closed prematurely");
    	totalBytesRcvd += bytesRcvd; /* Keep tally of total bytes */
    	echoBuffer[bytesRcvd] = '\0'; /* Terminate the string! */
    	printf("%s \n",echoBuffer); /* Print the echo buffer */
	}
}