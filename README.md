# COSSIM OMNETPP WORKSPACE 

COSSIM OMNETPP WORKSPACE implements a basic demo with 4 simulated nodes which are connected through Ethernet and Wireless networks. In addition, it contains all modifications required to extend the functionality of the OMNET++ network simulator to work with the COSSIM framework based on INET (version 3.2.4).

## Introduction
The key idea behind using a dedicated network simulator in COSSIM is to be able to support multiple network protocols, topologies and devices through which nodes (represented by cgem5 instances) can be interconnected. OMNET++ has been chosen as the most capable and feature-rich network simulator in that context, however there are issues that arise. OMNET++ does not support the real protocol stacks (e.g. linux ones) as GEM5 and therefore to be able to bridge the two packages and use freely all OMNET++ legacy requires a procedure that encapsulates/decapsulates cGEM5 binary packets into OMNET++ - compatible packets. The latter can then be used throughout OMNET++ and ensure 100% compatibility with all OMNET++ packages and structures.

## HLA Enabled Node Functionality

cGEM5 instances are connected to OMNET++ through HLA. For each cGEM5 instance, a node is created within OMNET++, called HLA-Enabled node. These nodes can transparently communicate with cGEM5 through an HLA run-time infrastructure (RTI) wrapper, while encapsulating/decapsulating network packets in a comprehensible form for both simulators. Within OMNET++, the HLA-Enabled nodes can communicate with any normal OMNeT++ node, e.g. a router or a switch, and thus any kind of topology can be supported. Furthermore, it is even possible to have in the same simulation OMNET++ nodes that model user-defined systems, if the user of COSSIM is not interested in simulating them in cGEM5.

## Transparent micro-Routers Functionality

Currently, there are no other Network Interface Cards available for GEM5 besides Ethernet ones. To be able to support different network protocols (generally compatible till a certain level, such as WiFi protocols) within OMNET++. a mirco-router functionality is implemented within OMNET++ allowing user to change the physical medium (and as a result the network adapter) from Ethernet to any other supported option without affecting at all the GEM5 node hook point implementation. In this way, the GEM5 nodes could always "sees" an "Ethernet attached" network on the OMNeT++ side.

COSSIM transparent micro-routers from the one side always have an Ethernet interface 100% compatible with the linux protocol stack talking to the GEM5 node counterpart and from the other side an interchangeable option for the user to switch the network interface.

## What is contained in the COSSIM OMNET WORKSPACE?

- `INET 3.2.4` version which is required by COSSIM to model real networks. 
- `HLANode` project which interconnects the OMNET++ with the cgem5 through HLA.
- `test` project which contains a basic demo with 4 simulated nodes using both Ethernet and Wireless networks. 

Please refer to [COSSIM _framework](https://github.com/H2020-COSSIM/COSSIM_framework) repository for more details.


## Build the COSSIM OMNET_WORKSPACE

- Select Project -> Clean -> Clean Projects Selected Below -> Select “INET” -> Select “Start a build immediately” -> Select “Build only the selected projects” -> Press “OK”
- Select Project -> Clean -> Clean Projects Selected Below -> Select “HLANode” & “test” -> Select “Start a build immediately” -> Select “Build only the selected projects” -> Press “OK”

## Using OMNET_WORKSPACE in the context of the COSSIM simulation framework

Please refer to [COSSIM _framework](https://github.com/H2020-COSSIM/COSSIM_framework) repository for all required instructions.

## Licensing

Refer to the [LICENSE](LICENSE) files included. Individual license may be present in different files in the source codes.

#### Authors

* Nikolaos Tampouratzis (ntampouratzis@isc.tuc.gr)

Please contact for any questions.

## Acknowledgments

Code developed for the H2020-COSSIM project.

