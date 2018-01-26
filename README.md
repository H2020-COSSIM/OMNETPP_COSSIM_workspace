# OMNET++/OMNEST WORKSPACE 

All modifications required to extend the functionality of the OMNET++/OMNEST network simulator to work with the COSSIM framework. 

## What is contained in the WORKSPACE?

- `INET 3.2.4` version which is required by COSSIM to model real networks. 
- `HLANode` project which interconnects the OMNET++ with the cgem5 through HLA.
- `test` project which contains a basic demo with 4 simulated nodes using both Ethernet and Wireless networks. 

Please refer to [COSSIM _framework](https://github.com/H2020-COSSIM/COSSIM_framework) repository for more details.


## Build the OMNET_WORKSPACE

- Select Project -> Clean -> Select “INET” -> Select “Start a build immediately” -> Select “Build only the selected projects” -> Press “OK”
- Select Project -> Clean -> Select “HLANode” & “test” -> Select “Start a build immediately” -> Select “Build only the selected projects” -> Press “OK”

## Using OMNET_WORKSPACE in the context of the COSSIM simulation framework

Please refer to [COSSIM _framework](https://github.com/H2020-COSSIM/COSSIM_framework) repository for all required instructions.

## Licensing

Refer to the [LICENSE](LICENSE) files included. Individual license may be present in different files in the source codes.

#### Authors

* Nikolaos Tampouratzis (ntampouratzis@isc.tuc.gr)

Please contact for any questions.

## Acknowledgments

Code developed for the H2020-COSSIM project.

