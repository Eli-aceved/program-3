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
