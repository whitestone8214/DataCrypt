#!/bin/bash

if (test "$1" = "build"); then
	rm -f datacrypt || exit 1
	gcc datacrypt.c $(pkg-config --cflags --libs libfile) -I/usr/include/mbedtls -lmbed{crypto,tls,x509} -o datacrypt || exit 1
	
elif (test "$1" = "install"); then
	cp -f datacrypt /bin || exit 1
	mkdir -p /usr/share/doc/datacrypt || exit 1
	cp -f readme.txt /usr/share/doc/datacrypt || exit 1
	
	rm -f taibu
	
elif (test "$1" = "remove"); then
	rm -rf /bin/datacrypt /usr/share/doc/datacrypt || exit 1
	
fi
