# COSSIM OMNETPP WORKSPACE 

COSSIM OMNETPP WORKSPACE implements a basic demo with 4 simulated nodes which are connected through Ethernet and Wireless networks. In addition, it contains all modifications required to extend the functionality of the OMNET++ network simulator to work with the COSSIM framework. 

## Introduction
The key idea behind OMNeT++ integration into COSSIM simulator was to support real protocol stacks (i.e linux), although they were not natively supported by it, and at the same time, preserving 100% compatibility with its legacy. Towards this direction, we had to implement a transparent procedure (a special form of node) of encapsulation/decapsulation from the cGEM5 binary Ethernet packet into a compatible form always comprehensible from the OMNeT++ protocol stack.

## HLA Enabled Node Functionality

The nodes implementing that procedure inside cOMNeT++ are called HLA Enabled nodes as it is possible to transparently communicate with cGEM5 through an HLA run-time infrastructure (RTI) wrapper, while encapsulate/decapsulate network packets in a comprehensible form for both simulators. Normal OMNeT++ nodes, such as routers and switches, could also be present in the same simulation to support the connectivity in the network and the routing of the packets.

In this respect the COSSIM approach requires each simulated node to consist of two parts, the processing (simulated in cGEM5) and the network (which is part of cOMNET++) communicating to each other through the HLA interfaces. Each of the HLA Enabled Nodes has a minimum functionality that allows them to communicate with their counterpart on the cGEM5 side. The network sub-system of the COSSIM simulator is designed to model the complete network related behavior of each simulated node along with all the network characteristics. The upper protocol stack (from Layer 2 and above) is accurately simulated from the processing sub-system.


More specifically a CERTI-HLA compliant wrapper was developed within OMNET++, offering a unique interface to each node simulated in the network subsystem in order to communicate consistently and synchronized with the processing subsystem of the COSSIM simulator. These nodes communicate (via the HLA sockets) transparently with the processing simulator. All the added functionality was deployed in the user space to assure 100% compatibility with OMNeT++ and its libraries (e.g. INET). As mentioned earlier in order to increase COSSIM’s simulation accuracy the upper protocol stack of the network should be simulated in the processing sub-system (that is cGEM5). On the other hand, the network sub-system should be able to forward network packets in the data link layer (L2) or in the network layer (L3) in order for other aspects of the network to be modeled (e.g packet latency). Furthermore OMNeT++, like any other network simulator (e.g ns2), does not produce RFC-compliant packets, meaning that there is no actual payload data (from the application layer for simplicity and performance purposes) and actually, only the payload length field in handled.

On the other hand, and since the goal of COSSIM is to provide cycle-accurate simulation, the whole linux protocol stack is executed within cGEM5, thus producing a fully RFC-compliant binary IP packet. In order for these packets to be forwarded, they have to be properly encapsulated inside the cOMNeT++ L2/L3 packet structure. The first step towards this direction was to modify the standard OMNeT++ cPacket structure to include payload data. The second step was to develop a custom-fit functionality that will be automatically inherited in each of the HLA enabled nodes of the simulation to seamlessly convert the cGEM5 packets to cOMNeT++ packets and vice versa. Each packet sent from the cGEM5 subsystem into the OMNeT++ passes through a sequential de-capsulation procedure (i.e parsing) from the Ethernet/L2 level to the L3 level (the IP level) followed by an encapsulation procedure in the OMNeT++ protocol stack. In the opposite direction (from OMNeT++ to cGEM5) all the L2/L3 fields had to be properly de-serialized into a single RFC-compliant binary packet (i.e. a valid Ethernet packet) that will form the payload for the HLA channel. 

## Transparent micro-Routers Functionality
The key idea behind OMNeT++ integration into COSSIM simulator was to support real protocol stacks (i.e linux) while preserving 100% compatibility with the OMNeT++ and at the same time provide a constant hook point with the cGEM5 (i.e ethernet interface). The HLA Node functionality partially fulfilled this goal as it linked theoretically incompatible protocol stacks between the two simulators. A second challenge though, was still ongoing: how we could possibly support different network interfaces without affecting the attaching point (i.e Ethernet adapter) between GEM5 and OMNeT++, knowing that it was very difficult for GEM5 to support different network interfaces.

A smart way to overcome this challenge was to implement a mirco-router functionality within OMNET++ allowing user to change the physical medium (and as a result the network adapter) from Ethernet to any other supported option (ppp, wireless etc.) without affecting at all the GEM5 node hook point implementation. In this way, the GEM5 nodes could always "see" an "Ethernet attached" network on the OMNeT++ side.

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

