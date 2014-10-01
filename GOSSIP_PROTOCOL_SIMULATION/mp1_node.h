/**********************
*
* Progam Name: MP1. Membership Protocol.
* 
* Code authors: <your name here>
*
* Current file: mp2_node.h
* About this file: Header file.
* 
***********************/

#ifndef _NODE_H_
#define _NODE_H_

#include "stdincludes.h"
#include "params.h"
#include "queue.h"
#include "requests.h"
#include "emulnet.h"
#define MAX_NODES 100 				// static max count of membership table

/* Configuration Parameters */
char JOINADDR[30];                    /* address for introduction into the group. */
extern char *DEF_SERVADDR;            /* server address. */
extern short PORTNUM;                 /* standard portnum of server to contact. */

/* Miscellaneous Parameters */
extern char *STDSTRING;

typedef struct membershiplist{            
        struct address maddr;             // node's address
        int tocleanup;                    // boolean indiciating if this member is to be marked for deletion ( 1 ) or to be deleted ( 2 )
        int lastupdatetime;               // Here the lastupdatetime saves the snapshot.
        int heartbeatCounter;			  //  Heartbeat counter

} membershiplist;
typedef struct member{            
        struct address addr;            // my address
        int inited;                     // boolean indicating if this member is up
        int ingroup;                    // boolean indiciating if this member is in the group
        queue inmsgq;                   // queue for incoming messages
        int bfailed;                    // boolean indicating if this member has failed
        int nodenumber;     			// This saves the unique node number
        int memcount;					// count of members in its membership table
		membershiplist memtable[MAX_NODES];
} member;

/* Message types */
/* Meaning of different message types
  JOINREQ - request to join the group
  JOINREP - replyto JOINREQ
*/
enum Msgtypes{
		JOINREQ,			
		JOINREP,
		GOSSIPMSG,
		DUMMYLASTMSGTYPE
};

/* Generic message template. */
typedef struct messagehdr{ 	
	enum Msgtypes msgtype;
} messagehdr;


/* Functions in mp2_node.c */

/* Message processing routines. */
STDCLLBKRET Process_joinreq STDCLLBKARGS;
STDCLLBKRET Process_joinrep STDCLLBKARGS;
STDCLLBKRET Process_gossipmsg STDCLLBKARGS;

/*
int recv_callback(void *env, char *data, int size);
int init_thisnode(member *thisnode, address *joinaddr);
*/

/*
Other routines.
*/

void nodestart(member *node, char *servaddrstr, short servport);
void nodeloop(member *node);
int recvloop(member *node);
int finishup_thisnode(member *node);

#endif /* _NODE_H_ */

