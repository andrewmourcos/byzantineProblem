# Byzantine Generals Oral Message Problem
The byzantine generals problem is a classic message passing problem that has 
its uses in technologies such as blockchain and other decentralized 
applications where device states need to be agreed upon.

If there are `m` traitor generals (or "broken devices"), then 3m+1 generals 
(total devices) must send messages for the OM algo to deterministically 
deduce the correct message.

This is an implementation written in C for an NXP microcontroller. It utilizes 
CMSIS RTOS to allow "generals" (threads) to communicate asynchronously via 
message queues. The Oral Message algo (OM) verifies the messages sent by each 
general to identify the deterministic "true" message even when some 
generals/threads are lying.
