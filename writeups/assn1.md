Assignment 1 Writeup
=============

My name: Oh SangYoon

My POVIS ID: dbs1367

My student ID (numeric): 20200220

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

Program Structure and Design of the StreamReassembler:
I added some member variables within The StreamReassembler.
Two of them are vector array and they both are initialized with the capacity. _unassmebled_buffer store data which have to be reassembled and _occupied indiccates whether each index of buffer is occupied.
Others are for managing the buffer and reassembling the data segments. Especially, _is_eof_data_received is the flag for checking that last substring of the entire stream has been recieved.
Public interface functions perform like below.
1. push_substring()
	i. store the substring(data) in buffer as long as possible.
	ii. check that there are some of bytes thrown away from original data due to the finite capacity.
	iii. update the eof flag.
	iv. transfer reassembled data from the buffer to the output byte stream
	v. if all data are written into the output stream in right order, call end_input() of _output
2. unassembled_bytes() : return the number of bytes not yet assembled.
3. empty() : return 'true' if there is no unassembled substring and the output stream is empty.

Implementation Challenges:
It was hard to come up with a idea for managing data within finite storage. Choosing the suitable data structure for the reassembler was also difficult. At first, I chose string for the buffer and other variables and start to implement but I thought that was not efficient and leading to messy codes. So I chose vector for the implementation of buffer and I havea written the code more clearly and efficiently. I felt the importance of choosing a data structure once again.
And there were some exception cases that I couldn't predict, so I struggled with them. I was trying to consider all exception cases by drawing some sketches about possible situations. I think the sketches worked well.

Remaining Bugs:
I hopefully believe there is no remaning bugs. At least I cannot see any bugs now.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
