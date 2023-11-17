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


