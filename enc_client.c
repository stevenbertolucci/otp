// Author: Steven Bertolucci
// Course: Operating System I
// Assignment: otp
// File: enc_client.c
// Description: 
// --------------------------------------------------------------------------------------------------------
//  This program connects to enc_server, and asks it to perform a one-time pad style encryption as 
//  detailed above. By itself, enc_client doesn’t do the encryption - enc_server does. 
//  If enc_client receives key or plaintext files with ANY bad characters in them, or the key file 
//  is shorter than the plaintext, then it should terminate, send appropriate error text to stderr,
//  and set the exit value to 1.
//
//  enc_client should NOT be able to connect to dec_server, even if it tries to connect on the correct
//  port - you’ll need to have the programs reject each other. If this happens, enc_client should 
//  report the rejection to stderr and then terminate itself. In more detail: if enc_client cannot 
//  connect to the enc_server server, for any reason (including that it has accidentally tried to 
//  connect to the dec_server server), it should report this error to stderr with the attempted port, 
//  and set the exit value to 2. Otherwise, upon successfully running and terminating, enc_client 
//  should set the exit value to 0.
//
//  Again, any and all error text must be output to stderr (not into the plaintext or ciphertext files).
// --------------------------------------------------------------------------------------------------------

/* ##################################################################################################### */
/* #                                                                                                   # */
/* #                               !! NOTICE OF REUSED CODE !!!                                        # */
/* #                                                                                                   # */
/* #     I am reusing SOME of the code from last quarter. The only code that I reused are those        # */
/* #     was in the modules and provided stater code like setupAddressStruct(), socket(), bind(),      # */
/* #     listen(), accept(), fork(), recv(), send(), waitpid(), and close(). All other code is         # */
/* #     written by me with the help of Linux man page and the textbook.                               # */
/* #                                                                                                   # */
/* #                                                                                                   # */
/* ##################################################################################################### */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

#define BUFFER_SIZE 256                                                                 /* Constant size for buffer */

int charsRead;                                                                          /* Set charsRead as a global variable */
int charsWritten;                                                                       /* Set charsWritten as a gglobal variable */
int plaintextLength = 0;                                                                /* Initialize count for the plaintext file */
int keyLength = 0;                                                                      /* Initialize count for the key file */
int socketFD;
int totalLengthOfFile;

void 
error(const char *msg)                                                                  /* Error function used for reporting issues */
{                                                                  
  perror(msg);
  exit(1);
}


void   
setupAddressStruct(struct sockaddr_in* address, int portNumber, char* hostname)         /* Set up the address struct */
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
checkPort(int argc, char *argv[])
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


void checkBadCharacters(const char* filename)                                    /* Function to check for bad characters */ 
{    
  /* ************************************** */
  /*                                        */
  /*        Check for Bad Characters        */
  /*                                        */
  /* ************************************** */

  FILE* file = fopen(filename, "r");                                            /* Open the plaintext file */

  if (file == NULL)                                                             /* Extra measure to check if the file is empty */
  {
    exit(0);
  }
  
  char ch;                                                                      /* To hold characters */

  while ((ch = fgetc(file)) != EOF) {                                           /* Start checking the characters */

    if (ch != ' ' && ch != '\n' && (ch < 'A' || ch > 'Z')) {                    /* If the character is not a space or A-Z */
      fprintf(stderr, "Stop right there, criminal scum! Your plaintext file contains bad characters!\n");    /* Copyrighted by 2023 ZeniMax Media Inc. All Rights Reserved. Quote was taken from the imperial 
                                                                                                                    guard in the game. Reference: https://elderscrolls.bethesda.net/en/oblivion */
      fclose(file);                                                             /* Close file */
      exit(1);

    }
  }
  
  fclose(file);                                                                 /* File is good to go! Close the file */
}


void 
checkFileLength(const char* plaintext, const char* key)                         /* Function to count how many characters are in each file (plaintext and key) */
{        
  /* ************************************** */
  /*                                        */
  /*    Calculating How Many Characters     */
  /*           Are In Each File             */
  /*                                        */
  /* ************************************** */

  FILE* plaintextFile = fopen(plaintext, "r");                                  /* Open plaintext file */
  FILE* keyFile = fopen(key, "r");                                              /* Open key file */

  int ch;                                                                       /* For holding characters */


  while ((ch = fgetc(plaintextFile)) != EOF && ch != '\n') {                    /* Count cheacters in the plaintext file */
      plaintextLength++;
  }

  while ((ch = fgetc(keyFile)) != EOF && ch != '\n') {                          /* Count cheacters in the key file */
      keyLength++;
  }

  fclose(plaintextFile);                                                        /* Close the plaintext file */
  fclose(keyFile);                                                              /* Close the key file */

  if (keyLength < plaintextLength)                                              /* Check if the plaintext file is larger than the key */
  {
      fprintf(stderr, "CLIENT: ERROR, key is shorter than plaintext!\n");       /* Error message */
      exit(1);
  }
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
sendPlaintextFile(char *argv[], int socketFD, long lengthOfBuffer)
{

  char* plaintext = argv[1];
  char buffer[BUFFER_SIZE]; 
  int bytesSent = resetBytesSent(), length = lengthOfBuffer - 1, value;
  FILE *file_descriptor;
  
  file_descriptor = fopen(plaintext, "rb");                                     /* Open plaintext file for reading */

  /* ************************************** */
  /*                                        */
  /*        SENDING PLAINTEXT FILE          */
  /*                                        */
  /* ************************************** */

  while (bytesSent < plaintextLength)                                           /* Send plaintext */
  {
    fread(buffer, 1, length, file_descriptor);
    value = send(socketFD, buffer, length, 0);                                  /* Send the message */
    bytesSent = value + bytesSent;

    if (bytesSent == -1)                                                        /* Check if the server returned an error "-1" */
    {
      error("CLIENT: ERROR writing plaintext to socket");                       /* Error message when sending message to server */ 
    }
  }

  fclose(file_descriptor);

  //printf("8. Sent plaintext\n");


}


void 
sendKeyFile(char *argv[], int socketFD, long lengthOfBuffer)
{

  char* key = argv[2];
  char buffer[BUFFER_SIZE]; 
  int bytesSent = resetBytesSent(), length = lengthOfBuffer - 1, value;
  FILE *file_descriptor;
  
  file_descriptor = fopen(key, "rb");                                     /* Open plaintext file for reading */

  /* ************************************** */
  /*                                        */
  /*          SENDING KEY FILE              */
  /*                                        */
  /* ************************************** */

  while (bytesSent < plaintextLength)                                           /* Send plaintext */
  {
    fread(buffer, 1, length, file_descriptor);
    value = send(socketFD, buffer, length, 0);                                  /* Send the message */
    bytesSent = value + bytesSent;

    if (bytesSent == -1)                                                        /* Check if the server returned an error "-1" */
    {
      error("CLIENT: ERROR writing plaintext to socket");                       /* Error message when sending message to server */ 
    }
  }

  fclose(file_descriptor);

  //printf("8. Sent key\n");
}


void
receiveCiphertext(int socketFD, char* buffer, long lengthOfBuffer, char* ciphertext)
{

  int bytesReceived, totalReceived = 0, length = lengthOfBuffer - 1;

  /* ************************************** */
  /*                                        */
  /*          Receiving Ciphertext          */
  /*                                        */
  /* ************************************** */
  //printf("11. Receiving the ciphertext from the server\n");
  //bytesReceived = resetBytesReceived();
  
  //printf("Plaintext length: %d\n", plaintextLength);
  while (totalReceived < plaintextLength) {
    bytesReceived = recv(socketFD, buffer, length, 0);                          /* Receive the ciphertext */

    if (bytesReceived < 0) {

      error("ERROR reading ciphertext from socket");
      break;
    }

    for (int i = 0; i < bytesReceived; i++) {

      ciphertext[totalReceived + i] = buffer[i];                                /* Append Buffer to ciphertext */ 
    }

    totalReceived += bytesReceived;
  }
}


void
printCiphertext(char* ciphertext)
{
  /* ************************************** */
  /*                                        */
  /*          Printing Ciphertext           */
  /*                                        */
  /* ************************************** */
  //printf("12. Printing the ciphertext\n");
  printf("%s\n", ciphertext);                                                   /* Output the ciphertext */

}


void 
authenticate(int socketFD, struct sockaddr_in serverAddress, char* confirmPortNumber, void* buffer, int portNumber)
{
  int length = BUFFER_SIZE - 1;
  int size = sizeof(confirmPortNumber);
  //printf("ENC_CLIENT port number: %s\n", confirmPortNumber);

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
    fprintf(stderr, "Failed. enc_client attempted to connect to dec_server port #: %d\n", portNumber);
    exit(2);
  }

}


void
clearBuffer(char* buffer) 
{
  /* ************************************** */
  /*                                        */
  /*      Clearing the Buffer Array         */
  /*                                        */
  /* ************************************** */

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
  snprintf(buffer, sizeof(buffer), "%d", plaintextLength);

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
  char* confirmPortNumber = argv[3];
  char host[10] = "localhost";
  //printf("ENC_CLIENT ARGS: %s %s %s %s\n", argv[0], argv[1], argv[2], argv[3]);
  //printf("ENC_CLIENT Server port: %d\n", serverAddress.sin_port);

  checkPort(argc, argv);                                                        /* Check to see if we have less than 3 arguments before proceeding to creating a socket */

  //printf("1. Counting plaintext and key file\n");
  checkFileLength(argv[1], argv[2]);                                            /* Check file length */
  char ciphertext[plaintextLength];

  //printf("2. Checking for bad characters\n");
  checkBadCharacters(argv[1]);                                                  /* Check for bad characters */
 
  createSocket();                                                               /* Create Socket */

  setupAddressStruct(&serverAddress, portNumber, host);                         /* Set up the server address struct */

  makeSocketReusableAndConnect(serverAddress);
  
  authenticate(socketFD, serverAddress, confirmPortNumber, buffer, portNumber);   /* Authenticate the connection with the server */
  
  handshake(socketFD, buffer, BUFFER_SIZE);                                     /* Complete the handshake after authenticating with the server */
 
  if (strcmp(buffer, "ACK") == 0) {
    
    /* If server is authenticated, send the files to the server */
    sendPlaintextFile(argv, socketFD, BUFFER_SIZE);
    sendKeyFile(argv, socketFD, BUFFER_SIZE);
   
  }

  receiveCiphertext(socketFD, buffer, BUFFER_SIZE, ciphertext);                 /* Receive the ciphertext */
  printCiphertext(ciphertext);

  close(socketFD);                                                              /* Close the socket */

  return 0;
}
