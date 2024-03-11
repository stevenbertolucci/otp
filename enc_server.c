// Author: Steven Bertolucci
// Course: Operating System I
// Assignment: otp
// File: enc_server.c
// Due Date: March 17, 2024
// Description: 
// ------------------------------------------------------------------------------------------------------
//   This program is the encryption server and will run in the background as a daemon. 
//    * Its function is to perform the actual encoding, as described above in the Wikipedia quote. 
//    * This program will listen on a particular port/socket, assigned when it is first ran (see syntax 
//      below).
//    * Upon execution, enc_server must output an error if it cannot be run due to a network error, 
//      such as the ports being unavailable.
//    * When a connection is made, enc_server must call accept to generate the socket used for actual 
//      communication, and then use a separate process to handle the rest of the servicing for this 
//      client connection (see below), which will occur on the newly accepted socket.
//    * This child process of enc_server must first check to make sure it is communicating with 
//      enc_client (see enc_client, below).
//    * After verifying that the connection to enc_server is coming from enc_client, then this child 
//      receives plaintext and a key from enc_client via the connected socket.
//    * The enc_server child will then write back the ciphertext to the enc_client process that it is 
//      connected to via the same connected socket.
//    * Note that the key passed in must be at least as big as the plaintext.
// ------------------------------------------------------------------------------------------------------

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
#define SIZE 69335
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
error(const char *msg)                                                   /* Error handling function */
{ 
  perror(msg); 
  exit(1); 
}


// Set up the address struct for the server socket
void 
setupAddressStruct(struct sockaddr_in* address, int portNumber)
{
    memset((char*) address, '\0', sizeof(*address));                     /* Clear out the address struct */
    address->sin_family = AF_INET;                                       /* The address should be network capable */
    address->sin_port = htons(portNumber);                               /* Store the port number */
    address->sin_addr.s_addr = INADDR_ANY;                               /* Allow a client at any address to connect to this server */
}


void 
reset()                                                                  /* Reset count */
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
bindSocket(int socket, struct sockaddr_in* serverAddress)                                             /* Bind socket */
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
receivePlaintext(int connectionSocket, char *plaintext, int buffer_length) 
{
  reset();
  char buffer[BUFFER_SIZE]; 
  
  /* ************************************ */
  /*                                      */
  /*        Receive Plaintext File        */
  /*                                      */
  /* ************************************ */

  while(1)
  {
    bytesReceived = recv(connectionSocket, buffer, BUFFER_255, 0);               /* Receive the plaintext from the client */
        
    if (bytesReceived < 0) {

      error("ERROR reading plaintext from socket");                              /* Print error if threre was an issue getting plaintext from client */
    }

    for (int i = 0; i < bytesReceived; i++) {
       plaintext[totalReceived + i] = buffer[i];
    }

    totalReceived += bytesReceived;

    if (totalReceived >= buffer_length)
    {
      break;
    }
  } 

  //printf("Received plaintext\n");
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
    bytesReceived = recv(connectionSocket, buffer, BUFFER_255, 0);
          
    if (bytesReceived < 0) {

      error("ERROR reading key from socket");

    }

    for (int i = 0; i < bytesReceived; i++) {
           
      key[totalReceived + i] = buffer[i];
    }

    totalReceived += bytesReceived;

    if (totalReceived >= buffer_length)
    {
      break;
    }
  }

  //printf("Received key\n");
}


void 
sendCiphertextBack(int connectionSocket, char* ciphertext, int buffer_length)
{

  /* ************************************** */
  /*                                        */
  /*          Sending Ciphertext            */
  /*                                        */
  /* ************************************** */
  //printf("Sending ciphertext to client\n");
  reset();                                                                        /* Reset count */

  while(1)
  {
    charsRead += send(connectionSocket, ciphertext, buffer_length, 0);            /* Send the ciphertext back to client */

    if (charsRead < 0) {

        error("ERROR writing ciphertext to socket");                              /* Print error if there was an issue sending the ciphertext back to client */

    }

    if (charsRead >= buffer_length)
    {
      break;
    }
  }
}


int 
receive(int connectionSocket, char* buffer, int size)                             /* Function to receive messages */
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
    send(connectionSocket, message, strlen(message), 0);                /* Send response to client to verify connection */
    result = 0;

  } else {

    result = 1;
  }

  return result;
}


void 
sendRequest(int connectionSocket, char* request)
{

   send(connectionSocket, request, strlen(request), 0);                 /* Send message */

}


void 
receiveResponse(int connectionSocket, char* buffer, int size_of_buffer)
{

  recv(connectionSocket, buffer, size_of_buffer, 0);                    /* Receive message */

}


void 
encrypt(char* ciphertext, const char* plaintext, const char* key)       /* Function encrypt to encrypt the plaintext with the key  */
{  
  /* ************************************** */
  /*                                        */
  /*          Encrypt Plaintext             */
  /*                                        */
  /* ************************************** */

  int i;        

  for (i = 0; plaintext[i] != '\n'; i++) {                              /* Loop through each character in the plaintext and encrypt them */

    char c = plaintext[i]; 
    char k = key[i];

    if (c == ' ') {                                                     /* If space, save the space since it does not need to be encrypted */

      ciphertext[i] = ' ';

    } else {

      c = (c - 'A');                                                    /* Convert ascii value to int */
      k = (k - 'A');                                                    /* Convert ascii value to int */

      int value = ((c + k) % 27);                                       /* Modulo operation using 27 characters */

      if (value < 0) {                                                  /* If value is negative, add 27 */

        value += 27;

      }

      value += 'A';                                                     /* Convert int back to char */

      ciphertext[i] = value;                                            /* Save the character to the ciphertext array */
    }
  }

  ciphertext[i] = '\0';                                                 /* Null-terminate the ciphertext */
}

/* ##################################################################################################### */
/* #                                                                                                   # */             
/* #                               END OF FUNCTION DECLARATIONS                                        # */
/* #                                                                                                   # */
/* ##################################################################################################### */

int main(int argc, char *argv[]) {

  // Variables
  int connectionSocket;
  char* portNumber = argv[1];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);
  char message1[7] = "SYNACK", message2[4] = "ACK";

  checkArgs(argc, argv);

  createSocket();
 
  setupAddressStruct(&serverAddress, atoi(argv[1]));                                                  /* Set up the address struct for the server socket */

  bindSocket(listenSocket, &serverAddress);

  listen(listenSocket, 5);                                                                            /* Start listening up to 5 connections */                           
  
  while(1) 
  {
    connectionSocket = acceptConnection(listenSocket, &clientAddress, &sizeOfClientInfo);
  

   // printf("13. SERVER: Connected to client running at host %d port %d\n", 
     //      ntohs(clientAddress.sin_addr.s_addr),
       //    ntohs(clientAddress.sin_port));

    pid_t pid = fork();                                                                 /* Fork a process to handle multiple connections */

    if (pid == -1) 
    {

      error("SERVER: Error Forking");                                                   /* Couldn't fork. Fork error */

    }

    if (pid == 0) {                                                                     /* Child process */

      char plaintext[SIZE], key[KEY_SIZE], ciphertext[SIZE], buffer[BUFFER_SIZE];       /* Declare size of each array */
      charsRead = 0;

      // Initalize the arrays
      memset(plaintext, '\0', sizeof(plaintext));
      memset(key, '\0', sizeof(key));
      memset(ciphertext, '\0', sizeof(ciphertext));
      memset(buffer, '\0', sizeof(buffer));

      receive(connectionSocket, buffer, BUFFER_255);                                    /* Receive message */

      int result = verify_connection(connectionSocket, buffer, portNumber);             /* Verify connection */
      int the_buffer_size = BUFFER_255;

      if (!result)
      {
        exit(2);                                                                        /* If incorrect port is being used, exit */

      } else {
        
        // Send/receive messages
        sendRequest(connectionSocket, message1);
        receiveResponse(connectionSocket, buffer, the_buffer_size);
        sendRequest(connectionSocket, message2);
 
        int length = strtol(buffer, NULL, 10);                                          /* Convert string size to integer */
        receivePlaintext(connectionSocket, plaintext, length);                          /* Receive plaintext */
        receiveKey(connectionSocket, key, length);                                      /* Receive key */

        //printf("Now encrypting plaintext\n");
        encrypt(ciphertext, plaintext, key);                                            /* Encrypt the plaintext with the key and save it to ciphertext */

        sendCiphertextBack(connectionSocket, ciphertext, sizeof(ciphertext));           /* Send ciphertext back to client */
 
        exit(0);
        }
      }

    close(connectionSocket);                                                            /* Close the connection socket for the child process */
    while(waitpid(-1, NULL, WNOHANG) > 0);                                              /* Clean up zombie processes */
  }
  
  close(listenSocket);                                                                  /* Close the listening socket */

  return 0;
}

