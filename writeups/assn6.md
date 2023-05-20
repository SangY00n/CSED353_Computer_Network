Assignment 6 Writeup
=============

My name: Oh SangYoon

My POVIS ID: dbs1367

My student ID (numeric): 20200220

This assignment took me about [4] hours to do (including the time on studying, designing, and writing the code).

If you used any part of best-submission codes, specify all the best-submission numbers that you used (e.g., 1, 2): []

- **Caution**: If you have no idea about above best-submission item, please refer the Assignment PDF for detailed description.

Program Structure and Design of the Router:
I added _routing_table to manage routes as a member variable of the Router class.
In add_route(), routes are added in descending order of prefix_length to the routing table.
route_one_datagram() traverses the routing table, finds a matched route, and sends datagram through the interface of the route.
Since the _routing_table is sorted in descending order of prefix_length, the first match will be the "longest-prefix match".

Implementation Challenges:
At first, I tried to sort the routing table by implementing route as a class, making the routing table through the route vector,
and using sort() of the algorithm library whenever a new route is pushed back to the routing table.
However, as I tried to use sort() while implementing Route as a class, errors related to operator overloading of the class occurred.
So, I implemented Route as a struct instead of a class, and instead of using sort(),
I changed it to traverse the routing table and insert the Route at the correct location.

Remaining Bugs:
I hopefully believe there is no remaning bugs. At least I cannot see any bugs now.

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]
