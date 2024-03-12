// Author: Steven Bertolucci
// Course: Operating System I
// Assignment: otp
// File: dec_server.c
// Due Date: March 17, 2024
// Description: 
// --------------------------------------------------------------------------------------------------------
//   This program performs exactly like enc_server, in syntax and usage. In this case, however, dec_server 
//   will decrypt ciphertext it is given, using the passed-in ciphertext and key. Thus, it returns plaintext 
//   again to dec_client.
// --------------------------------------------------------------------------------------------------------

/* ##################################################################################################### */
/* #                                                                                                   # */
/* #                               !! NOTICE OF REUSED CODE !!!                                        # */
/* #                                                                                                   # */
/* #     I am reusing SOME of the code from last quarter. The only code that I reused are those        # */
/* #     were in the modules and provided stater code like setupAddressStruct(), setsockopt(),         # */ 
/* #     socket(), bind(), listen(), accept(), fork(), recv(), send(), waitpid(), and close(). All     # */
/* #     other code is written by me with the help of Linux man page and the textbook.                 # */
/* #     The provided replit in the modules for the starter code of the server is below:               # */
/* #                  https://replit.com/@cs344/83serverc?lite=true#server.c                           # */
/* #                                                                                                   # */
/* #                                                                                                   # */
/* ##################################################################################################### */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

/* ######################## */
/*                          */
/*     GLOBAL VARIABLES     */
/*                          */
/* ######################## */

#define BUFFER_SIZE 256
#define BUFFER_255 255
#define SIZE 70010
#define KEY_SIZE 70001
int charsRead, totalReceived, bytesReceived;
int listenSocket;
int result;

/* ##################################################################################################### */
/* #                                                                                                   # */             
/* #                              START OF FUNCTION DECLARATIONS                                       # */
/* #                                                                                                   # */
/* ##################################################################################################### */

void 
error(const char *msg)                                                                      /* Error handling function */
{ 
  perror(msg); 
  exit(1); 
}


// Set up the address struct for the server socket
void 
setupAddressStruct(struct sockaddr_in* address, int portNumber)
{
    memset((char*) address, '\0', sizeof(*address));                                        /* Clear out the address struct */
    address->sin_family = AF_INET;                                                          /* The address should be network capable */
    address->sin_port = htons(portNumber);                                                  /* Store the port number */
    address->sin_addr.s_addr = INADDR_ANY;                                                  /* Allow a client at any address to connect to this server */
}


void
reset()
{
  totalReceived = 0;
  bytesReceived = 0;
  charsRead = 0;
}


void 
checkArgs(int argc, char *argv[])
{

  if (argc < 2) {                                                                                     /* Check usage & args */

    fprintf(stderr, "USAGE: %s port\n", argv[0]);                                                     /* Print error */
    exit(1);

  }
}


void 
createSocket()
{
  listenSocket = socket(AF_INET, SOCK_STREAM, 0);                                                     /* Create the socket that will listen for connections */

  if (listenSocket < 0) 
  {

    error("ERROR opening socket");                                                                    /* Print error */

  }
}


void 
bindSocket(int socket, struct sockaddr_in* serverAddress)                                             /* Bind Socket */
{

    if (bind(socket, (struct sockaddr *)serverAddress, sizeof(*serverAddress)) < 0) {             
        error("ERROR on binding");
    }
}


int
acceptConnection(int listenSocket, struct sockaddr_in* clientAddress, socklen_t* sizeOfClientInfo) 
{

  int connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, sizeOfClientInfo);    /* Accept the connection */
    
  if (connectionSocket < 0) {                                                         /* Check for connectionn error */

    error("ERROR on accept");                                                         /* Print error message if there were accepting issues */

  }

  return connectionSocket;
}


void 
receiveCiphertext(int connectionSocket, char *ciphertext, int buffer_length)
{
  
  reset();
  char buffer[BUFFER_SIZE];

  /* ************************************ */
  /*                                      */
  /*        Receive Ciphertext File       */
  /*                                      */
  /* ************************************ */

  while(1)
  {
    bytesReceived = recv(connectionSocket, buffer, BUFFER_255, 0);           /* Receive the ciphertext from the client */

    if (bytesReceived < 0) {

      error("ERROR reading ciphertext from socket");                                 /* Error message */

    }

    for (int i = 0; i < bytesReceived; i++) {

      ciphertext[totalReceived + i] = buffer[i];                                     /* Save buffer content to ciphertext */

    }

    totalReceived += bytesReceived;              
    
    if (totalReceived > buffer_length) {
      break;
    }
  } 

  //printf("Received ciphertext\n");
}


void 
receiveKey(int connectionSocket, char *key, int buffer_length)
{

  reset();
  char buffer[BUFFER_SIZE];

  /* ************************************* */
  /*                                       */
  /*           Receive Key File            */
  /*                                       */
  /* ************************************* */
  //printf("Now reading in key\n");

  while(1)
  {
    bytesReceived = recv(connectionSocket, buffer, BUFFER_255, 0);            /* Receive the key from the client */

    if (bytesReceived < 0) {

      error("ERROR reading key from socket");                                         /* Print error message */

    }

    for (int i = 0; i < bytesReceived; i++) {

      key[totalReceived + i] = buffer[i];                                             /* Save buffer content to key */

    }

    totalReceived += bytesReceived;
    
    if (totalReceived > buffer_length) 
    {
      break;
    }
  } 

  // printf("Received key\n");
}


int
receive(int connectionSocket, char* buffer, int size)                                 /* Function to receive messages */
{
  
  charsRead = recv(connectionSocket, buffer, size, 0);

  if (charsRead < 0)
  {
    error("ERROR reading from socket");
  }


  return charsRead;
}


int
verify_connection(int connectionSocket, char* buffer, char* portNumber)
{

  char message[19] = "Wrong port, buddy!";

  if (strcmp(buffer, portNumber) < 0 || strcmp(buffer, portNumber) > 0)
  {
    send(connectionSocket, message, strlen(message), 0);                                    /* Send message back to client to verify connection */
    result = 0;

  } else {

    result = 1;
  }

  return result;
}


void 
sendPlaintextBack(int connectionSocket, char* plaintext, int buffer_length)
{

  /* ************************************** */
  /*                                        */
  /*          Sending Plaintext             */
  /*                                        */
  /* ************************************** */
  //printf("Sending plaintext to client\n");
  reset();                                                                                  /* Reset count */

  while(1)
  {
    charsRead += send(connectionSocket, plaintext, buffer_length, 0);                       /* Send the plaintext back to client */
    
    if (charsRead < 0) {

    error("ERROR writing plaintext to socket");                                             /* Print error if there was an issue sending the plaintext back to client */

    } 

    if (charsRead > buffer_length)
    {
      break;
    }
  }
}


void 
sendRequest(int connectionSocket, char* request)
{

   send(connectionSocket, request, strlen(request), 0);

}


void 
receiveResponse(int connectionSocket, char* buffer, int size_of_buffer)
{

  recv(connectionSocket, buffer, size_of_buffer, 0);

}


void 
decrypt(char* plaintext, const char* ciphertext, const char* key)                           /* Decrypt the message */
{
  /* ************************************** */
  /*                                        */
  /*          Decrypt Ciphertext            */
  /*                                        */
  /* ************************************** */

  int value, i;

	for (i = 0; ciphertext[i] != '\n'; i++)                                                   /* Loop through each characters and decrypt them */
  {		
    int c = ciphertext[i];
    int k = key[i];

    if (c == ' ') {

      plaintext[i] = ' ';
      

    } else {

      c = (c - 65);
      k = (k - 65); 

      value = ((c - k) % 27);                                                               /* Modulo 27 division */

     if (value < 0) {

       value += 27;

     }

     if (value == 26) {                                                                     /* Handle value 26 as a space */

        plaintext[i] = ' ';

     } else {

      value += 65;
      plaintext[i] = value;                                                                 /* Store the decrypted text to plaintext array */

	  }
   }
  }

	plaintext[i] = '\0';                                                                      /* Append null terminator to the last byte */
}    

/* ##################################################################################################### */
/* #                                                                                                   # */             
/* #                               END OF FUNCTION DECLARATIONS                                        # */
/* #                                                                                                   # */
/* ##################################################################################################### */

int main(int argc, char *argv[]) {

  // Variables
  int connectionSocket;
  char* portNumber = "62311";
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  char message1[7] = "SYNACK", message2[4] = "ACK";

  checkArgs(argc, argv);                                                                    /* Check if there are enough required arguments */

  createSocket();                                                                           /* Create socket */

  setupAddressStruct(&serverAddress, atoi(argv[1]));                                        /* Set up the address struct for the server socket */

  bindSocket(listenSocket, &serverAddress);

  listen(listenSocket, 5);                                                                  /* Start listening up to 5 connections */                           
  
  while(1) 
  {
    connectionSocket = acceptConnection(listenSocket, &clientAddress, &sizeOfClientInfo);

    // printf("13. SERVER: Connected to client running at host %d port %d\n", 
    //       ntohs(clientAddress.sin_addr.s_addr),
    //       ntohs(clientAddress.sin_port));

    pid_t pid = fork();                                                                 /* Fork a process to handle multiple connections */

    if (pid == -1) {

      error("SERVER: Error Forking");                                                   /* Couldn't fork. Fork error */

    }

    if (pid == 0) {                                                                     /* Child process */

      char plaintext[SIZE], key[KEY_SIZE], ciphertext[SIZE], buffer[BUFFER_SIZE];       /* Declare size of each array */
      charsRead = 0;

      // Initialize the arrays
      //memset(plaintext, '\0', sizeof(plaintext));
      //memset(key, '\0', sizeof(key));
      //memset(ciphertext, '\0', sizeof(ciphertext));
      //memset(buffer, '\0', sizeof(buffer));

      receive(connectionSocket, buffer, BUFFER_255);                                    /* Receive */

      int result = verify_connection(connectionSocket, buffer, portNumber);
      int the_buffer_size = BUFFER_255;

      if (!result)
      {
        exit(2);                                                                        /* Check correct port connection */

      } else {

         // Send receive messages
        sendRequest(connectionSocket, message1);
        receiveResponse(connectionSocket, buffer, the_buffer_size);
        sendRequest(connectionSocket, message2);

        int length = atoi(buffer);                                         /* Convert string size to integer */
        receiveCiphertext(connectionSocket, ciphertext, length);                        /* Receive ciphertext */
        receiveKey(connectionSocket, key, length);                                      /* Receive key */

        //printf("Now decrypting ciphertext\n");
        decrypt(plaintext, ciphertext, key);                                            /* Decrypt the ciphertext with the key and save it to plaintext */

        sendPlaintextBack(connectionSocket, plaintext, sizeof(plaintext));              /* Send plaintext back to client */
      
        exit(0);

        }
      }

    close(connectionSocket);                                                            /* Close the connection socket for the child process */
    while(waitpid(-1, NULL, WNOHANG) > 0);                                              /* Clean up zombie processes */
  }
  
  close(listenSocket);                                                                  /* Close the listening socket */

  return 0;
}


