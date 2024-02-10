// Author: Steven Bertolucci
// Course: Operating System I
// Assignment: otp
// File: dec_client.c
// Description: 
// --------------------------------------------------------------------------------------------------------
//   This program will connect to dec_server and will ask it to decrypt ciphertext using a passed-in 
//   ciphertext and key, and otherwise performs exactly like enc_client, and must be runnable in the same 
//   three ways. dec_client should NOT be able to connect to enc_server, even if it tries to connect on the 
//   correct port - you’ll need to have the programs reject each other, as described in enc_client.
// --------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFER_SIZE 256

int charsRead;                                                                  /* Set charsRead as a global variable */
int charsWritten;                                                               /* Set charsWritten as a global variable */
int ciphertextLength = 0;                                                       /* Initialize count for the ciphertext file */
int keyLength = 0;                                                              /* Initialize count for the key file */
int socketFD;


void 
error(const char *msg)                                                          /* Error function used for reporting issues */
{                                                       
  perror(msg);
  exit(1);
}


void   
setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname)   /* Set up the address struct */
{
  memset((char*) address, '\0', sizeof(*address));
  address->sin_family = AF_INET;
  address->sin_port = htons(portNumber);

  struct hostent* hostInfo = gethostbyname(hostname);

  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(2);
  }

  memcpy((char*) &address->sin_addr.s_addr, 
         hostInfo->h_addr_list[0], 
         hostInfo->h_length);
}


void
checkArgs(int argc, char *argv[])
{
  /* ************************************** */
  /*                                        */
  /*        Checking Hostname Port          */
  /*                                        */
  /* ************************************** */

  if (argc < 3)                                                                 /* Check usage and args */ 
   {                                                               
     fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);                     /* Display error message */ 
     exit(0);
   }

}


void
createSocket()
{
  /* ************************************** */
  /*                                        */
  /*            Create Socket               */
  /*                                        */
  /* ************************************** */

  socketFD = socket(AF_INET, SOCK_STREAM, 0);                                   /* Create a socket */

  if (socketFD < 0) {                                                           /* Check if there was an issue creating a socket */

    error("CLIENT: ERROR opening socket");                                      /* Error message */

  }
}


void
makeSocketReusableAndConnect(struct sockaddr_in serverAddress)
{
  /* ********************************************** */
  /*                                                */
  /*  Making Socket Resuable and Connect to Server  */
  /*                                                */
  /* ********************************************** */

  /* Reference: https://beej.us/guide/bgnet/html/#setsockoptman */
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));            /* Make socket reusable */

 
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)     /* Connect to server */
  {
    error("CLIENT: ERROR connecting");                                          /* Display error message */
  }
}


void 
getFileLength(const char* ciphertext)                                           /* Function to count how many characters are in each file (plaintext and key) */
{
    /* ************************************** */
    /*                                        */
    /*    Calculating How Many Characters     */
    /*             Are In File                */
    /*                                        */
    /* ************************************** */
    FILE* ciphertextFile = fopen(ciphertext, "rb");                             /* Open plaintext file */

    int ch;                                                                     /* For holding characters */


    while ((ch = fgetc(ciphertextFile)) != EOF && ch != '\n') {                 /* Count cheacters in the plaintext file */
        ciphertextLength++;
    }

    fclose(ciphertextFile);                                                     /* Close the plaintext file */
}

int
resetBytesReceived() 
{
  /* ************************************** */
  /*                                        */
  /*       Resetting Bytes Received         */
  /*                                        */
  /* ************************************** */
  return 0;                                                                     /* Reset bytes received */

}

int 
resetBytesSent() 
{
  /* ************************************** */
  /*                                        */
  /*         Resetting Byes Sent            */ 
  /*                                        */
  /* ************************************** */
  return 0;                                                                     /* Reset bytes sent */

}


void 
sendCiphertextFile(char *argv[], int socketFD, long lengthOfBuffer)
{

  char* plaintext = argv[1];
  char buffer[BUFFER_SIZE]; 
  int bytesSent = resetBytesSent(), length = lengthOfBuffer - 1, value;
  FILE *file_descriptor;
  
  file_descriptor = fopen(plaintext, "rb");                                     /* Open plaintext file for reading */

  /* ************************************** */
  /*                                        */
  /*        SENDING CIPHERTEXT FILE         */
  /*                                        */
  /* ************************************** */

  while (bytesSent < ciphertextLength)                                           /* Send plaintext */
  {
    fread(buffer, 1, length, file_descriptor);
    value = send(socketFD, buffer, length, 0);                                  /* Send the message */
    bytesSent = value + bytesSent;

    if (bytesSent == -1)                                                        /* Check if the server returned an error "-1" */
    {
      error("CLIENT: ERROR writing ciphertext to socket");                       /* Error message when sending message to server */ 
    }
  }

  fclose(file_descriptor);

  //printf("8. Sent ciphertext\n");


}


void 
sendKeyFile(char *argv[], int socketFD, long lengthOfBuffer)
{

  char* key = argv[2];
  char buffer[BUFFER_SIZE]; 
  int bytesSent = resetBytesSent(), length = lengthOfBuffer - 1, value;
  FILE *file_descriptor;
  
  file_descriptor = fopen(key, "rb");                                             /* Open plaintext file for reading */

  /* ************************************** */
  /*                                        */
  /*        SENDING KEY FILE                */
  /*                                        */
  /* ************************************** */

  while (bytesSent < ciphertextLength)                                           /* Send plaintext */
  {
    fread(buffer, 1, length, file_descriptor);
    value = send(socketFD, buffer, length, 0);                                  /* Send the message */
    bytesSent = value + bytesSent;

    if (bytesSent == -1)                                                        /* Check if the server returned an error "-1" */
    {
      error("CLIENT: ERROR writing key  to socket");                            /* Error message when sending message to server */ 
    }
  }

  fclose(file_descriptor);

  //printf("8. Sent key\n");
}                                     


void
receivePlaintext(int socketFD, char* buffer, long lengthOfBuffer, char* plaintext)
{

  int bytesReceived, totalReceived = 0, length = lengthOfBuffer - 1;

  /* ************************************** */
  /*                                        */
  /*          Receiving Plaintext           */
  /*                                        */
  /* ************************************** */
  //printf("11. Receiving the plaintext from the server\n");
  bytesReceived = resetBytesReceived();
  
  //printf("Ciphertext length: %d\n", ciphertextLength);
  while (totalReceived < ciphertextLength) {

    bytesReceived = recv(socketFD, buffer, length, 0);                          /* Receive the plaintext */

    if (bytesReceived < 0) {

      error("ERROR reading ciphertext from socket");
      break;
    }

    for (int i = 0; i < bytesReceived; i++) {

      plaintext[totalReceived + i] = buffer[i];                                 /* Append buffer to plaintet */

    }

    totalReceived += bytesReceived;

  }
}

void
printPlaintext(char* plaintext)
{
  /* ************************************** */
  /*                                        */
  /*          Printing Plaintext            */
  /*                                        */
  /* ************************************** */
  //printf("12. Printing the ciphertext\n");
  printf("%s\n", plaintext);                                                    /* Output the plaintext */

}


void 
authenticate(int socketFD, struct sockaddr_in serverAddress, char* confirmPortNumber, void* buffer, int portNumber)
{
  int length = BUFFER_SIZE - 1;
  int size = sizeof(confirmPortNumber);
  
  //printf("DEC_CLIENT port number: %s\n", confirmPortNumber);
  /* ************************************** */
  /*                                        */
  /*      Authenticating Connection         */
  /*                                        */
  /* ************************************** */
  //printf("3. Sending port number to server for confirmation\n");
  charsWritten = send(socketFD, confirmPortNumber, size, 0);                    /* Send port number to the server to begin three way handshake */

  if (charsWritten < 0) {
    error("CLIENT: ERROR writing to server");                                   /* Print error message */ 
  }
  
  // Receivce SYNACK from server
  charsRead = recv(socketFD, buffer, length, 0);                                /* Receive a SYNACK from the server */

  if (charsRead < 0) {
    error("CLIENT: ERROR reading from socket");
  }

  // If returned buffer is not "SYNACK", exit on 2
  if (strcmp(buffer, "-1") == 0) 
  {
    fprintf(stderr, "Failed. dec_client attempted to connect to enc_server port #: %d\n", portNumber);
    exit(2);
  }

}


void
clearBuffer(char* buffer) 
{

  memset(buffer, '\0', BUFFER_SIZE);                                            /* Clear buffer array */

}


void
handshake(int socketFD, void* buffer, long bufferLength)
{
  int length = bufferLength - 1; 

  /* ************************************** */
  /*                                        */
  /*      Completing 3-Way Handshake        */
  /*                                        */
  /* ************************************** */

  //printf("4. Getting length of plaintext and save it to the buffer\n");
  snprintf(buffer, sizeof(buffer), "%d", ciphertextLength);

  //printf("5. Sending the file length to the server\n");
  charsWritten = send(socketFD, buffer, length, 0);
  clearBuffer(buffer);

  if (charsWritten < 0) 
  {
    error("CLIENT: ERROR sending to server");
  }

  //printf("6. Receiving a response from the server after sending buffer size\n");
  recv(socketFD, buffer, length, 0);

}


int main(int argc, char *argv[]) {

  int portNumber = atoi(argv[3]);
  struct sockaddr_in serverAddress;
  char buffer[256];
  char plaintext[70010];
  char* confirmServer = "62311"; 
  char host[10] = "localhost";
  //printf("DEC_CLIENT ARGS: %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);  
  //printf("DEC_CLIENT Port Number: %s\n", confirmServer);

  checkArgs(argc, argv);                                                        /* Check to see if we have less than 3 arguments before proceeding to creating a socket */

  getFileLength(argv[1]);                                                       /* Get file length */

  createSocket();                                                               /* Create Socket */

  setupAddressStruct(&serverAddress, portNumber, host);                         /* Set up the server address struct */

  makeSocketReusableAndConnect(serverAddress);
  
  authenticate(socketFD, serverAddress, confirmServer, buffer, portNumber);     /* Authenticate the connection with the server */

  handshake(socketFD, buffer, BUFFER_SIZE);                                     /* Complete the handshake after authenticating with the server */

  if (strcmp(buffer, "ACK") == 0) {

    /* If server is authenticated, send the files to the server */
    sendCiphertextFile(argv, socketFD, BUFFER_SIZE);
    sendKeyFile(argv, socketFD, BUFFER_SIZE);

  }

  receivePlaintext(socketFD, buffer, BUFFER_SIZE, plaintext);                   /* Receive the ciphertext */
  printPlaintext(plaintext); 

  close(socketFD);                                                              /* Close the socket */

  return 0;
}


