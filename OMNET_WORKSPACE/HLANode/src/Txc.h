/* ----------------------------------------------------------------------------
 * COSSIM - HLANode
 * Copyright (c) 2018, H2020 COSSIM.
 * Copyright (c) 2018, Telecommunications Systems Institute.
 * 
 * Author: Nikolaos Tampouratzis, ntampouratzis@isc.tuc.gr
 * ----------------------------------------------------------------------------
 *
*/

#ifndef __HLA_NODE_TCX_H
#define __HLA_NODE_TCX_H
//#define DEBUG_MSG
//#define NO_HLA

#include <typeinfo>
#include <omnetpp.h>

#include <EtherEncap.h>
#include <EtherFrame.h>
#include <Ethernet.h>
#include <Ieee802Ctrl.h>
#include <ARPPacket_m.h>

#include <IPv4ControlInfo.h>
#include <ICMPMessage.h>
#include <IPv4Datagram.h>
#include <IPv4Address.h>
#include <IPv4.h>
#include <ARP.h>

#ifndef NO_HLA
    #include "HLA_OMNET.hh"
#endif

#define MAX_PACKET_LENGTH 2048

#include "myPacket_m.h"

#include <dlfcn.h> //dynamic linking
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdio.h>

using std::string ;
using std::cout ;
using std::endl ;

#ifdef NO_HLA
class EtherTestPacket{

public:

    uint8_t *data;
    unsigned int length ;

    EtherTestPacket();
};
#endif


namespace HLANode {


class SyncNode : public cSimpleModule
{
public:
  int NUMBER_OF_HLA_NODES;
  int NODE_NO;

  double SYNCH_TIME;

  string federate = "SYNCH_OMNET" ;

#ifndef NO_HLA
  HLA_OMNET * HLAGlobalSynch=0;
#endif

  bool TerminateNormal; //! Terminate from GEM5 !//

    virtual void initialize();
    virtual void HLA_initialization(void);
    virtual void handleMessage(cMessage *msg);

    virtual void HLANodesInitialization(cModule *parent);
    virtual void HLANodesFinalization(cModule *parent);


};


class Txc0 : public cSimpleModule
{
public:
    int NUMBER_OF_HLA_NODES;
    int NODE_NO;

    double RX_PACKET_TIME;

    string federationName;

    string federate = "OMNET" ;

#ifndef NO_HLA
  HLA_OMNET * NODEHLA=0;
#endif

    bool TerminateNormal; //! Terminate from GEM5 !//


    bool L2_Routing= false;
    bool send_packet= false;
    bool TOGGLE= true;
#ifdef DEBUG_MSG
    //----CRC stuff
    #define POLYNOMIAL 0x8408
    unsigned short  crcTable[256];

    virtual void  CRC_Init();
    virtual unsigned short CRC_Calculate(const uint8_t *message, int nBytes);
#endif
    virtual void HLA_initialization(int);

    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    void  setPayloadArray(myPacket *msg, uint8_t *payload, int length);
    void  getPayloadArray(myPacket *msg, uint8_t *payload, int length);
    void sendCopyOf(myPacket *msg);

private:
    inet::IPv4Datagram *datagram;
    inet::ICMPMessage *icmp;
    inet::IPv4ControlInfo *controlInfo;
    long total_bytes_sent=0;
    long total_bytes_received=0;

    unsigned char tmp_src[6];
    unsigned char tmp_dst[6];

    uint8_t  ethertype_dec[2];
    int ethertype=0;
    int snap_local_code=0;
    int tmp_ethertype;

    uint8_t tmp_payload[MAX_PACKET_LENGTH];
    uint8_t total_payload[MAX_PACKET_LENGTH];
    uint8_t from_eth_payload[MAX_PACKET_LENGTH];
    //-----------------------------

    //http://www.tcpipguide.com/free/t_IPDatagramGeneralFormat.htm

    short my_Version; //15 1/2
    short my_IHL; //15 2/2 (Internet Header Length)

    unsigned char my_TOS; //16  (Type Of Service)
    int my_Total_Length; //17-18
    int my_Identification; //19-20

    bool my_Flags_DF;//21: 2/8
    bool my_Flags_MF;//21: 3/8
    int my_Fragment_Offset; //21: 5/8 -22

    short my_Time_to_Live; //23
    int my_Protocol; //24
    unsigned char my_Header_Checksum[2]; //25-26
    int my_source_ip[4];//27-30
    int my_dest_ip[4];//31-34

};


}; // namespace



#endif
