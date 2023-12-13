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
//
// Citation: Most of this implementation came from the provided replit: https://replit.com/@cs344/83serverc?lite=true#server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256


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


int 
convertChar (char c)                                                     /* Function to convert characters to integers */
{
	if (c == ' ')
  {
		return 26;                                                           /* End of file marker */
	} else {
		return (c - 'A');                                                    /* Replace the character with whatever value is subtracted */
	}

	return 0;
}


char 
convertInt (int i)                                                       /* Function to convert integers to characters */
{
	if (i == 26)                           
  {
		return ' ';                                                          /* Return space. */
	} else {
		return (i + 'A');
	}
}


void 
encrypt(char* message, char* key)                                        /* Encrypt the message */
{
	int i;
	char n;

	for (i = 0; message[i] != '\n'; i++)                                   /* Loop through each characters and encrypt them */
  {		
    n = (convertChar(message[i]) + convertChar(key[i])) % 27;        

	  message[i] = convertInt(n);                                          /* Save each encrypted characters */
	}

	  message[i] = '\0';                                                   /* Append null terminator to the last byte */

	  return;
}

void 
clearBuffer(char* buffer) {
    memset(buffer, '\0', BUFFER_SIZE);
}


int main(int argc, char *argv[])
{
	int connectionSocket, charsRead, status;
  char* unsuccessful = "-1";
  char* acknowledged = "ACK";
  char* synchronizedAck = "SYNACK";
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;
	pid_t pid;                                                           /* For concurrency */

	if (argc < 2)                                                        /* Check usage & args. If there are less than 2 arguments */
  { 
    fprintf(stderr, "USAGE: %s port\n", argv[0]); 
    exit(1);                                                           /* Exit failure */
  }

  setupAddressStruct(&serverAddress, atoi(argv[1]));                   /* Set up the address struct for the server socket */

	int listenSocket = socket(AF_INET, SOCK_STREAM, 0);                  /* Create the socket using TCP */

	if (listenSocket < 0)                                                /* Check for socket errors */
  {
    error("ERROR opening socket");
  }

	/* Bind the socket to begin listening */
	if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {       /* Associate the socket to the port */
		error("ERROR on binding");
  }

	while(1) {                                                          
		listen(listenSocket, 5);                                           /* It can now "listen" up to 5 connections */	


		sizeOfClientInfo = sizeof(clientAddress);                          /* Get the size of the client address */
		connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);       /* Accept the connection */

		if (connectionSocket < 0) {                                        /* Check for connection error */
      error("ERROR on accept");
    }

		
		pid = fork();                                                      /* Fork a process */	

	  if (pid == -1)
    {
		  error("Couldn't fork\n");                                        /* Error when forking */
		}

		if (pid == 0)
    {
		  char buffer[256];                                                /* Allocate the buffer */
			char message[71000];                                             /* Allocate the message */
			char key[71000];                                                 /* Allocate the key */
			int charsWritten = 0;

			clearBuffer(buffer);                                             /* Clear buffer array */
			charsRead = 0;

			while(charsRead == 0)
      {
				charsRead = recv(connectionSocket, buffer, 255, 0);            /* Receive the client's message */
      }

			if (charsRead < 0)                                               /* If nothing has been received, send error */
      { 
        error("ERROR reading from socket");
      }

		  if (strcmp(buffer, "Hello, from enc_client!") != 0)                             /* Send response to client */
      {

				charsRead = send(connectionSocket, unsuccessful, strlen(unsuccessful), 0);    /* The response to be sent (-1) because it failed to connect */
				exit(2);                                                                      /* Exit on status 2 per assignment requirement */
          
			} else {

				clearBuffer(buffer);                                                          /* Clear the buffer */
				charsRead = send(connectionSocket, acknowledged, strlen(acknowledged), 0);    /* Send response, "acknowledged" to the client */
				charsRead = 0;
          
				while (charsRead == 0)                                                        /* For receiving the file */                                    
        {	
          charsRead = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);          /* Receive the file from the client */
				}

				int size_buffer = atoi(buffer);                                               /* Get buffer length */

				charsRead = send(connectionSocket, synchronizedAck, strlen(synchronizedAck), 0);  /* Send SYNACK to the client that the files have been received */
				charsRead = 0;
				int charsSent = 0;
					
				while (charsRead < size_buffer)
        {
					clearBuffer(buffer);                                                        /* Clear the buffer */
					charsSent = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);          /* Receive the response */
					charsRead += charsSent;
					charsSent = 0;
					strcat(message, buffer);                                                    /* Concatenate the buffer into message */
					clearBuffer(buffer);                                                        /* Clear the buffer */
				}

				charsRead = 0;                                                                /* Reset for receving messages */
				charsSent = 0;                                                                /* Reset for sending messages */

				while (charsRead < size_buffer)
        {
					clearBuffer(buffer);                                                        /* Clear the buffer */
					charsSent = recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);          /* Receive the response */
					charsRead += charsSent;
					charsSent = 0;
					strcat(key, buffer);                                                        /* Concatentate the key from the buffer */
					clearBuffer(buffer);                                                        /* Clear the buffer */
				}

				encrypt(message, key);                                                        /* Encrypt the message */
				clearBuffer(buffer);                                                          /* Clear the buffer */

				charsWritten = 0;

				while (charsWritten < size_buffer)
        {
					clearBuffer(buffer);                                                        /* Clear out the buffer */
					charsWritten += send(connectionSocket, message, sizeof(message), 0);        /* Send message to client */
					clearBuffer(buffer);                                                        /* Clear out the buffer */						
				}	

				exit(0);

			}
		}

		if (pid == 1)
    {
		  waitpid(pid, &status, WNOHANG);                                                 /* Wait for the child process to finish */
		}
		

		close(connectionSocket);                                                          /* Close the conection socket for this client */
                                    
	}

	close(listenSocket);                                                                /* Close the listening socket */

	return 0;
}


