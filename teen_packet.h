#ifndef _teen_packet_h_
#define _teen_packet_h_
#include <packet.h>
//===============================
//Packet format
//===============================
#define TEEN_REQ 0x01
#define TEEN_RESP 0x02
#define TEEN_CONT 0x03
#define TEEN_RESULT 0x04
#define TEEN_CRESET 0x05
//===============================
//Direct access to  packet header 
//===============================
#define HDR_TEEN(p) ((struct hdr_teen*)hdr_teen::access(p))
#define HDR_TEEN_REQ(p) ((struct hdr_teen_req*)hdr_teen::access(p))
#define HDR_TEEN_RESP(p) ((struct hdr_teen_resp*)hdr_teen::access(p))
#define HDR_TEEN_CONT(p) ((struct hdr_teen_cont*)hdr_teen::access(p))
#define HDR_TEEN_RESULT(p) ((struct hdr_teen_result*)hdr_teen::access(p)) 
#define HDR_TEEN_CRESET(p) ((struct hdr_teen_creset*)hdr_teen::access(p))
//===============================
//Dfault TEEN packet 
//===============================
struct hdr_teen {
  u_int8_t pkt_type;
  //header access
  static int offset_;
  inline static int& offset() { return offset_; }
  inline static hdr_teen* access(const Packet *p){
    return (hdr_teen*) p->access(offset_);

  }
};
//===============================
//TEEN_REQ packet format
//===============================
struct hdr_teen_req {
  u_int8_t pkt_type;
  u_int8_t pkt_id;
  nsaddr_t src_nodeid;


  inline int size(){
    int sz=0;
    sz=sizeof(struct hdr_teen_req);
    assert (sz>=0);
    return sz;
  }
};

//================================
//TEEN_RESP packet format
//================================
struct hdr_teen_resp {

  u_int8_t pkt_type;
  u_int8_t pkt_id;
  nsaddr_t src_nodeid;
  nsaddr_t desti_nodeid;
 // u_int8_t posx_;
 // u_int8_t posy_;

  inline int size(){
    int sz=0;
    sz=sizeof(struct hdr_teen_resp);
    assert (sz>=0);
    return sz;
  }
};
//================================
//TEEN_CONT packet format
//================================
struct hdr_teen_cont{
  u_int8_t pkt_type;
  u_int8_t pkt_id;
  nsaddr_t udesti_nodeid;
  nsaddr_t desti_nodeid;
  nsaddr_t src_nodeid;
  inline int size(){
    int sz=0;
    sz=sizeof(struct hdr_teen_cont);
    assert (sz>=0);
    return sz;
  }
};
//================================
//TEEN_SUCCESS packet format
//================================
struct hdr_teen_result {
  u_int8_t pkt_type;
  nsaddr_t src_nodeid;
  u_int8_t pkt_id;
  u_int8_t route_info;

  inline int size(){
    int sz=0;
    sz=sizeof(struct hdr_teen_result);
    assert (sz>=0);
    return sz;

  }
};
struct hdr_teen_creset {
 u_int8_t pkt_type;
 nsaddr_t src_nodeid;
 u_int8_t pkt_id;

 inline int size(){
    int sz=0;
    sz=sizeof(struct hdr_teen_creset);
    assert (sz>=0);
    return sz;

  }

};
#endif
