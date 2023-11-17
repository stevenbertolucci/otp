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


