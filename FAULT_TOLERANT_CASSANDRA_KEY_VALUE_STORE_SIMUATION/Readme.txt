Simulation of Fault­tolerant distributed key­value store
This project is based on the https://github.com/bourneagain/DistributedSystems/tree/master/GOSSIP_PROTOCOL_SIMULATION distributed systems
system computer to computer communication system.

Fault Tolerant : 
The project uses replication factor of 3 ( SECONDARY,TERTIARY inaddition to the PRIMARY ) and is fault tolerant up to 2 nodes failure.

Key-value pairs are stored through the process of applying consistent hashing algorithm which evenly distibutes the load over the networks of servers connected over a virtual ring topology. 
Read / Write access to the system is provided through the selection of a random node ( coordinator ) to which the client requests and receives responses from. 
The system is designed to emulate Create Read Update and Delete operations.

How to run CRUD
$ make clean
$ make
$ ./Application ./testcases/create.conf
or 
$ ./Application ./testcases/delete.conf
or
$ ./Application ./testcases/read.conf
or
$ ./Application ./testcases/update.conf
