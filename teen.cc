#include <teen/teen.h>
#include <teen/teen_packet.h>
#include <random.h>
#include <cmu-trace.h>
#include <math.h>
#include <unistd.h>

#define DEBUG
//=================================
//Tcl hooking class
//=================================
int hdr_teen::offset_;
static class TEENHeaderClass : public PacketHeaderClass
{
	public:
		TEENHeaderClass() : PacketHeaderClass("PacketHeader/TEEN", sizeof(hdr_teen))
	{
	  bind_offset(&hdr_teen::offset_);
	} 
} class_rtProtoTEEN_hdr;
	
static class TEENclass : public TclClass
{
 public:
	TEENclass() : TclClass("Agent/TEEN") {}
	TclObject* create(int argc, const char*const* argv)
	{
		assert(argc == 5);
		return (new TEEN((nsaddr_t)Address::instance().str2addr(argv[4])));
	}
} class_rtProtoTEEN;


//=======================================
//AGENT construction
//=======================================
TEEN::TEEN(nsaddr_t id) : Agent(PT_TEEN)
{
	#ifdef DEBUG
	printf("N node is set to %d, start broadcasting  \n",index);
	#endif               
	index=id;
	rlink_=-1;
	flink_=-1;
	flag_=0;
	ultimated_=0;
	result_=0;
	seqno=1;
}

int TEEN::command(int argc, const char*const* argv)
{
	if(argc==4)
	{
		if(strncasecmp(argv[1], "start", 5) == 0) 
		{
			nsaddr_t source = atoi(argv[2]);
			nsaddr_t dest = atoi(argv[3]);
			printf("Source = %d",source);
			printf("Dest = %d\n",dest);
			if(dest != 0)
			send_req(source,dest);
			return TCL_OK;
			#ifdef DEBUG
			printf("Node is set to %d, start broadcasting  \n", index);
			#endif               
		}	
	}  
	if (argc == 2 )
	{
		if (strcasecmp(argv[1],"start") == 0)
		{
			nsaddr_t  source=atoi(argv[2]);
			printf("source=%d",source);
			call_creset(source);
			return TCL_OK;
		}
	}	

	if (argc == 3)
	{
		if (strcmp(argv[1], "index") == 0)
		{
		      index = atoi(argv[2]);
		      return TCL_OK;
		}

		else if (strncasecmp(argv[1], "start", 5) == 0)
		{	
			nsaddr_t  source=atoi(argv[2]);
			printf("source=%d",source);
			call_creset(source);
			return TCL_OK;
     
			#ifdef DEBUG
			printf("N : node is set to %d, start broadcasting  \n", index);
			#endif               
  		} 

		else if(strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0)
		{
      			logtarget = (Trace*) TclObject::lookup(argv[2]);
			if(logtarget == 0)
				return TCL_ERROR;
			return TCL_OK;
		}
		else if(strcmp(argv[1], "drop-target") == 0)
		{
			return Agent::command(argc, argv);
    		}
    		else if(strcmp(argv[1], "if-queue") == 0)
		{
			ifqueue = (PriQueue*) TclObject::lookup(argv[2]);
      			if(ifqueue == 0)
				return TCL_ERROR;
      			return TCL_OK;
		}
		else if (strcmp(argv[1], "port-dmux") == 0)
		{
		    	dmux_ = (PortClassifier *)TclObject::lookup(argv[2]);
			if (dmux_ == 0)
			{
				fprintf (stderr, "%s: %s lookup of %s failed\n", __FILE__,argv[1], argv[2]);
				return TCL_ERROR;
			}
			return TCL_OK;
		}
	}
	return Agent::command(argc, argv);
}

//=========================================
//Send routine
//=========================================
void TEEN :: send_req(nsaddr_t src, nsaddr_t dest)
{
	printf("\n\nSource=%d\n",src);
	if(rlink_==-1)
	{
		if(flag_==0)
		{
			flag_=1;
			
		}
	}
	ultimated_=dest;
	if(index==src)
	{
		Packet *p = Packet::alloc();
		struct hdr_cmn *ch = HDR_CMN(p);
		struct hdr_ip *ih = HDR_IP(p);
		struct hdr_teen_req *req = HDR_TEEN_REQ(p);
		#ifdef DEBUG
		printf("sending request from %d\n", index);
		#endif  
	        
		// Write Channel Header
		ch->ptype() = PT_TEEN;
		ch->direction()=hdr_cmn::DOWN;
		ch->next_hop()=IP_BROADCAST;
		ch->size() = IP_HDR_LEN + req->size();
		//ch->next_hop() = IP_BROADCAST;
		ch->addr_type() = NS_AF_NONE;
		ch->prev_hop_ = index;
		        
		// Write IP Header
		ih->saddr() = index;
		ih->daddr() = IP_BROADCAST;
		ih->sport() = RT_PORT;
		ih->dport() = RT_PORT;
		ih->ttl_ = NETWORK_DIAMETER;
	        
		// Write req Header
		req->pkt_type = TEEN_REQ;
		req->src_nodeid = index;
		req->pkt_id = seqno;
		
		seqno += 1;
		
		Scheduler::instance().schedule(target_,p,JITTER);
	 
		#ifdef DEBUG
		
		#endif 
	}	

}
// ======================================================================
//  Recv req Packet
// ======================================================================
void TEEN::recv_req(Packet *p)
{
	struct hdr_teen_req *req = HDR_TEEN_REQ(p);
		
	// I have originated the packet, just drop it
	if(req->src_nodeid== index)
	{ //if source is myself
		Packet::free(p);
		return;
	}

	if(flag_ == 0)
	{
		flag_=1;	
		rlink_=req->src_nodeid;
		printf("Rlink of %d is %d\n",index,rlink_); 
		send_resp(rlink_,index);
	}   
	else
	{
		printf("\nNode %d has already a leader %d\n",index,rlink_);
	
		Packet::free(p);
 		return;
	}
	  
}

// ======================================================================
//  Send Response Routine
// ======================================================================
void TEEN::send_resp(nsaddr_t tnodeid_,nsaddr_t pnodeid_)
{
 	
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_teen_resp *resp = HDR_TEEN_RESP(p);

	ch->ptype() = PT_TEEN;
	ch->direction()=hdr_cmn::DOWN;
	ch->next_hop()=tnodeid_;
	ch->size() = IP_HDR_LEN + resp->size();
	ch->addr_type() = NS_AF_INET;
	ch->prev_hop_ = pnodeid_;

	// Write IP Header
	ih->saddr() = index;
	ih->daddr() = rlink_;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl_ = NETWORK_DIAMETER;

	// Write resp Header
	resp->pkt_type = TEEN_RESP;
	resp->desti_nodeid = rlink_;
	resp->src_nodeid =index;
	resp->pkt_id = seqno;
		
	// increase sequence number 
	seqno += 1;
	Scheduler::instance().schedule(target_, p, JITTER);	
	#ifdef DEBUG
	printf("send response by %d to %d \n",resp->src_nodeid,resp->desti_nodeid);
	#endif 
}

void TEEN::recv_resp(Packet *p) 
{
	struct hdr_teen_resp *resp = HDR_TEEN_RESP(p);
       	
	// I have originated the packet, just drop it	 
	if(resp->src_nodeid == index)
	{ //if source is myself
		Packet::free(p);
		return;
	}
	
        //if (resp->desti_nodeid == index)	
	{  
		printf("\nReceived response by %d from %d \n",resp->desti_nodeid,resp->src_nodeid);      
	
        	if(resp->src_nodeid == ultimated_)
		{
			if(rlink_==-1)
			{
				flink_=resp->src_nodeid;
        			printf("Route connected");
			}
			else
			{
				result_=1;
				flink_=resp->src_nodeid;
				printf("Flink of %d is %d",index,flink_);
       			 	send_result(rlink_,result_);	
			}
        	}     
		else if(resp->src_nodeid != ultimated_)
		{
			send_cont(resp->src_nodeid,ultimated_);
		}
	}
}		


void TEEN::send_cont(nsaddr_t desti_,nsaddr_t ulti_)
{   	
	Packet *p = Packet::alloc();
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_teen_cont *cont = HDR_TEEN_CONT(p);
	

	// Write Channel Header
	ch->ptype() = PT_TEEN;
	ch->direction()=hdr_cmn::DOWN;
	ch->next_hop()=desti_;
	ch->size() = IP_HDR_LEN + cont->size();
	ch->addr_type() = NS_AF_INET;
	ch->prev_hop_ = index;

	// Write IP Header
	ih->saddr() = index;
	ih->daddr() = desti_;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl_ = NETWORK_DIAMETER;

	// Write Confirm Header
	cont->pkt_type = TEEN_CONT;
	//cont->desti_nodeid = desti_;
        cont->src_nodeid= index;
        //want to write ultimate destination here
	cont->udesti_nodeid =ulti_ ;
	cont->pkt_id = seqno;
	// increase sequence number for next beacon
	seqno += 1;
	Scheduler::instance().schedule(target_, p, 0.0);
	#ifdef DEBUG
	printf(" \ncont pkt sent by %d to %d  \n",index,desti_);
	#endif 
}

void TEEN::recv_cont(Packet *p)
{	
	struct hdr_teen_cont *cont = HDR_TEEN_CONT(p);
        ultimated_=cont->udesti_nodeid; 
	if (cont->src_nodeid == rlink_)
	{
		printf("Recieved cont packet from %d\n",cont->src_nodeid);              
                send_req(index,ultimated_);    
	}
	else
	{
		Packet::free(p);
		return;
	}

}

void TEEN::send_result(nsaddr_t leader_,u_int8_t res)
{
	Packet *p = Packet::alloc();

	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	struct hdr_teen_result *result = HDR_TEEN_RESULT(p);
	//channel header
        ch->ptype() = PT_TEEN;
	ch->direction()=hdr_cmn::DOWN;
	ch->next_hop()=leader_;
	ch->size() = IP_HDR_LEN + result->size();
	ch->addr_type() = NS_AF_INET;
	ch->prev_hop_ = index;

	// Write IP Header
	ih->saddr() = index;
	ih->daddr() = leader_;
	ih->sport() = RT_PORT;
	ih->dport() = RT_PORT;
	ih->ttl_ = NETWORK_DIAMETER;

	//write result header
     	result->pkt_type=TEEN_RESULT;
     	result->pkt_id=seqno;
     	result->src_nodeid=index;
	//write the status of the route
     	result->route_info=res; 

	seqno+=1;
	Scheduler::instance().schedule(target_, p, 0.0);
	#ifdef DEBUG
	printf(" sent result pkt by %d  to %d \n",index,leader_);
	#endif 
}

void TEEN::recv_result(Packet *p)
{
	struct hdr_teen_result *result = HDR_TEEN_RESULT(p);
	if(result->route_info==1 && rlink_!=-1)
	{
		flink_=result->src_nodeid;
		printf("Flink of %d is %d",index,flink_);
		send_result(rlink_,result->route_info);
	}
	else if(result->route_info==1 && rlink_==-1)
	{
		flink_=result->src_nodeid;
		printf("Route successful");

	}
	else
	{
		Packet::free(p);
 		return;
	}
}

void TEEN::recv(Packet *p, Handler* h)
{
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);
	// if the packet is routing protocol control packet, give the packet to agent
	if(ch->ptype() == PT_TEEN)
	{//printf(" entered2\n");
		ih->ttl_ -= 1;
		recv_teen(p);
		return;
	}

	//  Must be a packet I'm originating
	if((ih->saddr() == index) && (ch->num_forwards() == 0))
	{
 	
		// Add the IP Header. TCP adds the IP header too, so to avoid setting it twice, 
		// we check if  this packet is not a TCP or ACK segment.

		if (ch->ptype() != PT_TCP && ch->ptype() != PT_ACK)
		{
			ch->size() += IP_HDR_LEN;
		}

	}

	// I received a packet that I sent.  Probably routing loop.
	else if(ih->saddr() == index)
	{
   		drop(p, DROP_RTR_ROUTE_LOOP);
		return;
	}

	//  Packet I'm forwarding...
	else
	{
		if(--ih->ttl_ == 0)
		{
			drop(p, DROP_RTR_TTL);
			return;
   		}
	}

	// This is data packet, find route and forward packet
	recv_data(p);
}


void TEEN::recv_data(Packet *p)
{
	struct hdr_ip *ih = HDR_IP(p);
	

	#ifdef DEBUG
		printf("R : Recieved data by %d  \n", index);
	#endif 
	
	/*if(ih->daddr() == index)
	{
		printf("destination reached\n");
		return;
	}*/

	if(flink_!=-1)
	{
		printf("Forwarding to %d\n",flink_);
		forward(p, flink_, 0.0);
 	}
	else if(flink_==-1)
	{
		printf("No forward link");
	}
}

void TEEN::forward(Packet *p, nsaddr_t nexthop, double delay)
{
	struct hdr_cmn *ch = HDR_CMN(p);
	struct hdr_ip *ih = HDR_IP(p);

	if (ih->ttl_ == 0)
	{
		drop(p, DROP_RTR_TTL);
	}
	
	if (nexthop != (nsaddr_t) IP_BROADCAST)
	{
		ch->next_hop_ = nexthop;
		ch->prev_hop_ = index;
		ch->addr_type() = NS_AF_INET;
		ch->direction() = hdr_cmn::DOWN;
	}
	else
	{
		assert(ih->daddr() == (nsaddr_t) IP_BROADCAST);
		ch->prev_hop_ = index;
		ch->addr_type() = NS_AF_NONE;
		ch->direction() = hdr_cmn::DOWN; 
	}
	Scheduler::instance().schedule(target_, p, delay);

}

void TEEN::recv_teen(Packet *p)
{
	struct hdr_teen *wh = HDR_TEEN(p);
	assert(ih->sport() == RT_PORT);
	assert(ih->dport() == RT_PORT);




	// What kind of packet is this
	switch(wh->pkt_type)
	{

		case TEEN_REQ:
			recv_req(p);
			break;

		case TEEN_RESP:
			recv_resp(p);
			break;
		
		case TEEN_CONT:
			recv_cont(p);
			break;

		case TEEN_RESULT:
			recv_result(p);
			break;
	        
		case TEEN_CRESET:
			recv_creset(p);
			break;

		default:
			fprintf(stderr, "Invalid packet type (%x)\n", wh->pkt_type);
			exit(1);
	}
}

void TEEN::call_creset(nsaddr_t src)
{	
     	if(index == src)
	{
		Packet *p = Packet::alloc();
		struct hdr_cmn *ch = HDR_CMN(p);
		struct hdr_ip *ih = HDR_IP(p);
		struct hdr_teen_creset *rt = HDR_TEEN_CRESET(p);
		printf("Sending RESET from %d \n", index);
		
		// Write Channel Header
		ch->ptype() = PT_TEEN;
		ch->direction()=hdr_cmn::DOWN;
		ch->next_hop()=IP_BROADCAST;
		ch->size() = IP_HDR_LEN + rt->size();
		//ch->next_hop() = IP_BROADCAST;
		ch->addr_type() = NS_AF_NONE;
		ch->prev_hop_ = index;
	
		// Write IP Header
		ih->saddr() = index;
		ih->daddr() = IP_BROADCAST;
		ih->sport() = RT_PORT;
		ih->dport() = RT_PORT;
		ih->ttl_ = NETWORK_DIAMETER;
	
		// Write breq Header
		rt->pkt_type = TEEN_CRESET;
		rt->src_nodeid = index;
		rt->pkt_id = seqno;
		seqno += 1;
		
		Scheduler::instance().schedule(target_,p,JITTER);
	
	}	
}

void TEEN::recv_creset(Packet *p)
{
	struct hdr_teen_creset *rt = HDR_TEEN_CRESET(p);
	
	// I have originated the packet, just drop it
	if (rt->src_nodeid== index) 
	{ //if source is myself
		Packet::free(p);
		return;
	}
	if (flag_== 0||rlink_==-1)
	{
		Packet::free(p);
		return;
	}
		
        else if(flag_!=0||rlink_!=-1)
	{
		printf("Myid=%d\n",index);
	//	index=id;
		rlink_=-1;
		flink_=-1;
		flag_=0;
		ultimated_=0;
		result_=0;
		seqno=1;
		call_creset(index);
	}
	//call_creset(index);
}

