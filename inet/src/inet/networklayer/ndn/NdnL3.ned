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

package inet.networklayer.ndn;

import inet.networklayer.contract.INetworkLayer;

module NdnL3 like INetworkLayer
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        @display("i=block/fork;bgb=381,190");

    gates:
        input ifIn[];
        output ifOut[];
        input transportIn[];
        output transportOut[];
        input pingIn[] @labels(PingPayload/down);
        output pingOut[] @labels(PingPayload/up);

    submodules:
        forwarding: ForwardingBase {
            parameters:
                @display("p=191,105;q=queue");
        }
        pit: PitBase {
            parameters:
                @display("p=315,144");
        }
        fib: FibBase {
            parameters:
                @display("p=58,66");
        }
        cs: CsBase {
            parameters:
                @display("p=58,144");
        }
        xu: Xu {
            parameters:
                @display("p=315,66");
        }

    connections allowunconnected:
        for i=0..sizeof(transportIn)-1 {
            transportIn[i] --> { @display("m=n"); } --> forwarding.upperLayerIn++;
        }
        for i=0..sizeof(transportOut)-1 {
            forwarding.upperLayerOut++ --> { @display("m=n"); } --> transportOut[i];
        }

        ifIn++ --> forwarding.lowerLayerIn++;
        forwarding.lowerLayerOut++ --> ifOut++;

        pit.pitOut --> forwarding.pitIn;
        forwarding.pitOut --> pit.pitIn;

        fib.fibOut --> forwarding.fibIn;
        forwarding.fibOut --> fib.fibIn;

        cs.csOut --> forwarding.csIn;
        forwarding.csOut --> cs.csIn;

        xu.xuOut --> forwarding.xIn;
        forwarding.xOut --> xu.xuIn;
}
