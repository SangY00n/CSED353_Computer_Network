Assignment 2 Writeup
=============

My name: Oh SangYoon

My POVIS ID: dbs1367

My student ID (numeric): 20200220

This assignment took me about [6] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPReceiver and wrap/unwrap routines:
The member variable _isn was added to TCPReceiver class. _isn is pointing the number of Initial Sequence Number.
TCPReceiver class's public interface functions perform like below.
1. segment_received(const TCPSegment &seg)
	i. If _isn is null pointer, check TCP segment header's has SYN.
		a. If does, initialize Initial Sequence Number with TCP segment header's seqeunce number.
	ii. Else If _isn is not null pointer, calculate the absolute acknowledgement number using the total number of written bytes
		a. Calculate the absolute sequence number by calling unwrap function.
	iii. Push data(TCP segment's payload) into the stream.
2. ackno()
	i. If no SYN has been received, just return empty.
	ii. Calculate the acknowledgement number by calling wrap function with the absolute acknowledgement number and ISN.
3. window_size() : return window size. The window is a range of indexes the TCPReceiver is interested in, beginning with ackno.
I also Implemented wrap and unwrap functions.
1. wrap(uint64_t n, WrappingInt32 isn) : transform an absolute 64-bit sequence number into WrappingInt32.
2. unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) : transform a WrappingIn32 into an absolute 64-bit sequence number. There are ambiguous numbers for one WrappingInt32, so need to select the number which is closest to the checkpoint.

Implementation Challenges:
It was slightly complicated to select the abolute sequence number which is closest to the checkpoint in unwrap function. I struggled with selecting two closest numbers. To solve it, I distinguish the cases in which checkpoint's lowest 32-bit number is larger than absolute sequence number or not.
When implementing ackno() for TCPReceiver, I didn't care about last 1 byte for FIN. That resulted in 3 failed test. I could fix the problem with test failure message which mentioned about reported number and expected number.

Remaining Bugs:
I hopefully believe there is no remaning bugs. At least I cannot see any bugs now.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
