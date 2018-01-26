// ----------------------------------------------------------------------------
// COSSIM - GEM5 HLA RunTime Infrastructure
// Copyright (C) 2015  TUC
//
// ----------------------------------------------------------------------------

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
      /****** Create Signal Files (HLA initialization) *****/
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
      /****** END Create Signal Files (HLA initialization) *****/
      //! --- END CERTI INITIALIZATION IP --- !//
    }


}

// ----------------------------------------------------------------------------
/** Destructor
 */
HLA_OMNET::~HLA_OMNET()
    throw (RTI::FederateInternalError)
{
}


// ============================================================================
//          ------------ INITIALIZATION FUNCTIONS ------------
// ============================================================================

//! GLOBAL SYNCH CHANGES 18/01/2016 !//

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
        D.Out(pdInit, "Federation execution created.");
        creator = true ;
    }
    catch (RTI::FederationExecutionAlreadyExists& e) {
        printf("HLA_OMNET Note : %s Reason is : %s. OK I can join it\n",e._name,e._reason);
        D.Out(pdInit, "Federation execution already created.");
    }
    catch (RTI::CouldNotOpenFED& e) {
        printf("HLA_OMNET ERROR : %s Reason is : %s\n",e._name,e._reason);
        D.Out(pdExcept, "HLA_OMNET : Could not use FED file.");
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
            D.Out(pdExcept, "Federate %s : FederationExecutionDoesNotExist.",
                  federateName.c_str());
            // sleep(1);
        }
        catch (RTI::Exception& e) {
            D.Out(pdExcept,
                  "Federate %s :Join Federation Execution failed : %d .",
                  federateName.c_str(), &e);
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
        D.Out(pdRegister, "Local object deleted from federation.");
    }
    catch (RTI::Exception &e) {
        D.Out(pdExcept, "**** Exception delete object : %d", &e);
    }

    D.Out(pdTerm, "Local objects deleted.");

    setTimeRegulation(false, false);

    try {
        rtiamb.resignFederationExecution(
            RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
        D.Out(pdTerm, "Just resigned from federation");
    }
    catch (RTI::Exception &e) {
        D.Out(pdExcept,
              "** Exception during resignFederationExecution by federate");
    }
    // Detruire la federation

    if (creator) {
        for (;;) {
            tick();
            try {
                D.Out(pdTerm, "Asking from federation destruction...");
                rtiamb.destroyFederationExecution(federationName.c_str());

                D.Out(pdTerm, "Federation destruction granted.");
                break ;
            }
            catch (RTI::FederatesCurrentlyJoined) {
                sleep(5);
            }
        }
    }
    D.Out(pdTerm, "Destroying RTIAmbassador and FedAmbassador.");
    D.Out(pdTerm, "Federation terminated.");
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
  
    // Get all class and attributes handles
    //getHandles();

    // Add PositionX et PositionY to the attribute set
    /* auto_ptr<RTI::AttributeHandleSet> attributes(RTI::AttributeHandleSetFactory::create(3));
    attributes->add(AttrXID);
    attributes->add(AttrYID);

    // Subscribe to Bille objects.
    Debug(D, pdDebug) << "subscribe: class " << BilleClassID << ", attributes "
	       << AttrXID << " and " << AttrYID << "... " << endl ;
    rtiamb.subscribeObjectClassAttributes(BilleClassID, *attributes, RTI::RTI_TRUE);
    Debug(D, pdDebug) << "done." << endl ;

    // Publish Boule Objects.
    attributes->add(AttrColorID);
    rtiamb.publishObjectClass(BouleClassID, *attributes);
    */
   
   
    // Publish and subscribe to Bing interactions
    rtiamb.subscribeInteractionClass(NODE_TO_OMNET_ID, RTI::RTI_TRUE);
    
    //rtiamb.publishInteractionClass(NODE_TO_GEM5_ID);

    D.Out(pdInit, "Local Objects and Interactions published and subscribed.");
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
  
    // Get all class and attributes handles
    //getHandles();

    // Add PositionX et PositionY to the attribute set
    /* auto_ptr<RTI::AttributeHandleSet> attributes(RTI::AttributeHandleSetFactory::create(3));
    attributes->add(AttrXID);
    attributes->add(AttrYID);

    // Subscribe to Bille objects.
    Debug(D, pdDebug) << "subscribe: class " << BilleClassID << ", attributes "
	       << AttrXID << " and " << AttrYID << "... " << endl ;
    rtiamb.subscribeObjectClassAttributes(BilleClassID, *attributes, RTI::RTI_TRUE);
    Debug(D, pdDebug) << "done." << endl ;

    // Publish Boule Objects.
    attributes->add(AttrColorID);
    rtiamb.publishObjectClass(BouleClassID, *attributes);
    */
   
   
    // Publish and subscribe to Bing interactions
    //rtiamb.subscribeInteractionClass(NODE_TO_OMNET_ID, RTI::RTI_TRUE);
    
    rtiamb.publishInteractionClass(NODE_TO_GEM5_ID);

    D.Out(pdInit, "Local Objects and Interactions published and subscribed.");
}

// ----------------------------------------------------------------------------
/** get handles of objet/interaction classes
 */
/*void
HLA_OMNET::getHandles()
{
    Debug(D, pdDebug) << "Get handles..." << endl ;
    BilleClassID = rtiamb.getObjectClassHandle(CLA_BILLE);
    BouleClassID = rtiamb.getObjectClassHandle(CLA_BOULE);
    D.Out(pdInit, "BilleClassID = %d, BouleClassID = %d.",
          BilleClassID, BouleClassID);

    // Attributs des classes d'Objets
    AttrXID = rtiamb.getAttributeHandle(ATT_POSITION_X, BilleClassID);
    AttrYID = rtiamb.getAttributeHandle(ATT_POSITION_Y, BilleClassID);
    AttrColorID = rtiamb.getAttributeHandle(ATT_COLOR, BouleClassID);
    D.Out(pdInit, "AttrXID = %d, AttrYID = %d, AttrColorID = %d.",
          AttrXID, AttrYID, AttrColorID);
    
    
    
    
    
    
}*/
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
        D.Out(pdExcept, "**** Exception sending interaction : %d", &e);
    }

    delete parameterSet ;
}

/*
void
HLA_OMNET::sendInteraction(double PacketDataToGEM5)
{
    libhla::MessageBuffer buffer;
    RTI::ParameterHandleValuePairSet *parameterSet=NULL ;

    parameterSet = RTI::ParameterSetFactory::create(1);

    buffer.reset();
    buffer.write_double(PacketDataToGEM5);
    buffer.updateReservedBytes();
    parameterSet->add(DataToGem5ID, static_cast<char*>(buffer(0)), buffer.size());

  
    try {
      rtiamb.sendInteraction(NODE_TO_GEM5_ID, *parameterSet, "");
            
    }
    catch (RTI::Exception& e) {
        std::cout<<"sendInteraction raise exception "<<e._name<<"("<<e._reason<<")"<<std::endl;
        D.Out(pdExcept, "**** Exception sending interaction : %d", &e);
    }

    delete parameterSet ;
}
*/

// ----------------------------------------------------------------------------
/** Callback : receive interaction
 */

void 
HLA_OMNET::receiveInteraction(RTI::InteractionClassHandle theInteraction, 
			      const RTI::ParameterHandleValuePairSet & theParameters, 
			      const char */*theTag*/) 
	throw (RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown, 
	       RTI::FederateInternalError) 
{
    libhla::MessageBuffer buffer;
    RTI::ULong valueLength ;
    uint32_t PacketLengthFromGEM5 = 0;
    
    D.Out(pdTrace, "Fed : receiveInteraction");
    if (theInteraction != NODE_TO_OMNET_ID) {
        printf("CALLBACK receiveInteraction : Unknown Interaction received");
        exit(-1);
    }

    D.Out(pdDebug, "receiveInteraction - nb attributs= %d", theParameters.size());
    
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
            D.Out(pdError, "Unrecognized parameter handle");
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
        D.Out(pdInit, "Pause requested");
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
        D.Out(pdInit, "Pause requested for friend");
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
    D.Out(pdInit, "Time Regulation setup");

    if (start_constrained) {
        if (!constrained) {
            // change from no constrained to constrained
            rtiamb.enableTimeConstrained();
            constrained = true ;
            D.Out(pdInit, "Time Constrained enabled.");
        }
        //rtiamb.modifyLookahead(TIME_STEP);
    }
    else {
        if (constrained) {
            // change from constrained to no constrained
            rtiamb.disableTimeConstrained();
            constrained = false ;
            D.Out(pdInit, "Time Constrained disabled.");
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
    D.Out(pdInit, "Synchronize");

    if (creator) {
        // Wait a signal from user and stop the pause synchronization.

        D.Out(pdInit, "Creator can resume execution...");
        while (!paused)
            try {
		
                D.Out(pdInit, "not paused");
                tick();
            }
            catch (RTI::Exception& e) {
                D.Out(pdExcept, "******** Exception ticking the RTI : %d ", &e);
                throw ;
            }
        D.Out(pdDebug, "paused");

        try {
            rtiamb.synchronizationPointAchieved("Init");
        }
        catch (RTI::Exception& e) {
            D.Out(pdExcept, "**** Exception achieving a synchronization "
                  "point by creator : %d", &e);
        }
       
        while (paused)
            try {
                tick();
            }
            catch (RTI::Exception& e) {
                D.Out(pdExcept, "**** Exception ticking the RTI : %d.", &e);
                throw ;
            }
    }
    else {
        if (!paused) {
            D.Out(pdInit,
                  "Federate not paused: too early");
            while (!paused) {
                try {
                    tick();
                }
                catch (RTI::Exception& e) {
                    D.Out(pdExcept,
                          "******** Exception ticking the RTI : %d.", &e);
                    throw ;
                }
            }
        }
        D.Out(pdInit, "Federate paused");

        try {
            // Federate ends its synchronization.
            rtiamb.synchronizationPointAchieved("Init");
            D.Out(pdInit, "Pause achieved.");
        }
        catch (RTI::Exception& e) {
            D.Out(pdExcept,
                  "**** Exception achieving a synchronization point : %d",
                  &e);
        }

        D.Out(pdInit,
              "Federate waiting end of pause...");
        while (paused) {
            try {
                tick();
            }
            catch (RTI::Exception& e) {
                D.Out(pdExcept, "******** Exception ticking the RTI : %d.", &e);
                throw ;
            }
        }
        D.Out(pdInit, "End of pause");
    }

    D.Out(pdInit, "Federation is synchronized.");

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
        D.Out(pdExcept,
              "**** Exception asking for federate local time : ", &e);
    }

    try {
        RTIfedTime time_aux(localTime.getTime()+TIME_STEP.getTime());

        D.Out(pdDebug, "time_aux : %.2f - localtime : %.2f - "
              "timestep : %.2f", time_aux.getTime(),
              ((RTIfedTime&)localTime).getTime(),
              ((RTIfedTime&)TIME_STEP).getTime());
        granted = false ;
        rtiamb.timeAdvanceRequest(time_aux);
    }
    catch (RTI::Exception& e) {
        D.Out(pdExcept, "******* Exception sur timeAdvanceRequest.");
    }

    while (!granted) {
        try {
            tick2();
        }
        catch (RTI::Exception& e) {
            D.Out(pdExcept, "******** Exception ticking the RTI : %d.", &e);
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
    throw (RTI::InvalidFederationTime, RTI::TimeAdvanceWasNotInProgress, 
	   RTI::FederateInternalError)
{    
    granted = true ;
    localTime = theTime ;
    D.Out(pdTrace, "Time advanced, local time is now %.2f.",
          localTime.getTime());
}

// ----------------------------------------------------------------------------
/** Callback announce synchronization point
 */
void
HLA_OMNET::announceSynchronizationPoint(const char *label, const char */*tag*/)
    throw (RTI::FederateInternalError)
{
    if (strcmp(label, "Init") == 0) {
        paused = true ;
        printf("announceSynchronizationPoint\n");
    }
    else if (strcmp(label, "Friend") == 0) {
        std::cout<<"**** I am happy : I have a friend ****"<<std::endl;
        paused = true ;
        D.Out(pdProtocol, "announceSynchronizationPoint (friend).");
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
    throw (RTI::FederateInternalError)
{
    if (strcmp(label, "Init") == 0) {
        paused = false ;
        D.Out(pdProtocol,
              "CALLBACK : federationSynchronized with label %s", label);
    }
}

// ============================================================================
//          ------------ END SYNCHRONIZATION FUNCTIONS ------------
// ============================================================================






// ============================================================================
//          ------------ ATTRIBUTES FUNCTIONS ------------
// ============================================================================

/** Updates a ball by sending entity position and color.
    \param x X position
    \param y Y position
    \param color Color
    \param UpdateTime Event time
    \param id Object handle (ball)
 */
/*void
HLA_OMNET::sendUpdate(double x, double y, int color, RTI::ObjectHandle id)
{    
    libhla::MessageBuffer buffer;
    RTI::AttributeHandleValuePairSet *attributeSet ;

    attributeSet = RTI::AttributeSetFactory::create(3);

    printf("SendUpdate\n");
   
    buffer.reset();
    buffer.write_double(x);
    buffer.updateReservedBytes();
    attributeSet->add(AttrXID, static_cast<char*>(buffer(0)),buffer.size());    
    D.Out(pdDebug, "SendUpdate - AttrXID= %u, x= %f, size= %u, attribute size=%d",
          AttrXID, x, attributeSet->size(),buffer.size());
    
    buffer.reset();
    buffer.write_double(y);	
    buffer.updateReservedBytes();
    attributeSet->add(AttrYID, static_cast<char*>(buffer(0)),buffer.size());
    D.Out(pdDebug, "SendUpdate - AttrYID= %u, y= %f, size= %u",
          AttrYID, y, buffer.size());

    buffer.reset();
    buffer.write_int32(color);	
    buffer.updateReservedBytes();
    attributeSet->add(AttrColorID, static_cast<char*>(buffer(0)),buffer.size());
   
    D.Out(pdDebug, "SendUpdate - AttrColorID= %u, color= %f, size= %u",
          AttrColorID, color, buffer.size());

    try {
      rtiamb.updateAttributeValues(id, *attributeSet, "coucou");     
    }
    catch (RTI::Exception& e) {
        std::cout<<"Exception "<<e._name<<" ("<<e._reason<<")"<<std::endl;
        D.Out(pdExcept, "**** Exception updating attribute values: %d", &e);
    }

    delete attributeSet ;
}*/


// ----------------------------------------------------------------------------
/** Callback : reflect attribute values with time
 */
void
HLA_OMNET::reflectAttributeValues(RTI::ObjectHandle theObject, 
			    const RTI::AttributeHandleValuePairSet & theAttributes, 
			    const char */*theTag*/) 
	throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateOwnsAttributes,
	       RTI::FederateInternalError)
{
    /*
    libhla::MessageBuffer buffer;
    float x1 = 0 ;
    float y1 = 0 ;

    RTI::ULong valueLength ;  

    D.Out(pdDebug, "reflectAttributeValues - nb attributs= %d",
          theAttributes.size());

    for (unsigned int j=0 ; j<theAttributes.size(); j++) {

        RTI::AttributeHandle parmHandle = theAttributes.getHandle(j);
        valueLength = theAttributes.getValueLength(j);
        assert(valueLength>0);
        buffer.resize(valueLength);        
        buffer.reset();        
        theAttributes.getValue(j, static_cast<char*>(buffer(0)), valueLength);        
        buffer.assumeSizeFromReservedBytes();
        
        if (parmHandle == AttrXID) {
           x1 = buffer.read_double();     
	   printf("x1: %f\n",x1);
        }
        else if (parmHandle == AttrYID) {
           y1 = buffer.read_double();   
	   printf("y1: %f\n",y1);
        }
        else
            D.Out(pdError, "Fed: ERREUR: handle inconnu.");
    }
    */
   
}

/*void
HLA_OMNET::declare()
{
    ID = rtiamb.registerObjectInstance(BouleClassID, federateName.c_str());
   // test, quelle est la classe de l'objet cree
   cout << "the class of the new created object is: " <<
rtiamb.getObjectClass (ID) << endl ;
}*/

// ----------------------------------------------------------------------------
/** Callback : remove object instance
 */
void
HLA_OMNET::removeObjectInstance(RTI::ObjectHandle theObject,
			      const RTI::FedTime &,
			      const char *,
			      RTI::EventRetractionHandle)
    throw (RTI::ObjectNotKnown, RTI::InvalidFederationTime, RTI::FederateInternalError)
{
 
  
  
}

// ----------------------------------------------------------------------------
/** Callback : discover object instance
 */
void
HLA_OMNET::discoverObjectInstance(RTI::ObjectHandle theObject,
				RTI::ObjectClassHandle theObjectClass,
				const char */*theObjectName*/)
    throw (RTI::CouldNotDiscover, RTI::ObjectClassNotKnown, 
	   RTI::FederateInternalError)
{
   // if (theObjectClass != BilleClassID) {
   //     D.Out(pdError, "Object of Unknown Class discovered.");
   //     std::string msg = "Unknown objectClass < ";
   //     throw RTI::FederateInternalError(msg.c_str());
   // }

    //cout << "Discovered object handle = " << theObject <<", name = "<< rtiamb.getObjectInstanceName(theObject) <<endl ;
    
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
