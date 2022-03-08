/* ----------------------------------------------------------------------------
 * COSSIM - HLANode
 * Copyright (c) 2018, H2020 COSSIM.
 * Copyright (c) 2018, Telecommunications Systems Institute.
 * 
 * Author: Nikolaos Tampouratzis, ntampouratzis@isc.tuc.gr
 * ----------------------------------------------------------------------------
 *
*/

#ifndef CERTI_HLA_OMNET_HH
#define CERTI_HLA_OMNET_HH


#include "RTI.hh"
#include "NullFederateAmbassador.hh"
#include "fedtime.hh"
#include "certi.hh"

#include <vector>

#include <queue>

#include <fstream>
#include <limits>


#include <memory>


//! --- CERTI INITIALIZATION IP --- !//
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

//! Server Functions !//
#define CREATE 0
#define REMOVE 1
#define WRITE 2
#define READ 3
#define READ_GLOBAL 4
#define CLOSE_SERVER 5

typedef struct HLAInitializationRequests{
 int type;
 char name[30];
 int  node;
}HLAInitializationRequest;
//! --- END CERTI INITIALIZATION IP --- !//


/*
 * Reference counted class containing ethernet packet data
 */
class EthPacketData
{
  public:
    /*
     * Pointer to packet data will be deleted
     */
    uint8_t *data;

    /*
     * Length of the current packet
     */
    unsigned length;

  public:
    EthPacketData()
        : data(NULL), length(0)
    { }
    
    explicit EthPacketData(unsigned size)
        : data(new uint8_t[size]), length(0)
    { }

    ~EthPacketData() { if (data) delete [] data; }

};

typedef std::shared_ptr<EthPacketData> EthPacketPtr;


class HLA_OMNET : public NullFederateAmbassador
{
public:
  HLA_OMNET(std::string, int, int);
  virtual ~HLA_OMNET() throw (RTI::FederateInternalError);


  //virtual void declare();
    virtual void join(std::string, std::string);
    virtual void pause();
    virtual void pause_friend();
    virtual void publishAndSubscribeSend();
    virtual void publishAndSubscribeReceive();
    virtual void resign();
    virtual void setTimeRegulation(bool constrained, bool regulating);
    virtual void step();
    virtual void synchronize();
    virtual void tick();
    void tick2();
    virtual void HLAInitialization(std::string federation, std::string fedfile, bool start_constrained, bool start_regulating);
    virtual EthPacketPtr getPacket();
    virtual void clearRcvPacket();
    virtual bool BufferPacketEmpty();
    void ReadGlobalSynch(std::string federation);
    
    void WriteInFinalizeArray(int node);

    bool ReadInFinalizeArray();

    //! This array is used to inform the GlobalSynch to close the HLA connection !//
    bool * FinalizeArray;

    //! --- CERTI INITIALIZATION IP --- !//
    bool RequestFunction(HLAInitializationRequest rqst);
    bool FirstConnectionWithHLAInitialization;
    //! --- END CERTI INITIALIZATION IP --- !//

    unsigned long ID ; // object handle
    
    std::queue<EthPacketPtr> packetBuffer;
        
    RTI::FederateHandle getHandle() const ;

    // Callbacks
    void announceSynchronizationPoint(const char *label, const char *tag)
        throw (RTI::FederateInternalError);

    void federationSynchronized(const char *label)
        throw (RTI::FederateInternalError);

    void timeAdvanceGrant(const RTI::FedTime& theTime)
        throw (RTI::FederateInternalError, RTI::TimeAdvanceWasNotInProgress,
	       RTI::InvalidFederationTime);

    void discoverObjectInstance(RTI::ObjectHandle theObject,
                                RTI::ObjectClassHandle theObjectClass,
                                const char *theObjectName)
        throw (RTI::FederateInternalError, RTI::ObjectClassNotKnown, RTI::CouldNotDiscover);
	
	
    void reflectAttributeValues(RTI::ObjectHandle, const RTI::AttributeHandleValuePairSet &, const char *) 
	throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateOwnsAttributes,
	       RTI::FederateInternalError);


    void receiveInteraction(RTI::InteractionClassHandle theInteraction,
                                const RTI::ParameterHandleValuePairSet& theParameters,
                                const RTI::FedTime& theTime, const char *theTag,
                                RTI::EventRetractionHandle theHandle)
            throw (RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown,
                   RTI::InvalidFederationTime, RTI::FederateInternalError);

        void receiveInteraction(RTI::InteractionClassHandle,
                                const RTI::ParameterHandleValuePairSet &,
                                const char *)
            throw (RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown,
               RTI::FederateInternalError) { };

    void removeObjectInstance(RTI::ObjectHandle theObject, const RTI::FedTime& theTime,
			      const char *theTag,
			      RTI::EventRetractionHandle theHandle)
	throw (RTI::ObjectNotKnown, RTI::InvalidFederationTime, RTI::FederateInternalError);

    void removeObjectInstance(RTI::ObjectHandle, const char *)
	throw (RTI::ObjectNotKnown, RTI::FederateInternalError) { };

    
    virtual void sendInteraction(uint8_t* , uint32_t);

    void sendUpdate(double, double, int, RTI::ObjectHandle);

    bool getCreator(){return creator;};
  
  
protected:
 
    RTI::RTIambassador rtiamb ;

    std::string federateName ;
    std::string federationName ;

    int Node;
    int TotalNodes;
    
    RTI::FederateHandle handle ;
    bool creator ;
    long nbTicks ;

    bool regulating ;
    bool constrained ;
    RTIfedTime localTime ;
    const RTIfedTime TIME_STEP ;
    
    RTIfedTime next_step;
        
    
    bool paused ;
    bool granted ;
    
    
    //Node to OMNET++
    RTI::InteractionClassHandle NODE_TO_OMNET_ID ;
    RTI::ParameterHandle PacketDataToOmnetID ;
    RTI::ParameterHandle PacketLengthToOmnetID ;
    
    //Node to GEM5++
    RTI::InteractionClassHandle NODE_TO_GEM5_ID ;
    RTI::ParameterHandle PacketDataToGem5ID ;
    RTI::ParameterHandle PacketLengthToGem5ID ;
};


#endif
