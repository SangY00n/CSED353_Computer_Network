Assignment 4 Writeup
=============

My name: Oh SangYoon

My POVIS ID: dbs1367

My student ID (numeric): 20200220

This assignment took me about [9] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Your benchmark results (without reordering, with reordering): [0.42, 0.45]

Program Structure and Design of the TCPConnection:
Member variables such as _time_since_last_segment_received, _active, _is_fin_set, and _is_fin_acked were added to the TCPConnection class and used for implementation. _time_since_last_segment_received and _active are added to be used as the return value of an existing public function, and two member variables related to the FIN flag are added so that the TCPConnection class can use them to track the current state.
I implement the method of TCPConnection, flush_sender(). flush_sender() takes out one segment from segments_out until the TCPSender's segments_out is empty, fills the ackno, ack, and win fields in the segment, and pushes (sends) it to the outbound queue. It is named flush_sender because it plays the role of emptying the outbound queue of TCPSecder. Additionally, flush_sender() sets _is_fin_sent to record that a FIN segment was sent if there is a segment with the FIN flag set among the TCPSender's segments.
Also I implemented a function called send_rst(). send_rst() is a method called when an RST segment needs to be sent. When this function is called, it first sets the error flag for inbound and outbound ByteStreams, and sets _active to false to indicate that the connection is dead. Afterwards, all existing segments contained in TCPSender's segments_out are ignored, and a reset segment is created and sent.

Implementation Challenges:
I initially thought that the TCPConnection class should also track the SYN_SENT and SYN_ACKED states. So I added member variables to TCPConnection to keep track of them. However, as I completed the code, I realized that I didn't need to keep track of the two states. I even noticed that the connection was slowing down due to member variables to keep track of them, so I removed them.

Remaining Bugs:
When I run the test through the 'make check_lab4' command, I found that the 118th, 119th, 130th, and 131st tests sometimes fail. It seems to be influenced from outside, so I tried to fix the code as much as possible, but still one test fails occasionally.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
