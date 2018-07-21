CSC361 - Fall 2017 <br />
Mantis Cheng <br />
Project 2 <br />

tfc.c - ./tfc ip port zip_file <br />
tfs.c - ./tfs ip port <br /> 

An exercise in file transferring through TCP sockets and file compression and decompression. 
* the client reads the given zip file and sends it to the server (zipped).
* the server recieves the zip file, unzips the file, and sends the file back to the client (unzipped).
* the client receives the file now unzipped.
* md5sum is used to verify the checksum, ensuring all bytes were received. 

