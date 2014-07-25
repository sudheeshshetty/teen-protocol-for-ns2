

#ifndef __teen_h__
#define __teen_h__
#include "teen_packet.h"
#include <agent.h>
#include <packet.h>
#include <node.h>
#include <timer-handler.h>
#include <cmu-trace.h>
#include <random.h>
#include <priqueue.h>
#include <classifier/classifier-port.h>
#include <mobilenode.h>
#include <assert.h>
#include <sys/types.h>
#include <config.h>
#include <lib/bsd-list.h>
#include <scheduler.h>
#include <ip.h>

#define JITTER	(Random::uniform()*0.001)
#define NETWORK_DIAMETER		64
//class f_entry;

class TEEN : public Agent
{
	friend class f_entry;
 public:
  TEEN(nsaddr_t id);
  void recv(Packet*p,Handler *);
  int command(int,const char *const *);
  int initialized() {return 1 && target_;}
  
// Agent Attributes
	nsaddr_t	index;     // node address (identifier)
	nsaddr_t   	rlink_;     // link to node
        nsaddr_t        ultimated_;
	u_int8_t	flag_;
	//u_int8_t      cflag_;
        u_int8_t	result_;
        u_int8_t	seqno;     
        u_int8_t        route_;
        nsaddr_t        flink_;
    
//CC related attributes
        void insert(nsaddr_t id);
	//int lookup(nsaddr_t id);
	//f_entry* lookup(nsaddr_t id);
	//void Display(nsaddr_t id,nsaddr_t num);
//follower list
//	f_entry* first;
	
//send routine
        //void Node_info(nsaddr_t id,nsaddr_t num); 
	//void call_reset(nsaddr_t src);
	void send_req(nsaddr_t src,nsaddr_t dest);
        void send_resp(nsaddr_t tnodeid_,nsaddr_t pnodeid_);
	void send_cont(nsaddr_t desti,nsaddr_t ulti_);
        void send_result(nsaddr_t leader_,u_int8_t res);
        void forward(Packet *p, nsaddr_t nexthop, double delay);
	void call_creset(nsaddr_t src);	
//recv routine
	  void recv_data(Packet *p); 
	  void recv_req(Packet *p);
	  void recv_resp(Packet *p);
	  void recv_cont(Packet *p);
	  void recv_result(Packet *p);
	  void recv_teen(Packet *p);
	  void recv_creset(Packet *p);
	
          //void recv_creset(Packet *p);
        Trace		*logtarget;
 	// A pointer to the network interface queue that sits between the "classifier" and the "link layer"
        PriQueue	*ifqueue;

	// Port classifier for passing packets up to agents
	PortClassifier	*dmux_;

	 
	 
};
/*
class f_entry {
        
        friend class TEEN;
 public:
      	nsaddr_t f_id;
	f_entry* next;	
      f_entry* prev;
	     };
*/
#endif /* __TEEN_h__ */	
