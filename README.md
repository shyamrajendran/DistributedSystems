DistributedSystems
==================
A simulation of grep in distributed systems. 
Client can query for logs by logging into any server. Distributed grep will search for logs across the pool of servers and return 
the response to the client. 

distributedWorkAllocator.py
==================
This is invoked by the executable which sends the query to the pool of servers

bash mygrep <search>


remoteWorker.py
==================
The is run at every server in the server pool. 
python remoteWorker.py &
