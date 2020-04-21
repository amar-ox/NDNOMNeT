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

package inet.networklayer.ndn.forwarding;

import inet.networklayer.contract.ndn.IForwarding;

simple BF like IForwarding
{
    parameters:
        string interfaceTableModule;
        string pitModule = default("^.pit");
        string fibModule = default("^.fib");
        string csModule = default("^.cs"); 
        string xuModule = default("^.xu");       
        bool forwarding;
        bool delays = default(true);
        bool cacheUnsolicited = default(false);
        
        // for Blind Flooding
        int dw = default(255);
		double deferSlotTime = default(1.0);
		
        // for binary tree
        int fatherId = default(0);
        int treeDepth = default(0);
        int myLevel = default(0);
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
