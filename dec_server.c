// Author: Steven Bertolucci
// Course: Operating System I
// Assignment: otp
// File: dec_server.c
// Description: 
// --------------------------------------------------------------------------------------------------------
//   This program performs exactly like enc_server, in syntax and usage. In this case, however, dec_server 
//   will decrypt ciphertext it is given, using the passed-in ciphertext and key. Thus, it returns plaintext 
//   again to dec_client.
// --------------------------------------------------------------------------------------------------------

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
decrypt(char* plaintext, const char* ciphertext, const char* key)                           /* Decrypt the message */
{
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

      value = ((c - k) % 27);

     if (value < 0) {

       value += 27;

     }

     if (value == 26) {                                                                     /* Handle value 26 as a space */

        plaintext[i] = ' ';

     } else {

      value += 65;
      plaintext[i] = value;

	  }
   }
  }

	plaintext[i] = '\0';                                                                      /* Append null terminator to the last byte */
}                                                                      


void
resetBytesReceived() 
{

  charsRead = 0;                                                                            /* Reset bytes received */

}



int main(int argc, char *argv[]) {

  int listenSocket, connectionSocket, buffer_length;
  char* portNumber = "62311";
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

    if (connectionSocket < 0){                                                          /* Check for connection error */

      error("ERROR on accept");                                                         /* Print error message if there were accepting issues */

    }

  //  printf("13. SERVER: Connected to client running at host %d port %d\n", 
    //       ntohs(clientAddress.sin_addr.s_addr),
      //     ntohs(clientAddress.sin_port));

    pid_t pid = fork();                                                                 /* Fork a process to handle multiple connections */

    if (pid == -1) {

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

      resetBytesReceived();

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
        /*        Receive Ciphertext File       */
        /*                                      */
        /* ************************************ */
        buffer_length = atoi(buffer);

        while (totalReceived < buffer_length) 
        {
          bytesReceived = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);       /* Receive the ciphertext from the client */

          if (bytesReceived < 0) {

            error("ERROR reading ciphertext from socket");                              /* Error message */

          }

          for (int i = 0; i < bytesReceived; i++) {

            ciphertext[totalReceived + i] = buffer[i];                                  /* Save buffer content to ciphertext */

          }

          totalReceived += bytesReceived;              

        }

        //printf("Received ciphertext\n");


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
          bytesReceived = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);       /* Receive the key from the client */

          if (bytesReceived < 0) {

            error("ERROR reading key from socket");                                     /* Print error message */

          }

          for (int i = 0; i < bytesReceived; i++) {

            key[totalReceived + i] = buffer[i];                                         /* Save buffer content to key */

          }

          totalReceived += bytesReceived;

        }

        // printf("Received key\n");

        /* ************************************** */
        /*                                        */
        /*          Decrypt Ciphertext            */
        /*                                        */
        /* ************************************** */

        //printf("Now decrypting ciphertext\n");
        decrypt(plaintext, ciphertext, key);                                            /* Decrypt the ciphertext with the key and save it to plaintext */

      
        /* ************************************** */
        /*                                        */
        /*          Sending Plaintext             */
        /*                                        */
        /* ************************************** */
        //printf("Sending plaintext to client\n");
        //clearBuffer(buffer);
        resetBytesReceived();                                                           /* Reset count */

        while (charsRead < buffer_length) 
        {
          charsRead += send(connectionSocket, plaintext, sizeof(plaintext), 0);         /* Send the plaintext back to client */
        }

        if (charsRead < 0) {

          error("ERROR writing plaintext to socket");                                   /* Print error if there was an issue sending the plaintext back to client */

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


