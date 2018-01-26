#
# OMNeT++/OMNEST Makefile for libHLANode
#
# This file was generated with the command:
#  opp_makemake -f --deep --make-so -O out -I../../inet/src/inet/networklayer/arp/ipv4 -I../../inet/src/inet/networklayer/ipv4 -I../../inet/src/inet/linklayer/common -I../../inet/src -I../../inet/src/inet/linklayer/ethernet -I../../inet/src/inet/networklayer/contract/ipv4 -L../../inet/out/$$\(CONFIGNAME\)/src -lINET -DINET_IMPORT -KINET_PROJ=../../inet
#

# Name of target to be created (-o option)
TARGET = libHLANode$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I../../inet/src/inet/networklayer/arp/ipv4 \
    -I../../inet/src/inet/networklayer/ipv4 \
    -I../../inet/src/inet/linklayer/common \
    -I../../inet/src \
    -I../../inet/src/inet/linklayer/ethernet \
    -I../../inet/src/inet/networklayer/contract/ipv4 \
    -I.

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = -L../../inet/out/$(CONFIGNAME)/src  -lINET
LIBS += -Wl,-rpath,`abspath ../../inet/out/$(CONFIGNAME)/src`

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = $O/Txc.o $O/HLA_OMNET.o $O/myPacket_m.o

# Message files
MSGFILES = \
    myPacket.msg

# SM files
SMFILES =

# Other makefile variables (-K)
INET_PROJ=../../inet

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppenvir$D $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS) -DINET_IMPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE and have object
# files depend on it (except when "make depend" was called)
COPTS_FILE = $O/.last-copts
ifneq ($(MAKECMDGOALS),depend)
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
$(shell $(MKPATH) "$O" && echo "$(COPTS)" >$(COPTS_FILE))
endif
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# inserted from file 'makefrag':
#------------------------------------------------------------------------------

# The top-level source directory on which CMake was run.
CERTI_SOURCE_DIR = $(value CERTI_SOURCE_DIRECTORY)

# The top-level build directory on which CMake was run.
CERTI_BINARY_DIR = $(value CERTI_BINARY_DIRECTORY)

# ----------------------- FLAGS ----------------------- #

CXX_FLAGS_HLA = -I$(CERTI_SOURCE_DIR)/include -I$(CERTI_SOURCE_DIR)/libCERTI -I/usr/include/libxml2 \
-I$(CERTI_BINARY_DIR)/include -I$(CERTI_BINARY_DIR) -I$(CERTI_SOURCE_DIR)/libHLA \
-I$(CERTI_SOURCE_DIR)/include/hla-1_3 -I$(CERTI_BINARY_DIR)/include/hla-1_3    -DHLA13 -I/lib/x86_64-linux-gnu -std=c++11

CXX_DEFINES_HLA = -DHAVE_CONFIG_H=1 -DHAVE_XML -DHOST_IS_64BITS -DRTI_USES_STD_FSTREAM

LINK_FLAGS_HLA = -rdynamic $(CERTI_BINARY_DIR)/libRTI/hla-1_3/libRTI-NGd.so.1.0.0 $(CERTI_BINARY_DIR)/libCERTI/libCERTId.so.3.4.3 \
	-lxml2 $(CERTI_BINARY_DIR)/libHLA/libHLAd.so.3.4.3 -lrt $(CERTI_BINARY_DIR)/libRTI/hla-1_3/libFedTimed.so.1.0.0 \
	-Wl,-rpath,$(CERTI_BINARY_DIR)/libRTI/hla-1_3:$(CERTI_BINARY_DIR)/libCERTI:$(CERTI_BINARY_DIR)/libHLA

COPTS += $(CXXFLAGS) $(CXX_DEFINES_HLA) $(CXX_FLAGS_HLA)
EXTRA_OBJS+=$(LINK_FLAGS_HLA)

# --------------------- END FLAGS --------------------- #
#-------------------------------.-----------------------------------------------

# <<<
#------------------------------------------------------------------------------

# Main target
all: $O/$(TARGET)
	$(Q)$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	@echo Creating shared library: $@
	$(Q)$(SHLIB_LD) -o $O/$(TARGET)  $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS) $(LDFLAGS)
	$(Q)$(SHLIB_POSTPROCESS) $O/$(TARGET)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE)
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning...
	$(Q)-rm -rf $O
	$(Q)-rm -f HLANode HLANode.exe libHLANode.so libHLANode.a libHLANode.dll libHLANode.dylib
	$(Q)-rm -f ./*_m.cc ./*_m.h ./*_sm.cc ./*_sm.h

cleanall: clean
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(qecho) Creating dependencies...
	$(Q)$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES) $(SM_CC_FILES)  ./*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/HLA_OMNET.o: HLA_OMNET.cc
$O/Txc.o: Txc.cc \
	Txc.h \
	myPacket_m.h \
	$(INET_PROJ)/src/inet/common/Compat.h \
	$(INET_PROJ)/src/inet/common/INETDefs.h \
	$(INET_PROJ)/src/inet/common/InitStages.h \
	$(INET_PROJ)/src/inet/common/ModuleAccess.h \
	$(INET_PROJ)/src/inet/common/NotifierConsts.h \
	$(INET_PROJ)/src/inet/common/ProtocolMap.h \
	$(INET_PROJ)/src/inet/common/ReassemblyBuffer.h \
	$(INET_PROJ)/src/inet/common/TLVOption.h \
	$(INET_PROJ)/src/inet/common/TLVOption_m.h \
	$(INET_PROJ)/src/inet/common/lifecycle/ILifecycle.h \
	$(INET_PROJ)/src/inet/common/queue/AbstractQueue.h \
	$(INET_PROJ)/src/inet/common/queue/QueueBase.h \
	$(INET_PROJ)/src/inet/features.h \
	$(INET_PROJ)/src/inet/linklayer/common/Ieee802Ctrl.h \
	$(INET_PROJ)/src/inet/linklayer/common/Ieee802Ctrl_m.h \
	$(INET_PROJ)/src/inet/linklayer/common/MACAddress.h \
	$(INET_PROJ)/src/inet/linklayer/contract/IMACProtocolControlInfo.h \
	$(INET_PROJ)/src/inet/linklayer/ethernet/EtherEncap.h \
	$(INET_PROJ)/src/inet/linklayer/ethernet/EtherFrame.h \
	$(INET_PROJ)/src/inet/linklayer/ethernet/EtherFrame_m.h \
	$(INET_PROJ)/src/inet/linklayer/ethernet/Ethernet.h \
	$(INET_PROJ)/src/inet/networklayer/arp/ipv4/ARP.h \
	$(INET_PROJ)/src/inet/networklayer/arp/ipv4/ARPPacket_m.h \
	$(INET_PROJ)/src/inet/networklayer/common/IPProtocolId_m.h \
	$(INET_PROJ)/src/inet/networklayer/common/InterfaceEntry.h \
	$(INET_PROJ)/src/inet/networklayer/common/InterfaceToken.h \
	$(INET_PROJ)/src/inet/networklayer/common/L3Address.h \
	$(INET_PROJ)/src/inet/networklayer/common/ModuleIdAddress.h \
	$(INET_PROJ)/src/inet/networklayer/common/ModulePathAddress.h \
	$(INET_PROJ)/src/inet/networklayer/contract/IARP.h \
	$(INET_PROJ)/src/inet/networklayer/contract/IInterfaceTable.h \
	$(INET_PROJ)/src/inet/networklayer/contract/INetfilter.h \
	$(INET_PROJ)/src/inet/networklayer/contract/INetworkDatagram.h \
	$(INET_PROJ)/src/inet/networklayer/contract/INetworkProtocol.h \
	$(INET_PROJ)/src/inet/networklayer/contract/INetworkProtocolControlInfo.h \
	$(INET_PROJ)/src/inet/networklayer/contract/IRoute.h \
	$(INET_PROJ)/src/inet/networklayer/contract/IRoutingTable.h \
	$(INET_PROJ)/src/inet/networklayer/contract/NetworkProtocolCommand_m.h \
	$(INET_PROJ)/src/inet/networklayer/contract/ipv4/IPv4Address.h \
	$(INET_PROJ)/src/inet/networklayer/contract/ipv4/IPv4ControlInfo.h \
	$(INET_PROJ)/src/inet/networklayer/contract/ipv4/IPv4ControlInfo_m.h \
	$(INET_PROJ)/src/inet/networklayer/contract/ipv6/IPv6Address.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/ICMP.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/ICMPMessage.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/ICMPMessage_m.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/IIPv4RoutingTable.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/IPv4.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/IPv4Datagram.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/IPv4Datagram_m.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/IPv4FragBuf.h \
	$(INET_PROJ)/src/inet/networklayer/ipv4/IPv4Route.h
$O/myPacket_m.o: myPacket_m.cc \
	myPacket_m.h

