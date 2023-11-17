# OTP (One-Time Pad)

## Introduction
In this assignment, I will be creating five small programs that encrypt and decrypt information using a one-time pad-like system. 
These programs will combine the multi-processing code you have been learning with socket-based inter-process communication. My 
programs will also be accessible from the command line using standard Unix features like input/output redirection, and job control.
Finally, you will write a short compilation script.

## Definitions
* **Plaintext**: The information that you wish to encrypt and protect. It is human readable.
* **Ciphertext**: Plaintext after it has been encrypted by your programs. Ciphertext is not human-readable, and if the OTP system is used correctly, cannot be cracked.
* **Key**: A random sequence of characters that will be used to convert Plaintext to Ciphertext, and back again. It must not be re-used, or else the encryption is in danger of being broken.

## Specifications
My program will encrypt and decrypt plaintext into ciphertext, using a key, in exactly the same fashion as above, except it will be using modulo 27 operations: 
your 27 characters are the 26 capital letters, and the space character. All 27 characters will be encrypted and decrypted as above.

To do this, I will be creating five small programs in C. Two of these will function as servers, and will be accessed using network sockets. Two will be clients, 
each one of these will use one of the servers to perform work, and the last program is a standalone utility.

My programs must use the API for network IPC that we have discussed in the class (`socket`, `connect`, `bind`, `listen`, & `accept` to establish connections; `send`, `recv` to send
and receive sequences of bytes) for the purposes of encryption and decryption by the appropriate servers. The whole point is to use the network, even though for testing purposes
we’re using the same machine to run all the programs: if I just `open` the datafiles from the server without using the network calls, I’ll receive 0 points on the assignment.
