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
//
// Citation: Most of this implementation came from this provided replit: https://replit.com/@cs344/83clientc?lite=true#client.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>               /* ssize_t */
#include <sys/socket.h>              /* send(), recv() */
#include <netinet/in.h>
#include <netdb.h>                   /* gethostbyname() */
#include <ctype.h>
#include <fcntl.h>                   /* For read/write */

#define BUFFER_SIZE 256

void 
error (const char *msg)                                                                   /* Error function used for reporting issues */
{ 
  perror(msg); 
  exit(0); 
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname) {

    memset((char*) address, '\0', sizeof(*address));                                      /* Clear out the address struct */ 
    address->sin_family = AF_INET;                                                        /* The address should be network capable */
    address->sin_port = htons(portNumber);                                                /* Store the port number */

    struct hostent* hostInfo = gethostbyname(hostname);                                   /* Get the DNS entry for this host name */

    if (hostInfo == NULL) {                                                               /* If there is no host to connect */
        fprintf(stderr, "CLIENT: ERROR, no such host\n");                                 /* Display the stderr */
        exit(0); 
    }

    memcpy((char*) &address->sin_addr.s_addr,                                             /* Copy the first IP address from the DNS entry to sin_addr.s_addr */
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

int number_of_characters(const char* filename){                                           /* Function to keep track of the characters */

	int file_content;
	int count = 0;
	FILE* file = fopen(filename, "r");                                                      /* Open the file for reading */

    while (1) {
        file_content = fgetc(file);                                                       /* Get the data */

        if (file_content == EOF || file_content == '\n')                                  /* Break if end of file or newline at EOF */
        {
            break;
        }

		   if (!isupper(file_content) && file_content != ' ')
        {
          error("Input contains bad characters!\n");                                      /* If the file contains bad characters, display this message */
        }

        count++;                                                                          /* Increment the count of characters */
    }

	fclose(file);                                                                           /* Close the file */

	return count;
}

void clearBuffer(char* buffer) {
    memset(buffer, '\0', BUFFER_SIZE);
}


int main(int argc, char *argv[])
{
	int socketFD, charsWritten, charsRead, file_descriptor;
  char* unsuccessful = "-1";
  char* synchronizedAck = "SYNACK";
	struct sockaddr_in serverAddress;

	char buffer[256];                                                                       /* Allocate the buffer */
	char ciphertext[71000];                                                                 /* Allocate the ciphertext */

	if (argc < 3)                                                                           /* Check for usage & args. If there were less than 3 args, return an error */
  { 
    fprintf(stderr, "USAGE: %s hostname port\n", argv[0]); 
    exit(0); 
  }

  setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");                         /* Set up the address struct for the server socket */

	socketFD = socket(AF_INET, SOCK_STREAM, 0);                                             /* Create the socket using TCP */

	if (socketFD < 0)
  {
    error("CLIENT: ERROR opening socket");                                                /* Display error message */
  }

  /* Reference: https://beej.us/guide/bgnet/html/#setsockoptman */
	int yes = 1;                                                                            /* This was taken from the OTP assignment module */
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));                      /* Make socket reusable */
	
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)     /* Connect socket to server */
  {
		error("CLIENT: ERROR connecting");                                                    /* Error if connecting */
	}

	int length_of_file = number_of_characters(argv[1]);                                     /* Get length of plaintext */
	int length_of_key = number_of_characters(argv[2]);                                      /* Get length of key file */

  if(length_of_file > length_of_key)                                                      /* If length of string larger than the key length */
  {
    fprintf(stderr, "Key is too short\n");                                                /* Output error */
		exit(1);
	}
	
	char* client_msg = "Hello, from dec_client!";
	charsWritten = send(socketFD, client_msg, strlen(client_msg), 0);                       /* Send message to server */
	clearBuffer(buffer);                                                                    /* Clear out the buffer array */
                                        
	if (charsWritten < 0)                                                                   /* If no message was sent, return an error */
  {
		error("CLIENT: ERROR writing from socket");                                           /* Error message */
  }

	charsRead = 0;

	while (charsRead == 0)                                                                  /* If there was a confirmation from the server */
  {
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);                            /* Read the response from the server */
  }

	if (charsRead < 0)
  {
    error("CLIENT: ERROR reading from socket");                                           /* Display error message */
  }

	if (strcmp(buffer, unsuccessful) == 0)                                                                          /* Check the connection by comparing the response from the server */
  {
		fprintf(stderr, "Failed. dec_client attempted to connect to enc_server port #: %d\n", atoi(argv[3]));         /* Error when connecting */
		exit(2);                                                                                                      /* Exit on status 2 per assignment requirement */
	}


	clearBuffer(buffer);                                                                    /* Clear out the buffer array */
	sprintf(buffer, "%d", length_of_file);                                                  /* Print the length of the plaintext*/
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0);                               /* Send length of buffer to the server */
	clearBuffer(buffer);                                                                    /* Clear out the buffer array */

	charsRead = 0;

	while (charsRead == 0) 
  {
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);                            /* Receive the response from the server */
  }
	

	if (strcmp(buffer, synchronizedAck) == 0)                                               /* Process if I got a response from the server */
  { 
		file_descriptor = open(argv[1], 'r');                                                 /* Open the plaintext file for reading */
		charsWritten = 0;

		
		while (charsWritten <= length_of_file)                                                /* For sending the file text */
    {
			clearBuffer(buffer);                                                                /* Clear out the buffer aray */
			int bytesRead = read(file_descriptor, buffer, sizeof(buffer) - 1);                  /* Read the buffer */
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);                          /* Send the data to the server */
			clearBuffer(buffer);                                                                /* Clear the buffer array */
		}

		file_descriptor = open(argv[2], 'r');                                                 /* Open the key file for reading */
		charsWritten = 0;

		while (charsWritten <= length_of_file)
    {
			clearBuffer(buffer);                                                                /* Clear out the buffer array */
			int bytesRead = read(file_descriptor, buffer, sizeof(buffer) - 1);                  /* Read the buffer */
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);                          /* Send the data to the server */
			clearBuffer(buffer);                                                                /* Clear the buffer array */
		}
	}

	clearBuffer(buffer);                                                                    /* Clear out the buffer for reusability */

	charsRead = 0;                                                                          /* Reset the number of characters read */
	int charsSent = 0;


	while (charsRead < length_of_file)
  {
		clearBuffer(buffer);                                                                  /* Clear the buffer array */
		charsSent = recv(socketFD, buffer, sizeof(buffer) - 1, 0);                            /* Receive the response */
		charsRead += charsSent;
		charsSent = 0;
		strcat(ciphertext, buffer);                                                           /* Concatenate the message into ciphertext */
		clearBuffer(buffer);                                                                  /* Clear the buffer array */
	}

	strcat(ciphertext, "\n");                                                               /* Append the newline to the ciphertext */
	printf("%s", ciphertext);                                                               /* Display the ciphertext */
	close(socketFD);                                                                        /* Close the socket */

	return 0;
}


