// Author: Steven Bertolucci
// Course: Operating System I
// Assignment: otp
// File: enc_server.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
int charsRead; 


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
encrypt(char* ciphertext, const char* plaintext, const char* key)       /* Function encrypt to encrypt the plaintext with the key  */
{    
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


void
resetBytesReceived() 
{

  charsRead = 0;

}



int main(int argc, char *argv[]) {

  int listenSocket, connectionSocket, buffer_length;
  char* portNumber = argv[1];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  if (argc < 2) {                                                                                     /* Check usage & args */

    fprintf(stderr, "USAGE: %s port\n", argv[0]);                                                     /* Print error */
    exit(1);

  }
  
  listenSocket = socket(AF_INET, SOCK_STREAM, 0);                                                     /* Create the socket that will listen for connections */

  if (listenSocket < 0) 
  {

    error("ERROR opening socket");                                                                    /* Print error */

  }

  setupAddressStruct(&serverAddress, atoi(argv[1]));                                                  /* Set up the address struct for the server socket */

  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {             /* Associate the socket to the port */
    error("ERROR on binding");
  }

  listen(listenSocket, 5);                                                                            /* Start listening up to 5 connections */                           
  
  while(1) 
  {
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);    /* Accept the connection */

    if (connectionSocket < 0) {                                                         /* Check for connectionn error */

      error("ERROR on accept");                                                         /* Print error message if there were accepting issues */

    }

   // printf("13. SERVER: Connected to client running at host %d port %d\n", 
     //      ntohs(clientAddress.sin_addr.s_addr),
       //    ntohs(clientAddress.sin_port));

    pid_t pid = fork();                                                                 /* Fork a process to handle multiple connections */

    if (pid == -1) 
    {

      error("SERVER: Error Forking");                                                   /* Couldn't fork. Fork error */

    }

    if (pid == 0) {                                                                     /* Child process */

      char plaintext[70010];
      char key[70010];
      char ciphertext[70010];
      char buffer[256];
      charsRead = 0;
      int totalReceived = 0;
      int bytesReceived = 0;


      memset(plaintext, '\0', sizeof(plaintext));
      memset(key, '\0', sizeof(key));
      memset(ciphertext, '\0', sizeof(ciphertext));
      memset(buffer, '\0', sizeof(buffer));

    
      charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);

      if (charsRead < 0)
      {
        error("ERROR reading from socket");
      }

      if (strcmp(buffer, portNumber) != 0)
      {
        send(connectionSocket, "-1", strlen("-1"), 0);
        exit(2);

      } else {

        send(connectionSocket, "SYNACK", strlen("SYNACK"), 0);

        recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
        send(connectionSocket, "ACK", strlen("ACK"), 0);



        /* ************************************ */
        /*                                      */
        /*        Receive Plaintext File        */
        /*                                      */
        /* ************************************ */
        resetBytesReceived();
        buffer_length = atoi(buffer);

        while (totalReceived < buffer_length) 
        {
          bytesReceived = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);       /* Receive the plaintext from the client */
          
          if (bytesReceived < 0) {

            error("ERROR reading plaintext from socket");                              /* Print error if threre was an issue getting plaintext from client */
          }

          for (int i = 0; i < bytesReceived; i++) {
             plaintext[totalReceived + i] = buffer[i];
          }

          totalReceived += bytesReceived;
        }

        //printf("Received plaintext\n");


        /* ************************************* */
        /*                                       */
        /*           Receive Key File            */
        /*                                       */
        /* ************************************* */

        resetBytesReceived();
        totalReceived = 0;
        bytesReceived = 0;
        //printf("Now reading in key\n");

        while (totalReceived < buffer_length)
        {
          bytesReceived = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);
          
          if (bytesReceived < 0) {

            error("ERROR reading key from socket");

          }

          for (int i = 0; i < bytesReceived; i++) {
            
            key[totalReceived + i] = buffer[i];
          }

          totalReceived += bytesReceived;

        }

        //printf("Received key\n");

        /* ************************************** */
        /*                                        */
        /*          Encrypt Plaintext             */
        /*                                        */
        /* ************************************** */

        //printf("Now encrypting plaintext\n");
        encrypt(ciphertext, plaintext, key);                                            /* Encrypt the plaintext with the key and save it to ciphertext */

      
        /* ************************************** */
        /*                                        */
        /*          Sending Ciphertext            */
        /*                                        */
        /* ************************************** */
        //printf("Sending ciphertext to client\n");
        resetBytesReceived();                                                           /* Reset count */

        while (charsRead < buffer_length) 
        {
          charsRead += send(connectionSocket, ciphertext, sizeof(ciphertext), 0);       /* Send the ciphertext back to client */
        }

        if (charsRead < 0) {

          error("ERROR writing ciphertext to socket");                                  /* Print error if there was an issue sending the ciphertext back to client */

        }
      
        exit(0);
        }
      }

    close(connectionSocket);                                                            /* Close the connection socket for the child process */
    while(waitpid(-1, NULL, WNOHANG) > 0);                                              /* Clean up zombie processes */
  }
  
  close(listenSocket);                                                                  /* Close the listening socket */

  return 0;
}

