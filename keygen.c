// Author: Steven Bertolucci
// Course: Operating System I
// Assignment: otp
// File: keygen.c
// Due Date: March 17, 2024
// Description: 
// -----------------------------------------------------------------------------------------------
//   This program creates a key file of specified length. The characters in the file generated 
//   will be any of the 27 allowed characters, generated using the standard Unix randomization 
//   methods. Do not create spaces every five characters, as has been historically done. Note
//   that you specifically do not have to do any fancy random number generation: we’re not 
//   looking for cryptographically secure random number generation. rand()Links to an external 
//   site. is just fine. The last character keygen outputs should be a newline. Any error text 
//   must be output to stderr.
// -----------------------------------------------------------------------------------------------

/* ##################################################################################################### */
/* #                                                                                                   # */
/* #                               !! NOTICE OF REUSED CODE !!!                                        # */
/* #                                                                                                   # */
/* #                        I am reusing this keygen.c file from last quarter.                         # */
/* #                                                                                                   # */
/* #                                                                                                   # */
/* ##################################################################################################### */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

  int number = atoi(argv[1]);
  char key[number + 1];                         /* For null terminator */
  char* alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";  /* The accepted 26 characters plus space */
  int randomChar;                               /* For randomizing */

  if (argc != 2)                                /* If there are less than 2 arguments, exit */
  {
    printf("Usage: %s length\n", argv[0]);      /* Display error message */
    exit(EXIT_SUCCESS);
  }

  for (int i = 0; i < number; i++)              /* Randomize the key */
  {
    randomChar = rand() % 27;
    key[i] = alpha[randomChar];                 /* Randomize the characters */
  }

  key[number] = '\0';                           /* Add null terminator to the key */

  printf("%s\n", key);                          /* Print the key */                
  return 0;
}

