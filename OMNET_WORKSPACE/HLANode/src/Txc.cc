/* ----------------------------------------------------------------------------
 * COSSIM - HLANode
 * Copyright (c) 2018, H2020 COSSIM.
 * Copyright (c) 2018, Telecommunications Systems Institute.
 * 
 * Author: Nikolaos Tampouratzis, ntampouratzis@isc.tuc.gr
 * ----------------------------------------------------------------------------
 *
*/


#include "Txc.h"
#include "myPacket_m.h"

using namespace std;
using namespace inet;


namespace HLANode {


uint16_t NetIpChecksum(uint16_t  ipHeader[], int nWords)
{
    uint32_t  sum = 0;// it should be uint32_t i guess
/*      * IP headers always contain an even number of bytes.      */
 while (nWords-- > 0)     {         sum += *(ipHeader++);     }
 /*      * Use carries to compute 1's complement sum.      */
sum = (sum >> 16) + (sum & 0xFFFF);     sum += sum >> 16;
 /*      * Return the inverted 16-bit result.      */
return ((uint16_t) ~sum);
}   /* NetIpChecksum() */




void SyncNode::HLANodesInitialization(cModule *parent){
    char str[20];
    for (int i=0;i<NUMBER_OF_HLA_NODES;i++){
        //!Initialize the HLA enable NODES !//
        sprintf(str, "node%d", i);
        Txc0* TX = (Txc0*)parent->getSubmodule(str);
#ifndef NO_HLA
        TX->HLA_initialization(NUMBER_OF_HLA_NODES);
#endif
    }
}

void SyncNode::HLANodesFinalization(cModule *parent){

    char str[20];
    for (int i=0;i<NUMBER_OF_HLA_NODES;i++){
        //!Finalize the HLA enable NODES !//
        sprintf(str, "node%d", i);
        Txc0* TX = (Txc0*)parent->getSubmodule(str);
#ifndef NO_HLA
        if(!TX->TerminateNormal)
            TX->NODEHLA->resign();
#endif
    }
}

void SyncNode::initialize(void) {

    NUMBER_OF_HLA_NODES = (int)par("NumberOfHLANodes");
    SYNCH_TIME          = (double)par("SynchTime");

    cMessage* event = new myPacket("reloop");
    scheduleAt(simTime()+SYNCH_TIME, event);

    //! Initialize the SynchNode !//
    HLA_initialization();

    //! initialize the HLA enable nodes !//
    cModule *parent= getParentModule();
    HLANodesInitialization(parent);
#ifndef NO_HLA
    //! As last step, synchronize the SynchNode!//
    HLAGlobalSynch->ReadGlobalSynch("GLOBAL_SYNCHRONIZATION");
#endif
}


void SyncNode::HLA_initialization(void) {
    string fedfile = "Federation.fed";
    string Synchfederation = "GLOBAL_SYNCHRONIZATION" ;
#ifndef NO_HLA
    HLAGlobalSynch = new HLA_OMNET(federate, NUMBER_OF_HLA_NODES,NUMBER_OF_HLA_NODES);
    HLAGlobalSynch->HLAInitialization(Synchfederation,fedfile, true, true);
#endif
    TerminateNormal = false;

    cout <<"\n--------------------  HLA SyncNode initialized"<<"\n";

}
void SyncNode::handleMessage(cMessage *msg){
#ifndef NO_HLA
    if(HLAGlobalSynch->ReadInFinalizeArray()){
        cout<<"Terminate the HLA Synchronization Node\n";
        HLAGlobalSynch->resign();
        TerminateNormal = true;
		endSimulation();
    }
    else{
        cMessage* event = new cMessage("syncstep");
        scheduleAt(simTime()+SYNCH_TIME, event);
        if (msg->isSelfMessage()) {
            HLAGlobalSynch->step();
        }
    }
#endif
}

#ifdef DEBUG_MSG
//-------------------------CRC stuff
void Txc0::CRC_Init()
{
    unsigned short remainder;

    for (int dividend = 0; dividend < 256; ++dividend)
    {
        remainder = dividend;

        for (uint8_t bit = 8; bit > 0; --bit)
        {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            else
                remainder = (remainder >> 1);
        }

        crcTable[dividend] = remainder;
    }
}

unsigned short Txc0:: CRC_Calculate(const uint8_t *message, int nBytes)
{
    uint8_t data;
    unsigned short remainder=0;

    for (int byte = 0; byte < nBytes; ++byte)
    {
        data = message[byte] ^ remainder;
        remainder = crcTable[data] ^ (remainder >> 8);
    }

    return remainder;
}

//-----------------------end CRC stuff
#endif

void Txc0::setPayloadArray(myPacket *msg, uint8_t *payload, int length)
{
    for (int i=0;i<length;i++) msg->setPayload(i, (uint8_t) payload[i]);
}

void Txc0::getPayloadArray(myPacket *msg, uint8_t *payload, int length)
{
    for (int i=0;i<length;i++) payload[i]= msg->getPayload(i);
}

void Txc0::sendCopyOf(myPacket *msg)
{
   // Duplicate message and send the copy.
    cMessage *copy = (cMessage *) msg->dup();
    send(copy, "gate$o");
}



void Txc0::HLA_initialization(int _NUMBER_OF_HLA_NODES) {

    //! -------------- NODE HLA INITIALIZATION -------------- !//
    NUMBER_OF_HLA_NODES = _NUMBER_OF_HLA_NODES;
    federationName = "COSSIM_PROC_NET_NODE" + std::to_string(NODE_NO);
    string fedfile = "Federation.fed";
    TerminateNormal = false;
#ifndef NO_HLA
    NODEHLA = new HLA_OMNET(federate,NODE_NO,NUMBER_OF_HLA_NODES);
    NODEHLA->HLAInitialization(federationName,fedfile, false, false);

    //! -------------- END HLA INITIALIZATION -------------- !//

    cout <<"\n--------------------"<<"For NODE:"<<NODE_NO<< " HLA initialized"<<"\n";
#endif
}


void Txc0::initialize()

{
    char str[20];
    total_bytes_sent=0;
    total_bytes_received=0;
    NODE_NO = (int)par("nodeNo");
    RX_PACKET_TIME = (double)par("RXPacketTime");
    sprintf(str, "Txc%dreloop", NODE_NO);
    cMessage* event = new cMessage(str);
    scheduleAt(simTime()+RX_PACKET_TIME, event);
}


void Txc0::finish()
{

    cModule *parent= getParentModule();
#ifndef NO_HLA
    SyncNode* GEM5SyncNode =  (SyncNode*)parent->getSubmodule("syncnode");
    if(!GEM5SyncNode->TerminateNormal){

        cout<<"FINISH THE OMNET++ EXECUTION IN NODE: "<<NODE_NO<<"\n";

        //!Notify the SynchNode that this HLA node has been deleted !//

        GEM5SyncNode->HLAGlobalSynch->WriteInFinalizeArray(NODE_NO);

        //! Send Empty message to notify the OMNET++ termination !//
        if(!TerminateNormal){
            NODEHLA->sendInteraction(NULL,(uint32_t)0);
            NODEHLA->step(); // <-- Call with the sendInderaction
        }


        if(NODE_NO == NUMBER_OF_HLA_NODES-1){
            GEM5SyncNode->HLAGlobalSynch->step();
            GEM5SyncNode->HLAGlobalSynch->step();

            //! Finalize HLAEnableNodes !//
            GEM5SyncNode->HLANodesFinalization(parent);

            if(GEM5SyncNode->HLAGlobalSynch->ReadInFinalizeArray()){
                cout<<"Terminate the HLA Synchronization Node\n";
                GEM5SyncNode->HLAGlobalSynch->resign();
            }
            else{
                cout<<"Goes something wrong with HLA Termination! PLEASE RESTART THE rtig SERVER BEFORE THE NEXT EXECUTION !!!\n";
            }
        }
    }
#endif

    float bw=(total_bytes_sent/1024)/simTime();
    recordScalar("#Total Bytes Sent", total_bytes_sent);
    recordScalar("#total Bytes Received", total_bytes_received);
    recordScalar("#Simulation Time (sec)", simTime());
    recordScalar("#Bw sent utilization in KBytes per sec",bw );


}



void Txc0::handleMessage(cMessage *msg)
{


     if (msg->isSelfMessage()) {
         cMessage* event = new cMessage("reloop");
         scheduleAt(simTime()+RX_PACKET_TIME, event);

         //---------------Receive packet from GeM5--------------------------
#ifdef NO_HLA
         if(TOGGLE){
             TOGGLE=!TOGGLE;

          EtherTestPacket *Rcvpacket = new EtherTestPacket; //Initialize test packet

#else
       NODEHLA->step();

       if(!NODEHLA->BufferPacketEmpty()){

           EthPacketPtr Rcvpacket = NODEHLA->getPacket();
#endif
             /* ------- Rcvpacket is the Ethernet packet which we get from GEM5 ------- */

#ifdef DEBUG_MSG
             cout << "\n "<<"Node:"<< NODE_NO<<" Receive REAL Packet from GeM5 with length (integer print):"<< Rcvpacket->length<<"\n";
             for(unsigned i=0;i<Rcvpacket->length;i++){
                 cout<<(int)Rcvpacket->data[i]<<" ";
             }
             cout<<"\n";
#endif

             //------- Encapsulate Message for OMNET----------
             if (Rcvpacket->length > 0){ //! Receive Real Packet !//
                 myPacket *msg_gem5 =  new myPacket("fromGEM");
                 msg_gem5->setLength((unsigned int)Rcvpacket->length);
                 setPayloadArray(msg_gem5, Rcvpacket->data, Rcvpacket->length);
#ifndef NO_HLA
                 NODEHLA->clearRcvPacket();
#endif
                 //! -------------- Testing Ethernet -------------- !//

                inet::EthernetIIFrame *eth2Frame=0;
                eth2Frame = new EthernetIIFrame();

                MACAddress sourceMac;
                MACAddress destMac ;

                for (int i=0;i<6;i++) tmp_dst[i]=(unsigned char)Rcvpacket->data[i];
                for (int i=0;i<6;i++) tmp_src[i]=(unsigned char)Rcvpacket->data[i+6];
                tmp_ethertype = (int) ((Rcvpacket->data[12] << 8) + (Rcvpacket->data[13] & 0xFF)  );//-1000000; // 12-13 bits are ethertype bits


                sourceMac.setAddressBytes(tmp_src);
                destMac.setAddressBytes(tmp_dst);
                eth2Frame->setSrc(sourceMac);
                eth2Frame->setDest(destMac);
                eth2Frame->setEtherType(tmp_ethertype);

                myPacket *tmp_packet= new myPacket("tmp_packet");
                unsigned int tmp_packet_length=(unsigned int) (Rcvpacket->length-14);
                tmp_packet->setLength(tmp_packet_length); //only the payload after striping dst, src, ethertype


                total_bytes_sent=total_bytes_sent+tmp_packet_length;


                for (unsigned int i=0;i<tmp_packet_length;i++) tmp_payload[i]= (uint8_t)Rcvpacket->data[i+14];

#ifdef DEBUG_MSG
                //DEBUG
                cout<<"\n DEBUG: From Node:"<< NODE_NO <<"  From Gem to Omnet, ethernet only payload(integer print):"<<"With packet length:"<<tmp_packet_length<<". ";
                for(unsigned i=0;i<tmp_packet_length;i++) cout<<(int) tmp_payload[i]<<" ";
                cout<<"\n";
                cout<<"\n DEBUG: From Node:"<< NODE_NO <<"  From Gem to Omnet, ethertype (hex print): "<<hex<<(int) (Rcvpacket->data[12]) <<","<< hex<<(int)(Rcvpacket->data[13]) <<". ";
                cout.unsetf(ios::hex);
#endif

                setPayloadArray(tmp_packet, tmp_payload, tmp_packet_length);
                eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);

                // --------------- Ethernet (L2) Routing ---------------
                if (L2_Routing) {
                    eth2Frame->encapsulate(tmp_packet);
                    send_packet=true;
                }

                //--------------- IPv4 (L3) Routing ---------------
                else if ((Rcvpacket->data[13]==0x00) and (Rcvpacket->data[12]==0x08) and  Rcvpacket->length>35){

                    send_packet=true;

#ifdef DEBUG_MSG
                    cout<<"\n From Gem to Omnet: Got IP packet from GEM5 "<<" Node:"<< NODE_NO <<"\n";
                    //---------------IP Decaps----------------------------------------

                    cout<<"\n From Gem to Omnet: IP Starting parsing (reported from NODE:):"<< NODE_NO <<"\n";
                    cout<<"\n From Gem to Omnet: Packet length is (reported from NODE:):"<< (int)Rcvpacket->length<< NODE_NO  <<"\n";
#endif

                    //--------------- IP Decapsulation ---------------
                    my_Version=(short)  (Rcvpacket->data[14]<< 8);
                    my_IHL=(short) (Rcvpacket->data[14]& 0xFF);
                    my_TOS=(unsigned char) Rcvpacket->data[15];

                    my_Total_Length=(int) ((Rcvpacket->data[17] << 8) + (Rcvpacket->data[16] & 0xFF));
                    my_Identification=(int) ((Rcvpacket->data[19] << 8) + (Rcvpacket->data[18] & 0xFF));

                    my_Flags_DF=(bool)Rcvpacket->data[20] & 0x02;
                    my_Flags_MF=(bool)Rcvpacket->data[20] & 0x04;
                    my_Fragment_Offset=(int) ((Rcvpacket->data[21] << 5) + (Rcvpacket->data[20]>>3  & 0x1F)); //31

                    my_Time_to_Live= (short) Rcvpacket->data[22];
                    my_Protocol=(int)6;// Rcvpacket->data[23]; //try 6 suppress the icmp casting error in wireless routers

                    for (int i=0;i<2;i++)my_Header_Checksum[i]=(unsigned char)Rcvpacket->data[i+24];

                    for (int i=0;i<4;i++)my_source_ip[i]=(int) Rcvpacket->data[i+26];
                    for (int i=0;i<4;i++)my_dest_ip[i]=(int) Rcvpacket->data[i+30];

#ifdef DEBUG_MSG
                    //for (int i;i<tmp_packet_length-35;i++)IP_data[i]=(unsigned char)Rcvpacket->data[i+35];
                    cout<<"\n From Gem to Omnet: IP parsing Starting... (reported from NODE:):"<< NODE_NO <<"\n";
                    cout<<"\n From Gem to Omnet: Source IP:";
                    for (int i=0;i<4;i++) cout<< (int)my_source_ip[i];
                    cout <<"\n From Gem to Omnet: Destination IP:";
                    for (int i=0;i<4;i++) cout<< (int)my_dest_ip[i];
                    cout<<"\n From Gem to Omnet: IP parsing complete. (reported from NODE:):"<< NODE_NO <<"\n";
#endif

                    //--------------- END IP Decapsulation ---------------

                    //----------------- Build Datagram ------------
                    datagram= new IPv4Datagram("test");
                    icmp = new ICMPMessage("test");
                    datagram->setByteLength(IP_HEADER_BYTES);
                    datagram->encapsulate(tmp_packet); //transportPacket for us is the ethernet payload


                    IPv4Address IP_src,IP_dest;
                    // inet/src/inet/networklayer/contract/ipv4/IPv4Address.cc
                    IP_src.set(my_source_ip[0],my_source_ip[1],my_source_ip[2],my_source_ip[3]);
                    IP_dest.set(my_dest_ip[0],my_dest_ip[1],my_dest_ip[2],my_dest_ip[3]);

                    //------ set datagram ---
                    datagram->setVersion(my_Version);
                    datagram->setHeaderLength(my_IHL);
                    datagram->setTypeOfService(my_TOS);
                    datagram->setTotalLengthField(my_Total_Length);
                    datagram->setIdentification(my_Identification);

                    datagram->setDontFragment(my_Flags_DF ); // bit 2/8
                    datagram->setMoreFragments(my_Flags_MF); //bit 3/8
                    datagram->setFragmentOffset(my_Fragment_Offset);// bits: 4,5,6,7,8 + 8 = 13 bits
                    datagram->setTimeToLive(my_Time_to_Live);
                    datagram->setTransportProtocol(my_Protocol);
                    //Header Checksum omitted, modeled by error bit of packets

                    datagram->setSrcAddress(IP_src);
                    datagram->setDestAddress(IP_dest);
                    datagram->setByteLength(IP_HEADER_BYTES);

                    inet::Ieee802Ctrl *etherctrl;
                    etherctrl = new Ieee802Ctrl();

                    etherctrl->setSrc(sourceMac);
                    etherctrl->setDest(destMac);
                    etherctrl->setEtherType(tmp_ethertype);

                    if (datagram->getByteLength() > MAX_ETHERNET_DATA_BYTES)
                        throw cRuntimeError("packet from higher layer (%d bytes) exceeds maximum Ethernet payload length (%d)", (int)datagram->getByteLength(), MAX_ETHERNET_DATA_BYTES);

                    eth2Frame->encapsulate(datagram);
#ifdef DEBUG_MSG
                    cout<<"\n From Gem to Omnet: Datagram Encapsulated! (reported from NODE:):"<< NODE_NO <<"\n";
#endif
                }

                //--------------- IPv6 (L3) Routing (Not supported) ---------------
                else if ((Rcvpacket->data[13]==0xdd) and (Rcvpacket->data[12]==0x86)){
                    //IPV6 packet, don't send it
                    send_packet=false;
                }

                //--------------- ARP Request/Reply (L3) Routing ---------------
                else if ((Rcvpacket->data[13]==0x06) and (Rcvpacket->data[12]==0x08))
                {
#ifdef DEBUG_MSG
                    cout<<"\n From Gem to Omnet: Got ARP packet from GEM5 "<<" Node:"<< NODE_NO <<"\n";
#endif
                    ARPPacket* tmp_arp_packet  =new ARPPacket("MyARP_Packet");
                    tmp_arp_packet->setName("GEM5_arp");


                    tmp_arp_packet->encapsulate(tmp_packet); //To make GEM5 decapsulation easier (OMNET don't need this)
                    tmp_arp_packet->setByteLength(ARP_HEADER_BYTES);
                    //FIX accordingly
                    //20, 21 :ARP opcode
                    tmp_arp_packet->setOpcode((int) Rcvpacket->data[21]);
                    if (Rcvpacket->data[21]==2)tmp_arp_packet->setName("GEM5_arpREPLY");
                    else if (Rcvpacket->data[21]==1)tmp_arp_packet->setName("GEM5_arpREQUEST");
#ifdef DEBUG_MSG
                    cout<<"\n  --------- tmp_arp_packet->setOpcod --------:"<< (int) Rcvpacket->data[21] <<"From Node: "<< NODE_NO <<"\n";
#endif

                    tmp_arp_packet->setDestMACAddress(destMac);  //same as Ethernet no need to parse it from ARP
                    tmp_arp_packet->setSrcMACAddress(sourceMac); //same as Ethernet no need to parse it from ARP

                    IPv4Address IP_src,IP_dest;

                    // -------------Check Those:
                    for (int i=0;i<4;i++)my_source_ip[i]=(int) Rcvpacket->data[i+28];
                    for (int i=0;i<4;i++)my_dest_ip[i]=(int) Rcvpacket->data[i+38];

                    IP_src.set(my_source_ip[0],my_source_ip[1],my_source_ip[2],my_source_ip[3]);
                    IP_dest.set(my_dest_ip[0],my_dest_ip[1],my_dest_ip[2],my_dest_ip[3]);

#ifdef DEBUG_MSG
                    cout<<"\n From Gem to Omnet: ARP Source IP:";
                    for (int i=0;i<4;i++) cout<< (int)my_source_ip[i]<<".";
                    cout <<"\n From Gem to Omnet: ARP Destination IP:";
                    for (int i=0;i<4;i++) cout<< (int)my_dest_ip[i]<<".";
#endif
                    tmp_arp_packet->setDestIPAddress(IP_dest);
                    tmp_arp_packet->setSrcIPAddress(IP_src);

                    eth2Frame->encapsulate( tmp_arp_packet);///was tmp_packet
                    send_packet=true;

                }

                else{ //Handle packet as raw payload
                     eth2Frame->encapsulate(tmp_packet);///was tmp_packet
#ifdef DEBUG_MSG
                     cout<<"\n From Gem to Omnet: tmp_packet Encapsulated! (reported from NODE:):"<< NODE_NO <<"\n";
#endif
                     send_packet=true;
                }
                //End of Protocol cases

                if (eth2Frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES){
                    eth2Frame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
                }

                if (send_packet){
#ifdef DEBUG_MSG
                    cout<<"\n  ---------Sending Packet through OMNET NETWORK---------:"<<"From Node: "<< NODE_NO <<"\n";
#endif
                    send(eth2Frame, "gate$o");
                }
#ifdef DEBUG_MSG
                else  cout<<"\n  --------- Packet NOT SENT! (probably IPV6 packet) through OMNET NETWORK---------:"<<"From Node: "<< NODE_NO <<"\n";
#endif
                //! -------------- Testing Ethernet -------------- !//

             }
             else{ //! Receive Empty Packet -- Terminate the HLA Connection !//
#ifdef DEBUG_MSG
                 cout<<"\nNode:"<< NODE_NO<<" Terminate HLA Connection\n";
#endif

#ifndef NO_HLA
                 NODEHLA->clearRcvPacket();
                 NODEHLA->resign();

                 cancelEvent(event);

                 //!Notify the SynchNode that this HLA node has been deleted !//
                 cModule *parent= getParentModule();
                 SyncNode* GEM5SyncNode =  (SyncNode*)parent->getSubmodule("syncnode");
                 GEM5SyncNode->HLAGlobalSynch->WriteInFinalizeArray(NODE_NO);

                 TerminateNormal = true;
#endif
             }

            //-------END Encapsulate Message for OMNET----------
       }

     }
     else { //------- Receive real message from OMNET++ network (other OMNET++ node) -------
         bubble("-----just got a real message from OMNET++ network------");

#ifdef DEBUG_MSG
         cout<<"\n ---";
         cout<<"\n  ---------Receiving Packet from OMNET NETWORK---------:"<<"From Node: "<< NODE_NO <<"\n";
         cout<<"\n ---";
#endif
         TOGGLE=!TOGGLE;
         EtherFrame *eth2Frame2 = nullptr;

         if (dynamic_cast<EthernetIIFrame *>(msg) != nullptr){

             cout<<"\n EthernetIIFrame taken";
             EthernetIIFrame  *eth2Frame = new EthernetIIFrame();
             eth2Frame =(EthernetIIFrame*)(msg);
             ethertype=eth2Frame->getEtherType();
             eth2Frame2=eth2Frame;

#ifdef DEBUG_MSG
             cout<<"\n From OMNET to GEM5: Decoding Ethernet:"<<"From Node: "<< NODE_NO <<" Ethernet packet Name: "<<eth2Frame->getName() <<" "<<"\n";
#endif
             ethertype_dec[0]=(uint8_t) ((ethertype  >> 8) & 0xFF);
             ethertype_dec[1]=(uint8_t)(ethertype  & 0xFF);

         }
         else if (dynamic_cast<EtherFrameWithSNAP *>(msg) != nullptr)
         {
             cout<<"\n EtherFrameWithSNAP taken";
             EtherFrameWithSNAP  *snapFrame = new EtherFrameWithSNAP();
             snapFrame =(EtherFrameWithSNAP*)(msg);
             snap_local_code=snapFrame->getLocalcode();

             eth2Frame2=snapFrame;
         }

         MACAddress sourceMac = eth2Frame2->getSrc();   unsigned char tmp_src[6];
         MACAddress destMac   = eth2Frame2->getDest();  unsigned char tmp_dst[6];

         sourceMac.getAddressBytes(tmp_src);
         destMac.getAddressBytes(tmp_dst);

#ifdef DEBUG_MSG
         cout<<"\n From OMNET to GEM5: for packet type: From Node:"<< NODE_NO <<" ethertype_dec[0],[1]:" <<(int)ethertype_dec[0]<<","<< (int)ethertype_dec[1]<<".  ";
         cout<<"\n From OMNET to GEM5: IF for GEM5 valid packet taken ---------:"<<"From Node: "<< NODE_NO <<"\n";
#endif
         myPacket *from_eth_packet_pt;
         unsigned int from_eth_payload_length;

         // --------------- Ethernet (L2) Routing ---------------
         if (L2_Routing) {
             from_eth_packet_pt=   (myPacket*) (eth2Frame2->decapsulate());
             from_eth_payload_length=from_eth_packet_pt->getLength();
             getPayloadArray(from_eth_packet_pt,from_eth_payload,from_eth_payload_length);
        }

        //--------------- IPv4 (L3) Routing ---------------
        else if ((ethertype_dec[1]==0x00) and (ethertype_dec[0]==0x08) ){
#ifdef DEBUG_MSG
          cout<<"\n From OMNET to GEM5: Got IP packet from NETWORK From Node:"<< NODE_NO <<"\n";
#endif

          IPv4Datagram *my_datagram = (IPv4Datagram*) (eth2Frame2->decapsulate());

          from_eth_packet_pt=  (myPacket*) (my_datagram->decapsulate());
          from_eth_payload_length=from_eth_packet_pt->getLength();
          getPayloadArray(from_eth_packet_pt,from_eth_payload,from_eth_payload_length);

          //--------Testing IP Checksum--------
          //http://www.thegeekstuff.com/2012/05/ip-header-checksum

#ifdef DEBUG_MSG
          uint16_t ip_header[10];
          for (int i=0;i<10;i++) ip_header[i]= (uint16_t) (  (from_eth_payload[i+1] <<8)  + (from_eth_payload[i] & 0xFF)) ;


          cout << "\n "<<"------> From OMNET to GEM5: Previous checksum result from  Node (hex print):" <<NODE_NO <<"="<< hex << (int)from_eth_payload[10] <<hex<< (int)from_eth_payload[11]<<"\n";
          cout.unsetf(ios::hex);
          cout << "\n "<<"------> From OMNET to GEM5: Previous checksum result from  Node (2 integers print):" <<NODE_NO <<"="<<   (int)from_eth_payload[10]  <<","<< (int)from_eth_payload[11]<<"\n";
          cout.unsetf(ios::hex);
          cout << "\n "<<"------> From OMNET to GEM5: Previous checksum result from  Node (single integer print from uint16_t..ip_header[5] ):" <<NODE_NO <<"="<<   (int)ip_header[5]    <<"\n";
          cout.unsetf(ios::hex);

          ip_header[5]= (uint16_t) 0; //zero old checksum before recompute

          uint16_t  myIP_checksum= NetIpChecksum(ip_header,10);
          uint8_t myIP_checksum_0= (uint8_t)myIP_checksum & 0xFF;
          uint8_t myIP_checksum_1= (uint8_t)myIP_checksum >>8 & 0xFF;

          cout << "\n "<<"------> From OMNET to GEM5: myIP_checksum result from  Node (two integers print):" <<NODE_NO <<"="<<  (int)myIP_checksum_0 << ","<< (int) myIP_checksum_1<<"\n";
          cout.unsetf(ios::hex);
#endif
          //---------End Testing IP Checksum--------

        }
        //--------------- IPv6 (L3) Routing (Not supported) ---------------
        else if ((ethertype_dec[1]==0xdd) and (ethertype_dec[0]==0x86)){ //We don't need this as this packet will never be sent from the other end
#ifdef DEBUG_MSG
          cout<<"\n From OMNET to GEM5: Got IPV6 packet do nothing from NETWORK From Node: "<< NODE_NO <<"\n";
#endif
          //IPV6 packet, don't send it
        }

        //--------------- ARP Request/Reply (L3) Routing ---------------
        else if ((ethertype_dec[1]==0x06) and (ethertype_dec[0]==0x08)){ //UNIVERSAL ARP request/Reply
#ifdef DEBUG_MSG
            cout<<"\n From OMNET to GEM5: Got ARP packet from NETWORK From Node: "<< NODE_NO <<"\n";
#endif
             string s = typeid(eth2Frame2->decapsulate()).name();
#ifdef DEBUG_MSG
             cout<<"\n From OMNET to GEM5: Got ARP packet from NETWORK From Node: "<< NODE_NO << "With class name:"<<s<<"\n";
#endif
             ARPPacket* my_arp_packet  =(ARPPacket*) (eth2Frame2->decapsulate());

             MACAddress sourceMac;
             MACAddress destMac ;
             IPv4Address IP_src,IP_dest;

             sourceMac= my_arp_packet->getSrcMACAddress();
             destMac= my_arp_packet->getDestMACAddress();

             // inet/src/inet/networklayer/contract/ipv4/IPv4Address.cc
             IP_src= my_arp_packet->getSrcIPAddress();
             for (int i=0;i<4;i++) my_source_ip[i]= IP_src.getDByte(i);

             IP_dest= my_arp_packet->getDestIPAddress();
             for (int i=0;i<4;i++) my_dest_ip[i]= IP_dest.getDByte(i);

             int opcode=my_arp_packet->getOpcode();

#ifdef DEBUG_MSG
             cout<<"\n From Omnet to Gem  : ARP Source IP:";
             for (int i=0;i<4;i++) cout<< (int)my_source_ip[i]<<".";
             cout <<"\n From Omnet to Gem  : ARP Destination IP:";
             for (int i=0;i<4;i++) cout<< (int)my_dest_ip[i]<<".";
#endif
             //from_eth_payload,
             from_eth_payload_length=46; ///building raw arp packet for GEM
             //Serializing GEM5 compatible packet
             from_eth_payload[0]=(uint8_t) 0; //Hardware type (Ethernet)
             from_eth_payload[1]=(uint8_t) 1; //Hardware type (Ethernet)

             from_eth_payload[2]=(uint8_t) 8; //Protocol type (IP)
             from_eth_payload[3]=(uint8_t) 0; //Protocol type (IP)
             from_eth_payload[4]=(uint8_t) 6; //Hardware size
             from_eth_payload[5]=(uint8_t) 4; //Protocol size

             from_eth_payload[6]=(uint8_t) 0; //Opcode (reply)
             from_eth_payload[7]=(uint8_t) opcode; //Opcode (reply)

             for (int i=0;i<6;i++)  from_eth_payload[i+8]= (uint8_t)tmp_src[i]; //8-13: Source MAC address

             for (int i=0;i<4;i++)  from_eth_payload[i+14]= (uint8_t)my_source_ip[i]; //14-17

             for (int i=0;i<6;i++)  from_eth_payload[i+18]= (uint8_t)tmp_dst[i]; //18-23: Destination MAC address

             for (int i=0;i<4;i++)  from_eth_payload[i+24]= (uint8_t) my_dest_ip[i]; //24-27

             ///--------   Ending ARP ----------------

             for (int i=28;i<46;i++) from_eth_payload[i]=(uint8_t)0; //zero padding for the rest of the ethernet packet

             ethertype_dec[0]=(uint8_t)8; //Type for ARP protocol
             ethertype_dec[1]=(uint8_t)6; //Type for ARP protocol
#ifdef DEBUG_MSG
             cout<<"\n";
             cout<<"\n From OMNET to GEM5  : UNVERSAL ARP From Node: "<< NODE_NO << " opcode: "<<opcode<<"\n";
             cout<<"\n From Omnet to Gem5  : UNVERSAL ARP Source MAC:";
             for (int i=0;i<6;i++) cout<< (int)tmp_src[i] <<".";
             cout<<"\n From Omnet to Gem5  : UNVERSAL ARP Destination MAC:";
             for (int i=0;i<6;i++) cout<< (int)tmp_dst[i]<<".";
             cout<<"\n";
#endif
        }

        //------------------------------------END ADAPT ------------------------

        else
        {
#ifdef DEBUG_MSG
            cout<<"\n  From OMNET to GEM5: ---------Other type Decapsulated from OMNET NETWORK---------:"<<"From Node: "<< NODE_NO <<"\n";
#endif
            from_eth_packet_pt=   (myPacket*) (eth2Frame2->decapsulate());
            from_eth_payload_length=from_eth_packet_pt->getLength();
            getPayloadArray(from_eth_packet_pt,from_eth_payload,from_eth_payload_length);
        }
#ifdef DEBUG_MSG
         cout<<"\n From OMNET to GEM5: DEBUG: From Node:"<< NODE_NO <<" Ethernet only payload (integer print):"<<"With payload length:"<<from_eth_payload_length<<". ";
         for(unsigned i=0;i<from_eth_payload_length;i++) cout<<(int) from_eth_payload[i]<<" ";
         cout<<"\n";
         //Logs
#endif
         total_bytes_received=total_bytes_received+from_eth_payload_length;

         //dest, src ethernet
         for (int i=0;i<6;i++) total_payload[i]=(uint8_t)tmp_dst[i];
         for (int i=0;i<6;i++) total_payload[i+6]=(uint8_t)tmp_src[i];
         //ethertype

         total_payload[12]= ethertype_dec[0];//(uint8_t)ethertype2    & 0xFF;
         total_payload[13]= ethertype_dec[1];//(uint8_t) (ethertype2>>8 & 0xFF);


         for (unsigned i=0;i<from_eth_payload_length;i++) total_payload[i+14]=(uint8_t)from_eth_payload[i];

#ifdef DEBUG_MSG
         cout << "\n "<<"------From OMNET to GEM5: Node:"<< NODE_NO<<" SEND Packet into GeM5 with total length:"<< (int)(from_eth_payload_length+14)<<"\n";
         for(unsigned i=0;i<from_eth_payload_length+14;i++){
           cout<<(int)total_payload[i]<<" ";
         }
         cout<<"\n";

         CRC_Init();
         unsigned short CRC_result= CRC_Calculate(total_payload, (int)from_eth_payload_length+14);
         cout << "\n "<<"From OMNET to GEM5: CRC result from  Node:(hex print)" <<NODE_NO <<"="<< hex <<CRC_result<<"\n";
         cout.unsetf(ios::hex);
#endif

#ifndef NO_HLA
        NODEHLA->sendInteraction(total_payload,from_eth_payload_length+14); //with ethernet support


        //NODEHLA->sendInteraction(payload_temp,length_temp);
        NODEHLA->step(); // <-- Call with the sendInderaction (GEM5 Send Function)
#endif
     }
}


Define_Module(Txc0);
Define_Module(SyncNode);

}; // end namespace

#ifdef NO_HLA
EtherTestPacket::EtherTestPacket(){
 // uint8_t data2[]={0,144,0,0,0,1,0,144,0,0,0,0,8,0,69,0,0,84,146,63,64,0,64,1,148,103,10,0,0,1,10,0,0,2,8,0,115,52,185,2,0,0,115,200,88,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  //length=90;

   //uint8_t data2[]={255,255,255,255,255,255,0,144,0,0,0,0,8,6,0,1,8,0,6,4,0,1,0,144,0,0,0,0,10,0,0,1,0,0,0,0,0,0,10,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
   //length=60; //arp request packet packet, ethtype 8,6

   // uint8_t data2[]="\xff\xff\xff\xff\xff\xff\x00\x90\x00\x00\x00\x00\x08\x06\x00\x01\x08\x00\x06\x04\x00\x01\x00\x90\x00\x00\x00\x00\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    //length=60; //arp request packet packet, ethtype 8,6

    //uint8_t data2[]={0,144,0,0,0,1,0,144,0,0,0,0,8,0,69,0,0,84,146,63,64,0,64,1,148,103,10,0,0,1,10,0,0,2,8,0,115,52,185,2,0,0,115,200,88,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //length=98; //IPV4 packet

    uint8_t data2[]={10,170,0,0,0,4,0,144,0,0,0,0,8,0,69,0,0,84,196,181,64,0,64,1,94,235,10,0,2,3,10,0,1,6,8,0,216,156,188,2,0,0,161,95,194,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    length=98;

  // int data2[]={51,51,0,0,0,2,0,144,0,0,0,1,134,221,96,0,0,0,0,16,58,255,254,128,0,0,0,0,0,0,2,144,0,255,254,0,0,1,255,2,0,0,0,0,0,0,0,0,0,0,0,0,0,2,133,0,122,12,0,0,0,0,1,1,0,144,0,0,0,1};
  //EtherTestPacket::length=70; // IPV6 packet, ethtype 86,dd

  data= new uint8_t[length];
  for (int i=0; i<length; i++) data[i]=(uint8_t)data2[i];
}
#endif



