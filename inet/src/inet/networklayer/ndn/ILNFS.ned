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

//import inet.networklayer.contract.INetworkProtocol;
import inet.networklayer.contract.ndn.IForwarding;

simple ILNFS like IForwarding
{
    parameters:
        string interfaceTableModule;
        string pitModule = default("^.pit");
        string fibModule = default("^.fib");
        string csModule = default("^.cs"); 
        string xuModule = default("^.xu"); 
        int ndnMacMapping = default(11);
        bool forwarding;
        bool cacheUnsolicited = default(false);
        bool delays = default(true);
        @display("i=block/switch");
    gates:
        input upperLayerIn[];
        output upperLayerOut[];
        input lowerLayerIn[];
        output lowerLayerOut[];

        input pitIn;
        output pitOut;
        input fibIn;
        output fibOut;
        input csIn;
        output csOut;
        input xIn;
        output xOut;
}
