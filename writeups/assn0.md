Assignment 0 Writeup
=============

My name: Oh SangYoon

My POVIS ID: dbs1367

My student ID (numeric): 20200220

This assignment took me about [7] hours to do (including the time on studying, designing, and writing the code).

My secret code from section 2.1 was: 42d89f1196

- Optional: I had unexpected difficulty with: Sponge's classes related to the socket such as Address and TCPSocket.
I didn't know that the bytes popped out from buffer by pop_output function also has to be considered the bytes_read. Because I just thought bytes_read is only about the bytes read by read function, I was dealing with eof flag not in pop_output function but in read function. 'make check_lab0', however, seems like it checks eof flag after calling pop_output function so I had to spend some time looking for what was wrong and fixing my functions.

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
