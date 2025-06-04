
## Version 0.2.0 - Cleared warnings and errors
- Had errors preventing from running the program
- Cleared warning about 'linker' done in the makefile
- Made chenages to clear errors on:
    - buffmgmnt.c
    - buffmgmnt.h
    - helperfuncs.c
    - Makefile
    - rcopyCBuffer.h
    - server.c
- Computer is crashing (not sure why)
    - I had checked if i'm forking right and it looks good (mystery)

## Version 0.1.8
- Bug cleanup


## Version 0.1.7
- rcopy buffer done
- FSM
- server
- rcopy helper funcs
- networks
- link error on local computer not allowing me to test

## Version 0.1.6
- Saving progress

## Version 0.1.5 - Finished most server logic
- Created helperfuncs.c and .h to call func that creates PDUs on both server and rcopy
- Has not been tested
- new functions in server and windowBuffer
- ran make and came back error-free
- testing and doing rcopy next

## Version 0.1.4 - Pushing current progress
- Deleted any TCP stuff from networks.c and .h
- uncommented lines for using sendtoErr() library
- error rate on server command line args
- added necessary poll functions on server main
- sliding window control for server is on server file
- create PDU and reading from file done on server
- server is incomplete
- circular queue's need modifying

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
