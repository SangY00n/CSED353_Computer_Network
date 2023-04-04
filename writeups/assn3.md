Assignment 3 Writeup
=============

My name: Oh SangYoon

My POVIS ID: dbs1367

My student ID (numeric): 20200220

This assignment took me about [8] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the TCPSender:
I implemented the class which is named RetransmissionTimer to check retransmission timeout. It has some necessary public interface such as tick(), start(), stop(), etc. start() and stop() is called in order to start or stop the retransmission timer, tick() is called when check the retransmission timer has expired and return the value of boolean type which indicates whether the timer has expired or not. The TCPSender has RetransmissionTimer object as its retransmission timer and use that for managing RTO.
I added some member variables in TCPSender to store the information which need to be counted and the information from the receiver such as the receiver's ackno, advertised window size. And also added the queue of TCPSegments for keeping track of the outstanding segments until the sequence numbers they occupy have been fully acknowledged. When fill_window() send a segment by pushing it to the _segments_out queue, it also pushes the segment to the outstanding segments' queue to store it until it have been acknowledged.

Implementation Challenges:
It's difficult to decide when to send the segment which has FIN flag. At first, I decided to send the segment which has FIN flag when the stream has reach EOF. But there was a chance that there was no remaining window space left for FIN flag. So I decide to consider not only whether the stream has reach EOF or not but also whether the window space left or not.
When implementing ack_received() of TCPSender class, I didn't consider the case where the remote receiver's acknowledgement number is bigger then the sender's next sequence number, which needs to be ignored. Fortunately, since there was a failed test caused by this mistake, I was able to found the problem.

Remaining Bugs:
I hopefully believe there is no remaning bugs. At least I cannot see any bugs now.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
