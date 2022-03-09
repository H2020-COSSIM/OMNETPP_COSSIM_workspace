/* ----------------------------------------------------------------------------
 * COSSIM - HLANode
 * Copyright (c) 2018, H2020 COSSIM.
 * Copyright (c) 2018, Telecommunications Systems Institute.
 * 
 * Author: Nikolaos Tampouratzis, ntampouratzis@isc.tuc.gr
 * ----------------------------------------------------------------------------
 *
*/

#include <config.h>

#include "HLA_OMNET.hh"
#include "PrettyDebug.hh"
#include "MessageBuffer.hh"

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include <memory>
#include <iostream>
#include <cstdlib>
#include <cassert>

using std::auto_ptr ;
using std::string ;
using std::endl ;
using std::cout ;
using std::vector ;

using namespace std;



static PrettyDebug D("HLA_OMNET", __FILE__);

// ----------------------------------------------------------------------------
/** Constructor
 */
HLA_OMNET::HLA_OMNET(std::string federate_name, int node, int _TotalNodes)
    : rtiamb(),
      federateName(federate_name),
      Node(node),
      TotalNodes(_TotalNodes),
      handle(0),
      creator(false),
      nbTicks(0),
      regulating(false),
      constrained(false),
      localTime(0.0),
      TIME_STEP(1.0)
{

    if (federate_name.compare("SYNCH_OMNET") == 0){
      //! --- CERTI INITIALIZATION IP --- !//
      /****** Create COSSIM Signal Files (HLA initialization) *****/
      HLAInitializationRequest tmp;
      tmp.type = CREATE;
      tmp.node = TotalNodes;
      strcpy(tmp.name, "OmnetToGem5Signal");
      RequestFunction(tmp);

      strcpy(tmp.name, "Gem5ToOmnetSignal");
      RequestFunction(tmp);

      tmp.node = TotalNodes+1;
      strcpy(tmp.name, "GlobalSynchSignal");
      RequestFunction(tmp);
      /****** END Create COSSIM Signal Files (HLA initialization) *****/

      /****** Create APOLLON Signal Files (HLA initialization) *****/
      /* REMOVE ANY OLD CONFIGURATION */
      tmp.type = REMOVE;
      strcpy(tmp.name, "PtolemyToGem5Signal");
      RequestFunction(tmp);

      strcpy(tmp.name, "Gem5ToPtolemySignal");
      RequestFunction(tmp);
      /* END REMOVE ANY OLD CONFIGURATION */

      tmp.type = CREATE;
      tmp.node = TotalNodes;
      strcpy(tmp.name, "PtolemyToGem5Signal");
      RequestFunction(tmp);

      strcpy(tmp.name, "Gem5ToPtolemySignal");
      RequestFunction(tmp);
      /****** END Create APOLLON Signal Files (HLA initialization) *****/
      //! --- END CERTI INITIALIZATION IP --- !//
    }


}

// ----------------------------------------------------------------------------
/** Destructor
 */
HLA_OMNET::~HLA_OMNET()
{
}


// ============================================================================
//          ------------ INITIALIZATION FUNCTIONS ------------
// ============================================================================

//! GLOBAL SYNCH CHANGES !//

void 
HLA_OMNET::HLAInitialization(std::string federation, std::string fedfile, bool start_constrained, bool start_regulating){
  
  HLAInitializationRequest tmp;


  // Joins federation
  this->join(federation, fedfile);
  
  if (federation.compare("GLOBAL_SYNCHRONIZATION") != 0){
      /* Write "1" in Node+1 line of OmnetToGem5Signal Array */
      tmp.type = WRITE;
      strcpy(tmp.name, "OmnetToGem5Signal");
      tmp.node = Node;
      RequestFunction(tmp);
  }
  
  // Continue initialisation...
  this->pause();
  this->publishAndSubscribeReceive();
  this->publishAndSubscribeSend();
  
  this->setTimeRegulation(start_constrained, start_regulating);
  this->tick();
  
  if (federation.compare("GLOBAL_SYNCHRONIZATION") != 0){
      //! Wait until GEM5 federation will be joined !//
      while(1){
          tmp.type = READ;
          strcpy(tmp.name, "Gem5ToOmnetSignal");
          tmp.node = Node;
          bool ret = RequestFunction(tmp);
          if(ret){ break;}
      }
      this->synchronize();
  }

  if (federation.compare("GLOBAL_SYNCHRONIZATION") == 0){

      /* Remove previous GEM5 statistics */
      system("rm -rf $GEM5/node*");
      system("rm -rf $GEM5/McPat/mcpatNode*");
      system("rm -rf $GEM5/McPat/mcpatOutput*");
      system("rm -rf $GEM5/McPat/energy*");

      //! Execute the Gem5.sh !//
      char* pPath = getenv ("GEM5");
      char scriptName[7] = "run.sh";
      char * result = (char *) malloc(1 + strlen(pPath)+ strlen(scriptName) );
      strcpy(result, pPath);
      strcat(result, "/");
      strcat(result, scriptName);
      system(result);
  }

}

void
HLA_OMNET::ReadGlobalSynch(std::string federation){
    if (federation.compare("GLOBAL_SYNCHRONIZATION") == 0){
          assert(creator); //! OMNET must be always creator in GLOBAL_FEDERATION!//

          FinalizeArray = (bool *) malloc(TotalNodes*sizeof(bool));
          for(int i=0;i<TotalNodes;i++){
              FinalizeArray[i] = false;
          }


          HLAInitializationRequest tmp;
          while(1){
              tmp.type = READ_GLOBAL;
              strcpy(tmp.name, "GlobalSynchSignal");
              tmp.node = TotalNodes;
              bool ret = RequestFunction(tmp);
              if(ret){ break;}
          }
          this->synchronize();
     }
    else{
        cout<<"ReadGlobalSynch is called only from GlobalSynch\n";
        assert(true);
    }
}

void
HLA_OMNET::WriteInFinalizeArray(int node){
    FinalizeArray[node] = true;
}

bool
HLA_OMNET::ReadInFinalizeArray(){
    for(int i=0;i<TotalNodes;i++){
        if(FinalizeArray[i] == false)
            return false;
    }
    return true;
}



/** Get the federate handle
 */
RTI::FederateHandle
HLA_OMNET::getHandle() const
{
    return handle ;
}

// ----------------------------------------------------------------------------
/** Join the federation
    \param federation_name Federation name
    \param fdd_name Federation designator (.fed file)
 */
void
HLA_OMNET::join(std::string federation_name, std::string fdd_name)
{

    federationName = federation_name ;

    // create federation
    try {
        rtiamb.createFederationExecution(federation_name.c_str(),
                                         fdd_name.c_str());
        Debug(D, pdInit) << "Federation execution created." << std::endl;
        creator = true ;
    }
    catch (RTI::FederationExecutionAlreadyExists& e) {
        printf("HLA_OMNET Note : %s Reason is : %s. OK I can join it\n",e._name,e._reason);
        Debug(D, pdInit) << "Federation execution already created." << std::endl;
    }
    catch (RTI::CouldNotOpenFED& e) {
        printf("HLA_OMNET ERROR : %s Reason is : %s\n",e._name,e._reason);
        Debug(D, pdExcept) << "HLA_OMNET : Could not use FED file." << std::endl;
        delete &rtiamb ; // Says RTIA to stop.
        exit(0);
    }

    // join federation
    bool joined = false ;
    int nb = 5 ;

    while (!joined && nb > 0) {
        nb-- ;
        try {
            handle = rtiamb.joinFederationExecution(federateName.c_str(),
                                                    federation_name.c_str(),
                                                    this);
            joined = true ;
            break ;
        }
        catch (RTI::FederateAlreadyExecutionMember& e) {
            Debug(D, pdExcept) << "Federate " << federateName.c_str()
                        << "already exists." << endl ;

            throw ;
        }
        catch (RTI::FederationExecutionDoesNotExist& e) {
            Debug(D, pdExcept) << "Federate " << federateName << ": FederationExecutionDoesNotExist." << std::endl;
            // sleep(1);
        }
        catch (RTI::Exception& e) {
            Debug(D, pdExcept) << "Federate " << federateName << " : Join Federation Execution failed : " << &e
                                           << std::endl;
            throw ;
        }
    }
}



// ----------------------------------------------------------------------------
/** resign the federation
 */
void
HLA_OMNET::resign()
{
    try {
        rtiamb.deleteObjectInstance(ID, localTime, "DO");
        Debug(D, pdRegister) << "Local object deleted from federation." << std::endl;
    }
    catch (RTI::Exception &e) {
        Debug(D, pdExcept) << "**** Exception delete object : " << &e << std::endl;
    }

    Debug(D, pdTerm) << "Local objects deleted." << std::endl;

    setTimeRegulation(false, false);

    try {
        rtiamb.resignFederationExecution(
            RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
        Debug(D, pdTerm) << "Just resigned from federation" << std::endl;
    }
    catch (RTI::Exception &e) {
        Debug(D, pdExcept) << "** Exception during resignFederationExecution by federate" << std::endl;
    }
    // Destroy the federation

    if (creator) {
        for (;;) {
            tick();
            try {
                Debug(D, pdTerm) << "Asking from federation destruction..." << std::endl;
                rtiamb.destroyFederationExecution(federationName.c_str());

                Debug(D, pdTerm) << "Federation destruction granted." << std::endl;
                break ;
            }
            catch (RTI::FederatesCurrentlyJoined) {
                sleep(5);
            }
        }
    }
    Debug(D, pdTerm) << "Destroying RTIAmbassador and FedAmbassador." << std::endl;
    Debug(D, pdTerm) << "Federation terminated." << std::endl;
}


// ----------------------------------------------------------------------------
/** Carry out publications and subscriptions
 */
void
HLA_OMNET::publishAndSubscribeReceive()
{
    // Interactions 
    NODE_TO_OMNET_ID = rtiamb.getInteractionClassHandle("NODE_TO_OMNET");
    PacketLengthToOmnetID = rtiamb.getParameterHandle("PacketLengthToOmnet", NODE_TO_OMNET_ID);
    PacketDataToOmnetID = rtiamb.getParameterHandle("PacketDataToOmnet", NODE_TO_OMNET_ID);
   
    // Publish and subscribe interactions
    rtiamb.subscribeInteractionClass(NODE_TO_OMNET_ID, RTI::RTI_TRUE);
    
    Debug(D, pdInit) << "Local Objects and Interactions published and subscribed." << std::endl;
}

// ----------------------------------------------------------------------------
/** Carry out publications and subscriptions
 */
void
HLA_OMNET::publishAndSubscribeSend()
{
  
    NODE_TO_GEM5_ID = rtiamb.getInteractionClassHandle("NODE_TO_GEM5");
    PacketLengthToGem5ID = rtiamb.getParameterHandle("PacketLengthToGem5", NODE_TO_GEM5_ID);
    PacketDataToGem5ID = rtiamb.getParameterHandle("PacketDataToGem5", NODE_TO_GEM5_ID);
    
    rtiamb.publishInteractionClass(NODE_TO_GEM5_ID);

    Debug(D, pdInit) << "Local Objects and Interactions published and subscribed." << std::endl;
}

// ============================================================================
//          ------------ END INITIALIZATION FUNCTIONS ------------
// ============================================================================








// ============================================================================
//          ------------ INTERACTION FUNCTIONS ------------
// ============================================================================


void
HLA_OMNET::sendInteraction(uint8_t* PacketDataToGem5, uint32_t n)
{
  
    libhla::MessageBuffer buffer;
    RTI::ParameterHandleValuePairSet *parameterSet=NULL ;

    if(n>0){
        parameterSet = RTI::ParameterSetFactory::create(2);


        //! Send the Packet Data !//
        buffer.reset();
        buffer.write_uint8s(PacketDataToGem5,n);
        buffer.updateReservedBytes();
        parameterSet->add(PacketDataToGem5ID, static_cast<char*>(buffer(0)), buffer.size());
    }
    else{
        parameterSet = RTI::ParameterSetFactory::create(1);
    }
    
    //! Send the Packet Length !// 
    buffer.reset();
    buffer.write_uint32(n);
    buffer.updateReservedBytes();
    parameterSet->add(PacketLengthToGem5ID, static_cast<char*>(buffer(0)), buffer.size());
    
    
    try {
      rtiamb.sendInteraction(NODE_TO_GEM5_ID, *parameterSet, "");
            
    }
    catch (RTI::Exception& e) {
        std::cout<<"sendInteraction raise exception "<<e._name<<"("<<e._reason<<")"<<std::endl;
        Debug(D, pdExcept) << "**** Exception sending interaction : " << &e << std::endl;
    }

    delete parameterSet ;
}


// ----------------------------------------------------------------------------
/** Callback : receive interaction
 */
void 
HLA_OMNET::receiveInteraction(RTI::InteractionClassHandle theInteraction,
        const RTI::ParameterHandleValuePairSet& theParameters,
        const RTI::FedTime& /*theTime*/,
        const char* /*theTag*/,
        RTI::EventRetractionHandle /*theHandle*/)
{
    libhla::MessageBuffer buffer;
    RTI::ULong valueLength ;
    uint32_t PacketLengthFromGEM5 = 0;
    
    Debug(D, pdTrace) << "Fed : receiveInteraction" << std::endl;
    if (theInteraction != NODE_TO_OMNET_ID) {
        printf("CALLBACK receiveInteraction : Unknown Interaction received");
        exit(-1);
    }

    Debug(D, pdDebug) << "receiveInteraction - nb attributs= " << theParameters.size() << std::endl;
    
    EthPacketPtr RcvPacketPtr;
    
    //std::shared_ptr<EthPacketData> RcvPacketPtr(new EthPacketData());

    for (unsigned int j = 0 ; j < theParameters.size(); ++j) {
        RTI::ParameterHandle parmHandle = theParameters.getHandle(j);

        valueLength = theParameters.getValueLength(j);
        assert(valueLength>0);
        buffer.resize(valueLength);
        buffer.reset();
        theParameters.getValue(j, static_cast<char*>(buffer(0)), valueLength);        
        buffer.assumeSizeFromReservedBytes();

        if (parmHandle == PacketLengthToOmnetID) {
            PacketLengthFromGEM5 = buffer.read_uint32();
            if(PacketLengthFromGEM5 == 0){
              RcvPacketPtr = std::make_shared<EthPacketData>();
            }
        }
        else if (parmHandle == PacketDataToOmnetID) { 
            if(PacketLengthFromGEM5 > 0){
              RcvPacketPtr = std::make_shared<EthPacketData>(PacketLengthFromGEM5);
              buffer.read_uint8s(RcvPacketPtr->data, PacketLengthFromGEM5);
            }
        }
        else {
            Debug(D, pdError) << "Unrecognized parameter handle" << std::endl;
        }
    }
    
    RcvPacketPtr->length = PacketLengthFromGEM5;
    
    packetBuffer.push(RcvPacketPtr);    
    
}


bool 
HLA_OMNET::BufferPacketEmpty(){
  return packetBuffer.empty();
}

EthPacketPtr 
HLA_OMNET::getPacket(){
  EthPacketPtr packet = packetBuffer.front();
  return packet;
}

void 
HLA_OMNET::clearRcvPacket(){
  packetBuffer.front() = NULL;
  packetBuffer.pop();
}


// ============================================================================
//          ------------ END INTERACTION FUNCTIONS ------------
// ============================================================================







// ============================================================================
//          ------------ SYNCHRONIZATION FUNCTIONS ------------
// ============================================================================

/** Creator put federation in pause.
 */
void
HLA_OMNET::pause()
{
    if (creator) {
        Debug(D, pdInit) << "Pause requested" << std::endl;
        try {
            rtiamb.registerFederationSynchronizationPoint("Init", "Waiting all federations.");
        }
        catch (RTI::Exception& e) {
            Debug(D, pdExcept) << "Federate " << federateName
                        << " : Register Synchronization Point failed : %d"
                        << endl ;
        }
    }
}

// ----------------------------------------------------------------------------
/** Creator put federation in pause for synchronization with a friend
 */
void
HLA_OMNET::pause_friend()
{
    if (creator) {
        Debug(D, pdInit) << "Pause requested for friend" << std::endl;
        try {
            // For testing purpose
             RTI::FederateHandle numfed(0) ;
             RTI::FederateHandleSet *federateSet = RTI::FederateHandleSetFactory::create(1) ;
             cout << "Now we test Register Federation Synchronisation Point on some federates" << endl ;
             cout << "Please enter a federate handle (zero means none)" << endl ;
             cout << "This federate will be synchronized with the creator and not the others" << endl;


             scanf("%lu",&numfed);
             if (numfed != 0)
                 {
                 // We store numfed into the federate set
                 federateSet->add(numfed) ;
                 rtiamb.registerFederationSynchronizationPoint("Friend","Synchro with a friend",
                                                          *federateSet) ;
                 }
        }
        catch (RTI::Exception& e) {
            Debug(D, pdExcept) << "Federate " << federateName
                        << " : Register Synchronization Point failed : %d"
                        << endl ;
        }
    }
}
// ----------------------------------------------------------------------------
/** tick the RTI
 */
void
HLA_OMNET::tick()
{
    usleep( 0 ) ;
    rtiamb.tick();
    nbTicks++ ;
}
void
HLA_OMNET::tick2()
{
    rtiamb.tick2();
    nbTicks++ ;
}

// ----------------------------------------------------------------------------
/** Set time regulation (time regulating and time constrained)
    @param start_constrained boolean, if true federate is constrained
    @param start_regulating boolean, if true federate is regulating
 */
void
HLA_OMNET::setTimeRegulation(bool start_constrained, bool start_regulating)
{
    Debug(D, pdInit) << "Time Regulation setup" << std::endl;

    if (start_constrained) {
        if (!constrained) {
            // change from no constrained to constrained
            rtiamb.enableTimeConstrained();
            constrained = true ;
            Debug(D, pdInit) << "Time Constrained enabled." << std::endl;
        }
        //rtiamb.modifyLookahead(TIME_STEP);
    }
    else {
        if (constrained) {
            // change from constrained to no constrained
            rtiamb.disableTimeConstrained();
            constrained = false ;
            Debug(D, pdInit) << "Time Constrained disabled." << std::endl;
        }
    }

    if (start_regulating) {
        if (!regulating) {
            // change from no regulating to regulating
            for (;;) {
                rtiamb.queryFederateTime(localTime);

                try {
                    rtiamb.enableTimeRegulation(localTime, TIME_STEP);
                    regulating = true ;
                    break ;
                }
                catch (RTI::FederationTimeAlreadyPassed) {
                    // Si Je ne suis pas le premier, je vais les rattraper.
                    rtiamb.queryFederateTime(localTime);

                    RTIfedTime requestTime(((RTIfedTime&)localTime).getTime());
                    requestTime += TIME_STEP ;

		    granted = false ;
                    rtiamb.timeAdvanceRequest(requestTime);		    
                    while (!granted) {
                        try {
                            tick();
                        }
                        catch (RTI::RTIinternalError) {
                            printf("RTIinternalError Raised in tick.\n");
                            exit(-1);
                        }
                    }
                }
                catch (RTI::RTIinternalError) {
                    printf("RTIinternalError Raised in setTimeRegulating.\n");
                    exit(-1);
                }
            }
        }
    }
    else {
        if (regulating) {
            // change from regulating to no regulating
            rtiamb.disableTimeRegulation();
            regulating = false ;
        }
    }
}

// ----------------------------------------------------------------------------
/** Synchronize with other federates
 */
void
HLA_OMNET::synchronize()
{
    Debug(D, pdInit) << "Synchronize" << std::endl;

    if (creator) {
        // Wait a signal from user and stop the pause synchronization.

        Debug(D, pdInit) << "Creator can resume execution..." << std::endl;
        while (!paused)
            try {
		
                Debug(D, pdInit) << "not paused" << std::endl;
                tick();
            }
            catch (RTI::Exception& e) {
                Debug(D, pdExcept) << "******** Exception ticking the RTI : " << &e << std::endl;
                throw ;
            }
            Debug(D, pdDebug) << "paused" << std::endl;

        try {
            rtiamb.synchronizationPointAchieved("Init");
        }
        catch (RTI::Exception& e) {
            Debug(D, pdExcept) << "**** Exception achieving a synchronization point by creator : " << &e << std::endl;
        }
       
        while (paused)
            try {
                tick();
            }
            catch (RTI::Exception& e) {
                Debug(D, pdExcept) << "**** Exception ticking the RTI : " << &e << std::endl;
                throw ;
            }
    }
    else {
        if (!paused) {
            Debug(D, pdInit) << "Federate not paused: too early" << std::endl;
            while (!paused) {
                try {
                    tick();
                }
                catch (RTI::Exception& e) {
                    Debug(D, pdExcept) << "******** Exception ticking the RTI : " << &e << std::endl;
                    throw ;
                }
            }
        }
        Debug(D, pdInit) << "Federate paused" << std::endl;

        try {
            // Federate ends its synchronization.
            rtiamb.synchronizationPointAchieved("Init");
            Debug(D, pdInit) << "Pause achieved." << std::endl;
        }
        catch (RTI::Exception& e) {
            Debug(D, pdExcept) << "**** Exception achieving a synchronization point : " << &e << std::endl;
        }

        Debug(D, pdInit) << "Federate waiting end of pause..." << std::endl;
        while (paused) {
            try {
                tick();
            }
            catch (RTI::Exception& e) {
                Debug(D, pdExcept) << "******** Exception ticking the RTI : " << &e << std::endl;
                throw ;
            }
        }
        Debug(D, pdInit) << "End of pause" << std::endl;
    }

    Debug(D, pdInit) << "Federation is synchronized." << std::endl;

}




// ----------------------------------------------------------------------------
/** one simulation step advance)
 */
void
HLA_OMNET::step()
{
  
  
    granted = false ;

    try {
        rtiamb.queryFederateTime(localTime);
    }
    catch (RTI::Exception& e) {
        Debug(D, pdExcept) << "**** Exception asking for federate local time : " << &e << std::endl;
    }

    try {
        RTIfedTime time_aux(localTime.getTime()+TIME_STEP.getTime());

        Debug(D, pdDebug) << "time_aux : " << time_aux.getTime()
                                  << " - localtime : " << ((RTIfedTime&) localTime).getTime()
                                  << " - timestep : " << ((RTIfedTime&) TIME_STEP).getTime() << std::endl;
        granted = false ;
        rtiamb.timeAdvanceRequest(time_aux);
    }
    catch (RTI::Exception& e) {
        Debug(D, pdExcept) << "******* Exception sur timeAdvanceRequest." << std::endl;
    }

    while (!granted) {
        try {
            tick2();
        }
        catch (RTI::Exception& e) {
            Debug(D, pdExcept) << "******** Exception ticking the RTI : " << &e << std::endl;
            throw ;
        }
    }

    next_step = (localTime + TIME_STEP);

   // printf("next_step(federationName: %s):  %.2f\n",federationName.c_str(), ((RTIfedTime&)next_step).getTime());
    
}

// ----------------------------------------------------------------------------
/** Callback : time advance granted
 */
void
HLA_OMNET::timeAdvanceGrant(const RTI::FedTime& theTime)
{    
    granted = true ;
    localTime = theTime ;
    Debug(D, pdTrace) << "Time advanced, local time is now " << localTime.getTime() << std::endl;
}

// ----------------------------------------------------------------------------
/** Callback announce synchronization point
 */
void
HLA_OMNET::announceSynchronizationPoint(const char *label, const char */*tag*/)
{
    if (strcmp(label, "Init") == 0) {
        paused = true ;
        Debug(D, pdProtocol) << "announceSynchronizationPoint." << std::endl;
    }
    else if (strcmp(label, "Friend") == 0) {
        std::cout<<"**** I am happy : I have a friend ****"<<std::endl;
        paused = true ;
        Debug(D, pdProtocol) << "announceSynchronizationPoint (friend)." << std::endl;
    } 
    else {
        cout << "Unexpected synchronization label" << endl ;
        exit(1);
    }
}

// ----------------------------------------------------------------------------
/** Callback : federation synchronized
 */
void
HLA_OMNET::federationSynchronized(const char *label)
{
    if (strcmp(label, "Init") == 0) {
        paused = false ;
        Debug(D, pdProtocol) << "CALLBACK : federationSynchronized with label " << label << std::endl;
    }
}

// ============================================================================
//          ------------ END SYNCHRONIZATION FUNCTIONS ------------
// ============================================================================






// ============================================================================
//          ------------ ATTRIBUTES FUNCTIONS ------------
// ============================================================================


// ----------------------------------------------------------------------------
/** Callback : reflect attribute values with time
 */
void
HLA_OMNET::reflectAttributeValues(RTI::ObjectHandle theObject, 
			    const RTI::AttributeHandleValuePairSet & theAttributes, 
			    const char */*theTag*/) 
{

}


// ----------------------------------------------------------------------------
/** Callback : remove object instance
 */
void
HLA_OMNET::removeObjectInstance(RTI::ObjectHandle theObject,
			      const RTI::FedTime &,
			      const char *,
			      RTI::EventRetractionHandle)
{
 
}

// ----------------------------------------------------------------------------
/** Callback : discover object instance
 */
void
HLA_OMNET::discoverObjectInstance(RTI::ObjectHandle theObject,
				RTI::ObjectClassHandle theObjectClass,
				const char */*theObjectName*/)
{

}

// ============================================================================
//          ------------ END ATTRIBUTES FUNCTIONS ------------
// ============================================================================


//! --- CERTI INITIALIZATION IP --- !//
bool
HLA_OMNET::RequestFunction(HLAInitializationRequest rqst){
  int sockfd = 0, n = 0;
  bool ret = false;

  struct sockaddr_in serv_addr;

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
      printf("\n Error : Could not create socket \n");
      exit(-1);
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(60401);

  char* pPath = getenv ("CERTI_HOST");
  if (pPath==NULL)
    pPath = (char *) "127.0.0.1";

  if(inet_pton(AF_INET, pPath, &serv_addr.sin_addr)<=0)
  {
      printf("\n inet_pton error occured\n");
      exit(-1);
  }

  if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
     if(!FirstConnectionWithHLAInitialization){
       printf("\n Error : Connection with HLA Initialization Server (IP:%s) Failed \n",pPath);
       exit(-1);
     }
     else{
       printf("\n Warning : Connection with HLA Initialization Server (IP:%s) Failed.. Retry in 30 secs\n",pPath);
       sleep(30);
       return RequestFunction(rqst);
     }
  }
  else{
    FirstConnectionWithHLAInitialization = true;
  }

  write(sockfd, (const void *) &rqst, sizeof(rqst));

  n = read(sockfd, (void *) &ret, sizeof(ret));
  if(n < 0)
    printf("\n Reply error \n");

  close(sockfd);

  //! Set the appropriate delay if the server is in localhost or not !//
  if((strcmp(pPath,(char *)"127.0.0.1")!=0)&&((rqst.type == READ)||(rqst.type == READ_GLOBAL))){
    usleep(200000);
  }
  else{
    usleep(10000);
  }
  return ret;
}
//! --- END CERTI INITIALIZATION IP --- !//
