Nume: Nastase Maria
Grupă: 335CC

# Tema <2>

Organizare
-
1. Minimal implementation of stdio.h library, generating a dynamic library. The project implements the SO_FILE structure alongside the primary functions of the library.

Implementare
- The functions not implemented are: so_popen and so_pclose.
- The functions so_fgetc and so_fputc were the most difficult to implemnent, but once they were done, the tasks were simple.
- so_fgetc is implemented with a helper function, if the buffer is empty it will read BUFFERSIZE characters and reset the read index to 0, being further incremented while reading.
- there is a flag set to keep track of the last operation made.
- so_fputc is implemented with a helper function, when the buffer is full, write the BUFFERSIZE and reset the buffer and index.
- so_fread and so_fwrite goes through nmemb elements each of size 'size', checks for errors and eof and saves the current index in buffer.
- so_fflush writes in the file the content of the buffer if the last operation is write and resets the buffer.
- I enjoyed doing the homework since I had the opportunity to better understand how I/O works.

Cum se compilează și cum se rulează?
- so_stdio.c so_stdio.h link into teh dynamic library libso.stdio.so

Bibliografie
- https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
- https://www.tutorialspoint.com/
- https://linux.die.net/man/


Git
- https://github.com/marianastase0912/Tema2-SO/
