# NDNOMNeT

A [Named Data Networking](http://named-data.net/) framework for OMNeT++


# What is it?
NDNOMNeT is an extension for OMNeT++  to simulate **NDN** in IoT systems.
It allows to quickly simulate and visualize NDN scenarios for research and teaching purposes.
It includes a base implementation of NDN entities, simple forwarding strategy over wireless networks and typical NDN applications. 

# What is in the repo?
This framework is based on [INET](https://inet.omnetpp.org/). Thus, NDN-related code is directly included in INET directories.
This framework can be used in OMNeT++ juste as INET [is used](https://github.com/inet-framework/inet)

## The following directories are added:
- src/inet/applications/ndnApps/
- src/inet/networklayer/ndn
- src/inet/networklayer/contract/ndn
- src/inet/node/ndn
- examples/ndn

