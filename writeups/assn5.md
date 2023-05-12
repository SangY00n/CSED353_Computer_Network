Assignment 5 Writeup
=============

My name: Oh SangYoon

My POVIS ID: dbs1367

My student ID (numeric): 20200220

This assignment took me about [6] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the NetworkInterface:
First of all, to cache the sender's IP address and Ethernet address, and to record the time taken after caching, I added a map called _ethernet_addr_cache with the IP address as the key using the ordered_map container. And I added a list_queuing_IP_datagrams to keep the IP datagrams that should be sent to the destination when the destination Ethernet address is not cached. I had to send ARP requests only once every 5 seconds for a specific IP address. Therefore, I also added a _arp_timer map with an IP address as a key to record the time it took after the ARP request for each IP address was sent.
In the tick() function, it accesses the maps added earlier, updating the value that records the time. And it deletes records that have been cached in _ethernet_addr_cache for more than 30 seconds.
The send_datagram() function explores whether there is a mapping for the current destination ip address in _ethernet_addr_cache, and immediately sends an IPv4 type frame when it finds the corresponding record. Otherwise, broadcast an ARP request to find the Ethernet address for that ip address and update _arp_timer and _queueing_IP_datagrams. The recv_datagram() function also updates _ethernet_addr_cache and _arp_timer, _queueing_IP_datagrams if the received frame is ARP.

Implementation Challenges:
In this assignment, it didn't take much time to design the code for each function. However, in the process of implementing as designed, I made some mistakes in the process of inspecting conditions and exploring containers. The mistakes took a lot of time.

Remaining Bugs:
I hopefully believe there is no remaning bugs. At least I cannot see any bugs now.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this assignment better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
