## Version 0.1.3 - Circular Buffer Modifications
- Making changes on both circular buffers (server and rcopy) to be more compatible to the purpose of each buffer. 
- Prototypes for functions I'll be modifying were created
- Will be needing to use fread/fopen for server and fwrite/fopen for rcopy
- Would be a good idea to create a separate file for creating packets like for RR and SREJ maybe even just a regular data packet - call packetFactory
- renamed circularbuffer files to windowBuffer

## Version 0.1.2 - Client Buffer Created
- Tested rcopyCBuffer to see if retrieving packets and the circular aspect of the buffer works. 
- Testing passed. Currently data gets overrriden in the buffer, but program is designed to flush all packets before overriding data
- Created a .h for rcopyCBuffer
- All testing was done within the test folder where a separate makefile was used

## Version 0.1.1 - Created new files and worked on FSM
- server.c changes: checkArgs handles both command line arguments 
- Will renaming circularbuffer.c .h to serverWindowBuff (maybe) and reimplement the queue for the server
- Created buffmgmnt.c .h for FSM of buffer management on rcopy
    - The files are not yet complete and need more modification
- Imported more files that the Professor provided: networks, safeUtil, gethostbyname
- Created rcopyCBuffer which will be the circular queue for rcopy helping with buffer management

## Version 0.1.0 - Created Circular Buffer
- Created circular buffer .c and .h files
- Passed tests for enqueueing and dequeueing
