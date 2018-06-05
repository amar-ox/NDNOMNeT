//
// Copyright (c) 2018 Amar Abane (a_abane@hotmail.fr). All rights reserved.
// This file is part of NDN-Omnet.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.node.ndn;

import inet.applications.ndnApps.ConsumerAppBase;
import inet.applications.ndnApps.ProducerAppBase;

module NdnWirelessHost extends NdnWirelessHostBase
{
    parameters:
        @display("i=device/pocketpc_s;bgb=457,425");
        @networkNode;
        @labels(wireless-NDN-node-application);  
        int numConsumerApps = default(0);
        int numProducerApps = default(0);
        bool hasConsumer = default(numConsumerApps > 0);
        bool hasProducer = default(numProducerApps > 0);
        forwarding = default(false);          

    submodules:
        consumer[numConsumerApps]: ConsumerAppBase {
            @display("p=338,53");
        }

        producer[numProducerApps]: ProducerAppBase {
            @display("p=111,53");
        }
                
    connections allowunconnected:        
        for i=0..numConsumerApps-1 {
            ndnL3.transportIn++ <-- consumer[i].consumerOut;
            ndnL3.transportOut++ --> consumer[i].consumerIn;
        }

        for i=0..numProducerApps-1 {
            ndnL3.transportIn++ <-- producer[i].producerOut;
            ndnL3.transportOut++ --> producer[i].producerIn;
        }
}
