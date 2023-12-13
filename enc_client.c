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
//
// Citation: Most of the implementation came from this provided replit: https://replit.com/@cs344/83clientc?lite=true#client.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>                      /* ssize_t */
#include <sys/socket.h>                     /* send(), recv() */
#include <netinet/in.h>
#include <netdb.h>                          /* gethostbyname() */
#include <ctype.h>
#include <fcntl.h>                          /* For read/write */

#define BUFFER_SIZE 256

void 
error(const char *msg) {                    /* For error handling */
  perror(msg); 
  exit(0); 
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber, 
                        char* hostname) {

    memset((char*) address, '\0', sizeof(*address));                      /* Clear out the address struct */ 
    address->sin_family = AF_INET;                                        /* The address should be network capable */
    address->sin_port = htons(portNumber);                                /* Store the port number */

    struct hostent* hostInfo = gethostbyname(hostname);                   /* Get the DNS entry for this host name */

    if (hostInfo == NULL) {                                               /* If there is no host to connect */
        fprintf(stderr, "CLIENT: ERROR, no such host\n");                 /* Display the stderr */
        exit(0); 
    }

    memcpy((char*) &address->sin_addr.s_addr,                             /* Copy the first IP address from the DNS entry to sin_addr.s_addr */
           hostInfo->h_addr_list[0],
           hostInfo->h_length);
}

int number_of_characters (const char* filename) {                         /* Function to keep track of the characters */

	int file_content;
	int count = 0;
	FILE* file = fopen(filename, "r");                                      /* Open the file for reading */

    while (1) {
      file_content = fgetc(file);                                         /* Get the data */

      if (file_content == EOF || file_content == '\n')                    /* Break if end of file or newline at EOF */ 
      {
        break;
      }

		  if (!isupper(file_content) && file_content != ' ')
      {
			  error("Input contains bad characters!\n");                        /* If the file contains bad characters, display this message */
		  }

        count++;                                                          /* Increment the count of characters */
    }

	fclose(file);                                                           /* Close file */

	return count;
}

void clearBuffer(char* buffer) {
    memset(buffer, '\0', BUFFER_SIZE);                                    /* Clear buffer array */
}


int main(int argc, char *argv[])
{
	int socketFD, charsWritten, charsRead, file_descriptor;
  char* unsuccessful = "-1";
  char* synchronizedAck = "SYNACK";
	struct sockaddr_in serverAddress;

	char buffer[256];                                                     /* Allocate buffer length */
	char ciphertext[71000];                                               /* Allocate ciphertext length */

	if (argc < 3)                                                         /* Check usage and args */
  { 
    fprintf(stderr, "USAGE: %s hostname port\n", argv[0]);              /* Print to stderr if there are less than 3 args */
    exit(0);                                                            /* Exit Success (0) */
  }

  /* Set up the address struct for the server socket */
	setupAddressStruct(&serverAddress, atoi(argv[3]), "localhost");
  
	socketFD = socket(AF_INET, SOCK_STREAM, 0);                           /* Create the socket using TCP */

	if (socketFD < 0) {
    error("CLIENT: ERROR opening socket");                              /* Check for socket error */
  }

  /* Reference: https://beej.us/guide/bgnet/html/#setsockoptman */
	int yes = 1;
	setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));                        /* Make socket reusable */

	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {     /* Connect socket to the server */
		error("CLIENT: ERROR connecting");
	}

	int length_of_file = number_of_characters(argv[1]);                      /* Get length of the plaintext */
	int length_of_key = number_of_characters(argv[2]);                       /* Get length of the key */

	if (length_of_file > length_of_key) {                                    /* Check if the plaintext length is greater than the key length */
		fprintf(stderr, "Key is too short\n!");
		exit(1);
	}
	
	char* client_msg = "Hello, from enc_client!";                         /* Send confirmation message to server */
	charsWritten = send(socketFD, client_msg, strlen(client_msg), 0);     /* Sending message to server for confirmation */
	clearBuffer(buffer);                                                  /* Clear out the buffer array */

	if (charsWritten < 0) {                                               /* If no message has been received from the server */
		error("CLIENT: ERROR writing to socket");                           /* Print the error */
  }

	charsRead = 0;
	while(charsRead == 0) {                                               /* Get ack response from server */
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);          /* Read response from server */
  }

	if (charsRead < 0) {                                                  /* If nothing was returned from the server, print error message */
    error("CLIENT: ERROR reading from socket");
  }

	if (strcmp(buffer, unsuccessful) == 0) {                                                                 /* Check the connection */
		fprintf(stderr, "Failed. enc_client attempted to connect to dec_server port #: %d\n", atoi(argv[3]));  /* Error message */
		exit(2);                                                                                               /* Exit on status #2 per program requirement */
	}

	clearBuffer(buffer);                                                   /* Clear out the buffer array */
	sprintf(buffer, "%d", length_of_file);                                 /* Print the length of the buffer file */
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0);              /* Send the file length of the buffer */
	clearBuffer(buffer);                                                   /* Clear out the buffer array */

	charsRead = 0;
	while (charsRead == 0) {                                                
		charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);           /* Receive the response aka read the data from the socket */
  }
	
	if (strcmp(buffer, synchronizedAck) == 0) {                             
		file_descriptor = open(argv[1], 'r');                                /* Open the plaintext file */
		charsWritten = 0;

		while(charsWritten <= length_of_file) {
			clearBuffer(buffer);                                               /* Clear out the buffer array */
			int bytesRead = read(file_descriptor, buffer, sizeof(buffer) - 1); /* Gather the size of the file */              
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);         /* Send the buffer file to the server */
			clearBuffer(buffer);                                               /* Clear the buffer array */
		}

		file_descriptor = open(argv[2], 'r');                                /* Open the key file */
		charsWritten = 0;

		while(charsWritten <= length_of_file) {
			clearBuffer(buffer);                                               /* Clear out the buffer array */
			int bytesRead = read(file_descriptor, buffer, sizeof(buffer) - 1); /* Gather the size of the file */
			charsWritten += send(socketFD, buffer, strlen(buffer), 0);         /* Send the buffer file to server */
			clearBuffer(buffer);                                               /* Clear buffer array */
		}
	}

	clearBuffer(buffer);                                                   /* Clear out the buffer array */


	charsRead = 0;                                                         /* Resetted for receiving the message */ 
	int charsSent = 0;

	while (charsRead < length_of_file) {
		clearBuffer(buffer);                                                 /* Clear the buffer array */
		charsSent = recv(socketFD, buffer, sizeof(buffer)-1, 0);             /* Read data from socket */
		charsRead += charsSent;
		charsSent = 0;                                                      
		strcat(ciphertext, buffer);                                          /* Copy the content of the buffer to ciphertext by concatenating */
		clearBuffer(buffer);                                                 /* Clear the buffer array */
	}

	strcat(ciphertext, "\n");                                              /* Append new line to the ciphertext by concatenating */
	printf("%s", ciphertext);                                              /* Prints the ciphertext */
	close(socketFD);                                                       /* Close the socket */

	return 0;
}


