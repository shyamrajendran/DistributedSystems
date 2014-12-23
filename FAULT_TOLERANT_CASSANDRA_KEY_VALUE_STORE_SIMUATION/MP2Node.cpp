/**********************************
 * FILE NAME: MP2Node.cpp
 *
 * DESCRIPTION: MP2Node class definition
 **********************************/
#include <iterator>
#include "MP2Node.h"

/**
 * constructor
 */

MP2Node::MP2Node(Member *memberNode, Params *par, EmulNet * emulNet, Log * log, Address * address) {
	this->memberNode = memberNode;
	this->par = par;
	this->emulNet = emulNet;
	this->log = log;
	ht = new HashTable();
	this->memberNode->addr = *address;
	transactionID=0;
}

/**
 * Destructor
 */
MP2Node::~MP2Node() {
	delete ht;
	delete memberNode;
}

/**
 * FUNCTION NAME: updateRing
 *
 * DESCRIPTION: This function does the following:
 * 				1) Gets the current membership list from the Membership Protocol (MP1Node)
 * 				   The membership list is returned as a vector of Nodes. See Node class in Node.h
 * 				2) Constructs the ring based on the membership list
 * 				3) Calls the Stabilization Protocol
 */
void MP2Node::updateRing() {

	vector<Node> curMemList;
	curMemList = getMembershipList();
	// Sort the list based on the hashCode
	sort(curMemList.begin(), curMemList.end());

	if ( ring.size() == 0 ){
		ring=curMemList;
		return;
	}
	if (curMemList.size() != ring.size()){
		stabilizationProtocol();
	}
	ring=curMemList;
}

/**
 * FUNCTION NAME: getMemberhipList
 *
 * DESCRIPTION: This function goes through the membership list from the Membership protocol/MP1 and
 * 				i) generates the hash code for each member
 * 				ii) populates the ring member in MP2Node class
 * 				It returns a vector of Nodes. Each element in the vector contain the following fields:
 * 				a) Address of the node
 * 				b) Hash code obtained by consistent hashing of the Address
 */
vector<Node> MP2Node::getMembershipList() {
	unsigned int i;
	vector<Node> curMemList;
	for ( i = 0 ; i < this->memberNode->memberList.size(); i++ ) {
		Address addressOfThisMember;
		int id = this->memberNode->memberList.at(i).getid();
		short port = this->memberNode->memberList.at(i).getport();
		memcpy(&addressOfThisMember.addr[0], &id, sizeof(int));
		memcpy(&addressOfThisMember.addr[4], &port, sizeof(short));
		curMemList.emplace_back(Node(addressOfThisMember));
	}
	return curMemList;
}

/**
 * FUNCTION NAME: hashFunction
 *
 * DESCRIPTION: This functions hashes the key and returns the position on the ring
 * 				HASH FUNCTION USED FOR CONSISTENT HASHING
 *
 * RETURNS:
 * size_t position on the ring
 */
size_t MP2Node::hashFunction(string key) {
	std::hash<string> hashFunc;
	size_t ret = hashFunc(key);
	return ret%RING_SIZE;
}

/**
 * FUNCTION NAME: clientCreate
 *
 * DESCRIPTION: client side CREATE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientCreate(string key, string value) {
	/*
	 * Implement this
	 */
	 	int iterCount = 1;
	 	incrTransactionID();
		int transID = getTransactionID();
		Address myNode=getMemberNode()->addr;
		vector<Node> nodeList= findNodes(key);
		pendingListStruct p;
	 	p.msgType=CREATE;
		p.ackCount=0;
		p.successAcks=0;
		p.key=key;
		p.value=value;
		p.timeStamp=par->getcurrtime();
		pendingTableMap[transID] = p;

		cout << "SAVED INTO PENDING TABLE : TRANSID: KEY : VALUE "<< transID<<":" << key <<":"<< value << endl;

		 //char *msg;
		 ReplicaType rtype;
		 MessageType msgType=CREATE;
		 for(vector<Node>::iterator it=nodeList.begin();it!=nodeList.end();++it){

			 Address sendAddress=it->nodeAddress;

			 if ( iterCount == 1 )
				  rtype = PRIMARY;
			 if (iterCount == 2 )
				 rtype = SECONDARY;
			 else if ( iterCount == 3 )
				 rtype = TERTIARY;
			 Message m(transID,myNode, msgType, key, value, rtype );
		 	 string message = m.toString();
		 	 cout << "clientcreate message " << endl << message << endl;
		 	 size_t msgsize = message.length();
		 	 cout << "ADDRES MYNODE " << myNode.addr <<  "message before sending " << message << "DATASIZE " << msgsize << endl;
		 	 emulNet->ENsend(&myNode, &sendAddress, (char *)message.c_str(), msgsize);

		 	 iterCount++;

		 }
}

/**
 * FUNCTION NAME: clientRead
 *
 * DESCRIPTION: client side READ API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientRead(string key){
	/*
	 * Implement this
	 */
	 	int iterCount = 1;
	 	incrTransactionID();
		int transID = getTransactionID();
		Address myNode=getMemberNode()->addr;
		vector<Node> nodeList= findNodes(key);
		pendingListStruct p;
	 	p.msgType=READ;
		p.ackCount=0;
		p.successAcks=0;
		p.key=key;
		p.value="";
		p.timeStamp=par->getcurrtime();
		pendingTableMap[transID] = p;
	    MessageType msgType=READ;

		for(vector<Node>::iterator it=nodeList.begin();it!=nodeList.end();++it){
			 Address sendAddress=it->nodeAddress;
		     Message m(transID, myNode, msgType, key);
		 	 string message = m.toString();
		 	 cout << "client READ message " << endl << message << endl;
		 	 size_t msgsize = message.length();
		 	 cout << "CLIENTREAD SEDNING TO:FROM " << myNode.getAddress() << sendAddress.getAddress() << endl;
		 	 emulNet->ENsend(&myNode, &sendAddress, (char *)message.c_str(), msgsize);
		}
}

/**
 * FUNCTION NAME: clientUpdate
 *
 * DESCRIPTION: client side UPDATE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientUpdate(string key, string value){
	/*
		 * Implement this
		 */
		 	//int iterCount = 1;
		 	incrTransactionID();
			int transID = getTransactionID();
			Address myNode=getMemberNode()->addr;
			vector<Node> nodeList= findNodes(key);
			pendingListStruct p;
		 	p.msgType=UPDATE;
			p.ackCount=0;
			p.successAcks=0;
			p.key=key;
			p.value=value;
			p.timeStamp=par->getcurrtime();
			pendingTableMap[transID] = p;

			cout << "SAVED INTO PENDING TABLE : TRANSID: KEY : VALUE "<< transID<<":" << key <<":"<< value << endl;

			 //char *msg;
			 ReplicaType rtype;
			 MessageType msgType=UPDATE;
			 for(vector<Node>::iterator it=nodeList.begin();it!=nodeList.end();++it){
				 Address sendAddress=it->nodeAddress;
				 Message m(transID,myNode, msgType, key, value, rtype );
			 	 string message = m.toString();
			 	 cout << "client update message " << endl << message << endl;
			 	 size_t msgsize = message.length();
			 	 cout << "ADDRES MYNODE " << myNode.addr <<  "message before sending " << message << "DATASIZE " << msgsize << endl;
			 	 emulNet->ENsend(&myNode, &sendAddress, (char *)message.c_str(), msgsize);

			 }

}

/**
 * FUNCTION NAME: clientDelete
 *
 * DESCRIPTION: client side DELETE API
 * 				The function does the following:
 * 				1) Constructs the message
 * 				2) Finds the replicas of this key
 * 				3) Sends a message to the replica
 */
void MP2Node::clientDelete(string key){

	 	incrTransactionID();
		int transID = getTransactionID();

		Address myNode=getMemberNode()->addr;

		vector<Node> nodeList= findNodes(key);
		pendingListStruct p;
	 	p.msgType=DELETE;
		p.ackCount=0;
		p.successAcks=0;
		p.key=key;
		p.value="";
		p.timeStamp=par->getcurrtime();
		pendingTableMap[transID] = p;

		//loop through and find and send to the key's nodes to delete
	    MessageType msgType=DELETE;
		for(vector<Node>::iterator it=nodeList.begin();it!=nodeList.end();++it){
			 Address sendAddress=it->nodeAddress;
		     Message m(transID, myNode, msgType, key);
		 	 string message = m.toString();
		 	 cout << "client DELETE  message " << endl << message << endl;
		 	 size_t msgsize = message.length();
		 	 emulNet->ENsend(&myNode, &sendAddress, (char *)message.c_str(), msgsize);
		}

}

/**
 * FUNCTION NAME: createKeyValue
 *
 * DESCRIPTION: Server side CREATE API
 * 			   	The function does the following:
 * 			   	1) Inserts key value into the local hash table
 * 			   	2) Return true or false based on success or failure
 */
bool MP2Node::createKeyValue(string key, string value, ReplicaType replica) {
	/*
	 * Implement this
	 */
	bool returnValue;
	int timeStamp=par->getcurrtime();
	Entry valueEntry(value, timeStamp, replica);
	string valueString = valueEntry.convertToString();
	// Insert key, value, replicaType into the hash table

	returnValue = ht->create(key, valueString);
	return returnValue;

}

/**
 * FUNCTION NAME: readKey
 *
 * DESCRIPTION: Server side READ API
 * 			    This function does the following:
 * 			    1) Read key from local hash table
 * 			    2) Return value
 */
string MP2Node::readKey(string key) {
	return ht->read(key);
}

/**
 * FUNCTION NAME: updateKeyValue
 *
 * DESCRIPTION: Server side UPDATE API
 * 				This function does the following:
 * 				1) Update the key to the new value in the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::updateKeyValue(string key, string value, ReplicaType replica) {

	bool returnValue;
	int timeStamp=par->getcurrtime();
	Entry valueEntry(value, timeStamp, replica);
	string valueString = valueEntry.convertToString();
	// Insert key, value, replicaType into the hash table
	returnValue = ht->update(key,valueString);
	return returnValue;

}

/**
 * FUNCTION NAME: deleteKey
 *
 * DESCRIPTION: Server side DELETE API
 * 				This function does the following:
 * 				1) Delete the key from the local hash table
 * 				2) Return true or false based on success or failure
 */
bool MP2Node::deletekey(string key) {

	return ht->deleteKey(key);
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: This function is the message handler of this node.
 * 				This function does the following:
 * 				1) Pops messages from the queue
 * 				2) Handles the messages according to message types
 */
void MP2Node::checkMessages() {

	char * data;
	int size;
	map<int,pendingListStruct>::iterator it;
	Address myAddress = memberNode->addr;
	// code to check timeout of a pending message and log failure and delete the item from pending list
	for (it = pendingTableMap.begin(); it != pendingTableMap.end(); it++)
	{
		if ( par->getcurrtime()- it->second.timeStamp > 6 ){
			// failure due to non delivery of REPLY
			switch(it->second.msgType){
			case CREATE:
			{
				log->logCreateFail(&myAddress, true, it->first, it->second.key, it->second.value);
				break;
			}
			case READ:
			{
				log->logReadFail(&myAddress, true, it->first, it->second.key);
				break;
			}
			case UPDATE:
			{
				log->logUpdateFail(&myAddress, true, it->first, it->second.key, it->second.value);
				break;
			}
			case DELETE:
			{
				log->logDeleteFail(&myAddress, true, it->first, it->second.key);
				break;
			}
			}
			pendingTableMap.erase (it);
		}

	}

	while ( !memberNode->mp2q.empty() ) {

		data = (char *)memberNode->mp2q.front().elt;
		size = memberNode->mp2q.front().size;
		memberNode->mp2q.pop();
		Address myAddress = memberNode->addr;
		string message(data, data + size);
		cout << " WITHIN >" << myAddress.addr << "<INSIDE CHECK MESSAGE:"<< message  << endl;
		Message msg(message);

		string key = msg.key;
		string value = msg.value;
		ReplicaType replica = msg.replica;
		Address sendAddress = msg.fromAddr;
		int transID = msg.transID;
		string valueRead;

		switch (msg.type){

		case UPDATE:
		{
			if ( updateKeyValue(key,value,replica)) {
						cout << "logging inside UPDATE " << value << endl;
						log->logUpdateSuccess(&myAddress, false, transID, key, value);

						MessageType replyMsgType=REPLY;
						Message replyMsg(transID, myAddress, replyMsgType, true);

						string replyMsgString = replyMsg.toString();
						cout << "REPLY MESSAGE " << replyMsgString << endl;
						emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());

					} else {

						log->logUpdateFail(&myAddress, false, transID, key, value);
						MessageType replyMsgType=REPLY;
						Message replyMsg(transID, myAddress, replyMsgType, false);
						string replyMsgString = replyMsg.toString();
						cout << "REPLY MESSAGE TO UPDATE " << replyMsgString << endl;
						emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());
					}
					break;
		}
		case READ:
		{
			valueRead=readKey(key);
			if ( ! valueRead.empty() ){

				Entry valueEntry(valueRead);
				cout << "logging inside READ" << value << endl;
				log->logReadSuccess(&myAddress, false, transID, key, valueEntry.value);


				Message replyMsg(transID, myAddress,valueRead);
				string replyMsgString = replyMsg.toString();
				cout << "REPLY MESSAGE " << replyMsgString << endl;
				emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());

			} else {
				log->logReadFail(&myAddress, false, transID, key);
				Message replyMsg(transID, myAddress,valueRead);
				string replyMsgString = replyMsg.toString();
				cout << "REPLY MESSAGE TO READ FAILURE " << replyMsgString << endl;
				emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());

			}
			break;
		}

		case READREPLY:
		{
			cout << " WITHIN READ REPLY >" << myAddress.addr << "<INSIDE CHECK MESSAGE:"<< message  << endl;

			map<int,pendingListStruct>::iterator search = pendingTableMap.find(transID);
			Message receivedReply(message);

			if(search != pendingTableMap.end()) {

				cout << "SUCCESS ACK COUNT" << search->second.successAcks<<endl;


				if (receivedReply.value != "" ){
					search->second.successAcks++;
				}
				search->second.ackCount++;


				if (search->second.ackCount >= 2 && search->second.successAcks >= 2){

					Entry valueEntry(receivedReply.value);
					log->logReadSuccess(&myAddress, true, transID, search->second.key,valueEntry.value);
					pendingTableMap.erase (search);

				} else if ((search->second.ackCount == 2 && search->second.successAcks < 2) || ( par->getcurrtime() - search->second.timeStamp > 2)) {
					log->logReadFail(&myAddress, true, transID, search->second.key);
					pendingTableMap.erase (search);
				}

			}
			break;
		}

		case DELETE:
		{
			if (deletekey(key)){

				cout << "logging inside DELETE" << value << endl;
				log->logDeleteSuccess(&myAddress, false, transID, key);


				MessageType replyMsgType=REPLY;
				Message replyMsg(transID, myAddress, replyMsgType, true);

				string replyMsgString = replyMsg.toString();
				cout << "REPLY MESSAGE " << replyMsgString << endl;
				emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());

			} else {
				log->logDeleteFail(&myAddress, false, transID, key);
				MessageType replyMsgType=REPLY;
				Message replyMsg(transID, myAddress, replyMsgType, false);
				string replyMsgString = replyMsg.toString();
				cout << "REPLY MESSAGE TO DELETE " << replyMsgString << endl;
				emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());

			}
			break;
		}
		case CREATE:
		{
			if ( createKeyValue(key, value, replica )){

				//log->logCreateSuccess(myAddress, isCoordinator, transID,  key, value);
				cout << "logging inside CREATE " << value << endl;
				log->logCreateSuccess(&myAddress, false, transID, key, value);
				// also send reply

				MessageType replyMsgType=REPLY;
				Message replyMsg(transID, myAddress, replyMsgType, true);

				string replyMsgString = replyMsg.toString();
				cout << "REPLY MESSAGE " << replyMsgString << endl;
				emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());

			} else {

				log->logCreateFail(&myAddress, false, transID, key, value);
				// also send reply to coordinator
				MessageType replyMsgType=REPLY;
				Message replyMsg(transID, myAddress, replyMsgType, false);
				string replyMsgString = replyMsg.toString();
				cout << "REPLY MESSAGE TO CREATE " << replyMsgString << endl;
				emulNet->ENsend(&myAddress, &sendAddress, (char *) replyMsgString.c_str(),replyMsgString.length());

			}
			break;
		}
		case REPLY:
		{
				cout << " WITHIN REPLY >" << myAddress.addr << "<INSIDE CHECK MESSAGE:"<< message  << endl;
				map<int,pendingListStruct>::iterator search = pendingTableMap.find(transID);


				Message receivedReply(message);

				if(search != pendingTableMap.end()) {
					cout << "SUCCESS ACK COUNT" << search->second.successAcks<<endl;;
					if (receivedReply.success){

						search->second.successAcks++;
					}
					search->second.ackCount++;
					if (search->second.ackCount >= 2 && search->second.successAcks >= 2){

						if ( search->second.msgType == CREATE)
							log->logCreateSuccess(&myAddress, true, transID, search->second.key, search->second.value);
						else if ( search->second.msgType == DELETE)
							log->logDeleteSuccess(&myAddress, true, transID, search->second.key);
						else if ( search->second.msgType == UPDATE){
							//Entry valueEntry(search->second.value);
							log->logUpdateSuccess(&myAddress, true, transID, search->second.key, search->second.value);

						}

						pendingTableMap.erase (search);
					} else if ((search->second.ackCount >= 2 && search->second.successAcks < 2 ) || ( par->getcurrtime() - search->second.timeStamp > 2)) {
						if ( search->second.msgType == CREATE)
							log->logCreateFail(&myAddress, true, transID, search->second.key, search->second.value);
						else if (search->second.msgType == DELETE)
							log->logDeleteFail(&myAddress, true, transID, search->second.key);
						else if ( search->second.msgType == UPDATE){
							//Entry valueEntry(search->second.value);
							log->logUpdateFail(&myAddress, true, transID, search->second.key, search->second.value);
						}

						pendingTableMap.erase (search);
					}

				}
				break;
		}

		} /* switch end */

	} /* while end */

}
/**
 * FUNCTION NAME: findNodes
 *
 * DESCRIPTION: Find the replicas of the given keyfunction
 * 				This function is responsible for finding the replicas of a key
 */
vector<Node> MP2Node::findNodes(string key) {
	size_t pos = hashFunction(key);
	vector<Node> addr_vec;
	if (ring.size() >= 3) {
		// if pos <= min || pos > max, the leader is the min
		if (pos <= ring.at(0).getHashCode() || pos > ring.at(ring.size()-1).getHashCode()) {
			addr_vec.emplace_back(ring.at(0));
			addr_vec.emplace_back(ring.at(1));
			addr_vec.emplace_back(ring.at(2));
		}
		else {
			// go through the ring until pos <= node
			for (int i=1; i<ring.size(); i++){
				Node addr = ring.at(i);
				if (pos <= addr.getHashCode()) {
					addr_vec.emplace_back(addr);
					addr_vec.emplace_back(ring.at((i+1)%ring.size()));
					addr_vec.emplace_back(ring.at((i+2)%ring.size()));
					break;
				}
			}
		}
	}
	return addr_vec;
}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: Receive messages from EmulNet and push into the queue (mp2q)
 */
bool MP2Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
    	return emulNet->ENrecv(&(memberNode->addr), this->enqueueWrapper, NULL, 1, &(memberNode->mp2q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue of MP2Node
 */
int MP2Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}
/**
 * FUNCTION NAME: stabilizationProtocol
 *
 * DESCRIPTION: This runs the stabilization protocol in case of Node joins and leaves
 * 				It ensures that there always 3 copies of all keys in the DHT at all times
 * 				The function does the following:
 *				1) Ensures that there are three "CORRECT" replicas of all the keys in spite of failures and joins
 *				Note:- "CORRECT" replicas implies that every key is replicated in its two neighboring nodes in the ring
 */
void MP2Node::stabilizationProtocol() {
	vector<Node> curMemList;
	curMemList = getMembershipList();
	// Sort the list based on the hashCode
	sort(curMemList.begin(), curMemList.end());
	sort(ring.begin(), ring.end());
	string myAdd=memberNode->addr.getAddress();
	Address myNode=memberNode->addr;
	size_t myHash=hashFunction(memberNode->addr.addr)%RING_SIZE;
	cout << "MYADDR|HASH" << myAdd <<"|"<< myHash<< endl;
	size_t successorHash=0;
	size_t predecessorHash=0;
	size_t count;
	count=0;
	for(vector<Node>::iterator ringIterator=ring.begin(); ringIterator != ring.end();++ringIterator){
			if ( ringIterator->nodeHashCode == myHash ){
				cout << "FOUND YOUR PLACE IN RING" << endl ;
				if (count == 0 ){
					predecessorHash=ring.at(ring.size()-1).nodeHashCode;
				} else {
					ringIterator--;
					predecessorHash = ringIterator->nodeHashCode;
					ringIterator++;
				}

				if ( count == ring.size()-1){
					successorHash = ring.at(0).nodeHashCode;
				}else{
				ringIterator++;
				successorHash = ringIterator->nodeHashCode;
				ringIterator--;
				}
				//break;

			}
			cout << ringIterator->nodeAddress.getAddress() << "|"<< ringIterator->nodeHashCode << endl;
			count++;
		}

		vector<Node> v(10);
		vector<Node>::iterator it;

		cout << "SIZE OF RING | SIZE OF MEM" << ring.size()<< "|"<< curMemList.size() << endl;
		it=std::set_difference( ring.begin(), ring.end(), curMemList.begin(), curMemList.end(),v.begin());
		v.resize(it-v.begin());

		std::cout << "The difference has " << (v.size()) << " elements:\n";

		size_t failedNodeHash=0;
		for (it=v.begin(); it!=v.end(); ++it){
			cout << "printing difference";
			// list of failed nodes available here
			//memberNode->
			failedNodeHash = it->getHashCode();
			cout << "FAILED NODE HASH|" << failedNodeHash << endl;
			if (failedNodeHash == successorHash){
				// finding your position on the CURMEMLIST
				size_t  presentNodeToSendHash=0;
				Address presentNodeToSendHashAddress;
				size_t count=0;
				for(vector<Node>::iterator curMemIterator=curMemList.begin(); curMemIterator != curMemList.end();++curMemIterator){
					if ( curMemIterator->nodeHashCode == myHash ){
						cout << "FOUND YOUR PLACE IN MEMLIST" << endl ;
						if ( count == curMemList.size()-1){
							presentNodeToSendHash = curMemList.at(1).nodeHashCode;
							presentNodeToSendHashAddress=curMemList.at(1).nodeAddress;
						} else if ( count == curMemList.size()-2){
							presentNodeToSendHash = curMemList.at(0).nodeHashCode;
							presentNodeToSendHashAddress=curMemList.at(0).nodeAddress;
						} else {
							curMemIterator++;
							curMemIterator++;
							presentNodeToSendHash = curMemIterator->nodeHashCode;
							presentNodeToSendHashAddress=curMemIterator->nodeAddress;
							break;
						}
					}
					cout << curMemIterator->nodeAddress.getAddress() << "|"<< curMemIterator->nodeHashCode << endl;
					count++;
				}


				map<string, string>::iterator it;

				MessageType msgType = CREATE;
				ReplicaType rtype = TERTIARY;
				int transID;
				size_t htSize=ht->hashTable.size();
					// code to check timeout of a pending message and log failure and delete the item from pending list
					for (it = ht->hashTable.begin(); it != ht->hashTable.end(); it++){
						incrTransactionID();
						transID=transactionID;
						cout << "key|value" << it->first << "|" << it->second ;

						cout << "SENDING THIS TO " << presentNodeToSendHashAddress.getAddress() << endl;
						Entry valueEntry(it->second);

						Message m(transID,myNode, msgType, it->first, valueEntry.value, rtype );
						string message = m.toString();
					    cout << "client create message " << endl << message << endl;
						size_t msgsize = message.length();
						cout << "ADDRES MYNODE " << myNode.addr <<  "message before sending " << message << "DATASIZE " << msgsize << endl;
						emulNet->ENsend(&myNode, &presentNodeToSendHashAddress, (char *)message.c_str(), msgsize);

					}
			} else if ( failedNodeHash == predecessorHash ){
					// no need to take care ; get taken care by the above condition on some other node.
			}

		 }
}

void MP2Node::incrTransactionID(){
	transactionID++;
}
int MP2Node::getTransactionID(){
	return transactionID;
}
