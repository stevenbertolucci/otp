#!/bin/bash
:set ft=make

otp: enc_server.c
	gcc -o enc_server enc_server.c
otp: enc_client.c
	gcc -o enc_client enc_client.c
otp: dec_server.c 
	gcc -o dec_server dec_server.c
otp: dec_client.c
	gcc -o dec_client dec_client.c
otp: keygen.c
	gcc -o keygen keygen.c
