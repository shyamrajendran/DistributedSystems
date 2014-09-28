/**********************
*
* Progam Name: MP1. Membership Protocol
* 
* Code authors: <your name here>
*
* Current file: mp1_node.c
* About this file: Member Node Implementation
* 
***********************/

#include "mp1_node.h"
#include "emulnet.h"
#include "MPtemplate.h"
#include "log.h"
int NODECOUNT=0; // this is the location on the initiator's membership table saving each other node's information
int MEMCOUNT=0;  // this is to give global unique node number 

#define T_GOSSIP 1 // Here the nodeloopops is called for every global time. But we can control by setting this T_GOSSIP
#define T_FAIL 50  // Time after which the node marks the member as to be cleaned.
#define T_CLEANUP 100 // Time after which the node marks the member as deleted.
/*
 *
 * Routines for introducer and current time.
 *
 */

char NULLADDR[] = {0,0,0,0,0,0};
int isnulladdr( address *addr){
    return (memcmp(addr, NULLADDR, 6)==0?1:0);
}

/* 
Return the address of the introducer member. 
*/
address getjoinaddr(void){

    address joinaddr;

    memset(&joinaddr, 0, sizeof(address));
    *(int *)(&joinaddr.addr)=1;
    *(short *)(&joinaddr.addr[4])=0;

    return joinaddr;
}

/*
 *
 * Message Processing routines.
 *
 */

/* 
Received a JOINREQ (joinrequest) message.
*/
void Process_joinreq(void *env, char *data, int size)
{

	
   	// This function is only called by the initiator for messages sent from different other nodes in the network.
	// copy message into the membership list and send joinrep message type to the node from where it received the message
	// env is the node on which it is called 
	// data only contains the address of the sender


    member *node = (member *) env;
    address *destaddr= (address *)data; // typecast to address to directly get the dest address
	NODECOUNT++;


	// update the membership table

	node->memtable[node->memcount].lastupdatetime=getcurrtime();
	memcpy(&node->memtable[node->memcount].maddr,destaddr,sizeof(address));
	node->memcount++;

	logNodeAdd(&node->addr,destaddr);


	//create the message containing membership table and send to the sender's node
	messagehdr *msg;
    size_t msgsize = sizeof(messagehdr) + ((NODECOUNT+1) * sizeof(membershiplist));
    msg=malloc(msgsize);
	msg->msgtype=JOINREP;
	messagehdr *src=msg;
	src=src+1;
	//src=msg+1;
	// do a for loop to copy each membership array struct into message 
	int i;

	for(i=0;i<node->memcount;i++){
		//printf("\n COPYING for  %d",node->memtable[i].maddr.addr[0] );

    	memcpy((membershiplist *)(src)+i, &node->memtable[i], sizeof(membershiplist));

	}

   	MPp2psend(&node->addr, destaddr, (char *)msg, msgsize);

    return;
}
/* 
Received a GOSSIP message. 
*/
void nodeloopops(member *node){

	printf("\n ** INSIDE NODELOOPOPS %d",node->addr.addr[0]);

	if ( getcurrtime() % T_GOSSIP == 0 && node->memcount !=0 ){

		// sending a gossip message by selecting a random node which is not its own
		int n=-1;
		int randomNode=-1;

		// to select random node to send the gossip message
		while(1){
			randomNode = rand() % EN_GPSZ ;
			if ( randomNode == node->nodenumber)
				continue;
			else if ( randomNode > node->memcount)
				continue;
			else if (memcmp(&node->addr,&node->memtable[randomNode].maddr,sizeof(address))==0)
				continue;
			else if (node->memtable[randomNode].tocleanup!=0)
				continue;
			else
				break;

		}

		// when you found the node to send , loop through the  memberlisttable and update your lastupdatetime.
		int i=0;

		for(i=0;i<node->memcount;i++){
			printf("\n check time for %d",node->memtable[i].maddr.addr[0]);
				// check if the mem table contains the node's address
				n = memcmp ( &node->memtable[i].maddr, &node->addr,sizeof(address));
				if ( n == 0 ){
					// update current time for ur node
					printf("\nupdating the getcurrent time");
					node->memtable[i].lastupdatetime=getcurrtime();

				}else{
					if ( getcurrtime() - node->memtable[i].lastupdatetime > T_CLEANUP  ){
						if ( getcurrtime() - node->memtable[i].lastupdatetime < T_FAIL ) {
							printf("\nTime to set del bit for node %d",node->memtable[i].maddr.addr[0]);
							node->memtable[i].tocleanup=1;
						} else {
							if ( node->memtable[i].tocleanup != 2 ){
								printf("\nTime to mark the node deleted %d",node->memtable[i].maddr.addr[0]);
								node->memtable[i].tocleanup=2;

								logNodeRemove(&node->addr,&node->memtable[i].maddr);
							}
						}
					}

				}
		}

		//create the message containing membership table and send to the sender's node
		messagehdr *msg;
		size_t msgsize = sizeof(messagehdr) + sizeof(address) +(node->memcount) * sizeof(membershiplist);
		msg=malloc(msgsize);
		msg->msgtype=GOSSIPMSG;

		membershiplist *src;
		struct address *temp;
		temp=(struct address*) (msg+1);
		memcpy(temp, &node->addr,sizeof(address));
		src=(membershiplist *)(temp + 1);

		for(i=0;i<node->memcount;i++){
			memcpy(src, &node->memtable[i],sizeof(membershiplist));
			src=src + 1;
		}
    	MPp2psend(&node->addr, &node->memtable[randomNode].maddr, (char *)msg, msgsize);


		// constructing the message to send to random node with message type GOSSIP
		printf("\n Number of records in %d mem table is %d ",node->addr.addr[0],--i);
	}

	printf("\n *** NODELOOP DONE *** ");

    return;
}

void Process_gossipmsg(void *env, char *data, int size)
{
	// This process is called for every node when it receives a message of type GOSSIP
	// once received, update the current time for its own entry in the member ship table
	// Check for the data received ( a list of membership tables ) and then update its own node's membership table if the current time stamp is more for that specific node

	member *node = (member *) env;
	address *sourceAddr = (address *) data; // typecast to address to get address from data ( 6 bytes )
	membershiplist *memdata=(membershiplist *) ( sourceAddr + 1 ); // memdata pointer to array of structs

	printf("\n GOSSIP for node %d",node->addr.addr[0]);
	int tablecount=(size - sizeof(address)) / sizeof(membershiplist);
	printf("\nnumber of records received  : %d",tablecount);
	printf("\nnumber of records in its mem table : %d",node->memcount);
	int i=0;

	int j;
	int nodepresentindata=-1;

	for(i=0;i<tablecount;i++){
		printf("\nCHECKING FOR %d",memdata->maddr.addr[0]);
		nodepresentindata=0;
		for(j=0;j<node->memcount;j++){
			printf("\n : FOR %d",node->memtable[j].maddr.addr[0]);

			if(memcmp(&node->memtable[j].maddr,&memdata->maddr,sizeof(address)) == 0 ){

				//check if the node's memtable contains the sender's address
				nodepresentindata=1;
				//check if latest update time for this member
				if (node->memtable[j].lastupdatetime < memdata->lastupdatetime){
					printf("\n UPDATING TIME FOR %d",memdata->maddr.addr[0]);
					node->memtable[j].lastupdatetime = memdata->lastupdatetime;
				}

				if ((getcurrtime()-node->memtable[j].lastupdatetime) > T_CLEANUP ){
					if (getcurrtime() - memdata->lastupdatetime < T_FAIL ){
						node->memtable[j].tocleanup=1;
					} else {
						if ( node->memtable[j].tocleanup != 2){
							node->memtable[j].tocleanup=2;
							printf("\n REMOVING NODE %d",memdata->maddr.addr[0]);
							logNodeRemove(&node->addr,&node->memtable[i].maddr);
						}
					}
				}

			}
		}

		if ( nodepresentindata == 0  ){


			 memcpy(&node->memtable[node->memcount].maddr,&memdata->maddr,sizeof(address));
			 node->memtable[node->memcount].lastupdatetime=getcurrtime();
			 node->memtable[node->memcount].tocleanup=0;
			 printf("\nADDING NODE %d",memdata->maddr.addr[0]);
			 logNodeAdd(&node->addr,&memdata->maddr);
			 printf("\nADDING into mem table  node %d",memdata->maddr.addr[0]);
			 node->memcount++;
		}

		memdata++;
	}

	printf("\n\n***** GOSSIP OVER ****\n\n ");

	return;
}

/* 
Received a JOINREP (joinreply) message. 
*/
void Process_joinrep(void *env, char *data, int size)
{

	// reading the data received
	// This is called when the node receives membership table information from the initiator

    member *node = (member *) env;
    membershiplist *memdata = (membershiplist *)data; // typecast to membership table structure type


	int selfPresent=0;
	printf("\n ***************** JOINREP for %d *********************************",node->addr.addr[0]);
	// for each struct array element loop and print
	int tablecount=size/sizeof(membershiplist);
	printf("\nnumber of tables %d",tablecount);
	int i=0;
	int n=-1;
	for(i=0;i<tablecount;i++){

		memcpy(&node->memtable[i].maddr,&memdata->maddr,sizeof(address));
		//;
		node->memtable[i].tocleanup=memdata->tocleanup;
		node->memtable[i].lastupdatetime=memdata->lastupdatetime;

		printf("\nAdding node %d into %d s table",memdata->maddr.addr[0],node->addr.addr[0]);
		logNodeAdd(&node->addr,&memdata->maddr);
		n = memcmp (&memdata->maddr, &node->addr,sizeof(address));
			if ( n == 0 ) {
				selfPresent=1;
		}
		memdata++;
	}

	if ( selfPresent == 0 )  { 
		//indicates the node's mem list table does not contain its information
		// add to the table
		printf("SELF PRESEN MISSING \n");
		memcpy(&node->memtable[i].maddr,&node->addr,sizeof(address));
		node->memtable[i].lastupdatetime=getcurrtime();
		node->memtable[i].tocleanup=0;
		node->memcount++;
		logNodeAdd(&node->addr,&node->addr);
	}
	node->memcount=tablecount;
	// after copying the membership table , we set its ingroup to 1.
	node->ingroup = 1;
    return;
}


/* 
Array of Message handlers. 
*/
void ( ( * MsgHandler [20] ) STDCLLBKARGS )={
/* Message processing operations at the P2P layer. */
    Process_joinreq, 
    Process_joinrep,
	Process_gossipmsg
};

/* 
Called from nodeloop() on each received packet dequeue()-ed from node->inmsgq. 
Parse the packet, extract information and process. 
env is member *node, data is 'messagehdr'. 
*/
int recv_callback(void *env, char *data, int size){

    member *node = (member *) env;
    messagehdr *msghdr = (messagehdr *)data;
    char *pktdata = (char *)(msghdr+1);

    if(size < sizeof(messagehdr)){
#ifdef DEBUGLOG
        LOG(&((member *)env)->addr, "Faulty packet received - ignoring");
#endif
        return -1;
    }

#ifdef DEBUGLOG
    LOG(&((member *)env)->addr, "Received msg type %d with %d B payload", msghdr->msgtype, size - sizeof(messagehdr));
#endif

    if((node->ingroup && msghdr->msgtype >= 0 && msghdr->msgtype <= DUMMYLASTMSGTYPE)
        || (!node->ingroup && msghdr->msgtype==JOINREP))            
            /* if not yet in group, accept only JOINREPs */
        MsgHandler[msghdr->msgtype](env, pktdata, size-sizeof(messagehdr));
    /* else ignore (garbled message) */
    free(data);

    return 0;

}

/*
 *
 * Initialization and cleanup routines.
 *
 */

/* 
Find out who I am, and start up. 
*/
int init_thisnode(member *thisnode, address *joinaddr){
    
    if(MPinit(&thisnode->addr, PORTNUM, (char *)joinaddr)== NULL){ /* Calls ENInit */
#ifdef DEBUGLOG
        LOG(&thisnode->addr, "MPInit failed");
#endif
        exit(1);
    }
#ifdef DEBUGLOG
    else LOG(&thisnode->addr, "MPInit succeeded. Hello.");
#endif

    thisnode->bfailed=0;
    thisnode->inited=1;
    thisnode->ingroup=0;
    /* node is up! */

    return 0;
}


/* 
Clean up this node. 
*/
int finishup_thisnode(member *node){

	/* <your code goes in here> */
    return 0;
}


/* 
 *
 * Main code for a node 
 *
 */

/* 
Introduce self to group. 
*/
int introduceselftogroup(member *node, address *joinaddr){
    
    messagehdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if(memcmp(&node->addr, joinaddr, 4*sizeof(char)) == 0){
        /* I am the group booter (first process to join the group). Boot up the group. */
#ifdef DEBUGLOG
        LOG(&node->addr, "Starting up group...");
#endif

        node->ingroup = 1;
		node->nodenumber = 0;
		// adding the membership data of node0 
		node->memtable[node->nodenumber].lastupdatetime=getcurrtime();
		memcpy(&node->memtable[node->nodenumber].maddr,&node->addr,sizeof(address));
		node->memtable[node->nodenumber].tocleanup=0;
		node->memcount=1;


		logNodeAdd(&node->addr,&node->addr);
		printf("log node added ");
		
    }
    else{
        size_t msgsize = sizeof(messagehdr) + sizeof(address);
        msg=malloc(msgsize);

    /* create JOINREQ message: format of data is {struct address myaddr} */
        msg->msgtype=JOINREQ;
        memcpy((char *)(msg+1), &node->addr, sizeof(address));

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        LOG(&node->addr, s);
#endif

    /* send JOINREQ message to introducer member. */
        MPp2psend(&node->addr, joinaddr, (char *)msg, msgsize);
        // to increment unique node number count and assign to the newly formed node.
		MEMCOUNT++;
		node->nodenumber=MEMCOUNT;
		
        free(msg);
    }

    return 1;

}

/* 
Called from nodeloop(). 
*/
void checkmsgs(member *node){
    void *data;
    int size;

    /* Dequeue waiting messages from node->inmsgq and process them. */
	
    while((data = dequeue(&node->inmsgq, &size)) != NULL) {
        recv_callback((void *)node, data, size); 
    }
    return;
}


/* 
Executed periodically for each member. 
Performs necessary periodic operations. 
Called by nodeloop(). 
*/
/* 
Executed periodically at each member. Called from app.c.
*/
void nodeloop(member *node){
    if (node->bfailed) return;

    checkmsgs(node);
    printf("\n<<<<<<<<   CHECK MESSAGE DONE ");

    /* Wait until you're in the group... */
    if(!node->ingroup) return ;

    /* ...then jump in and share your responsibilites! */
    nodeloopops(node);
    
    return;
}

/* 
All initialization routines for a member. Called by app.c. 
*/
void nodestart(member *node, char *servaddrstr, short servport){

    address joinaddr=getjoinaddr();

    /* Self booting routines */
    if(init_thisnode(node, &joinaddr) == -1){

#ifdef DEBUGLOG
        LOG(&node->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if(!introduceselftogroup(node, &joinaddr)){
        finishup_thisnode(node);
#ifdef DEBUGLOG
        LOG(&node->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/* 
Enqueue a message (buff) onto the queue env. 
*/
int enqueue_wrppr(void *env, char *buff, int size){    return enqueue((queue *)env, buff, size);}

/* 
Called by a member to receive messages currently waiting for it. 
*/
int recvloop(member *node){
    if (node->bfailed) return -1;
    else return MPrecv(&(node->addr), enqueue_wrppr, NULL, 1, &node->inmsgq); 
    /* Fourth parameter specifies number of times to 'loop'. */
}

