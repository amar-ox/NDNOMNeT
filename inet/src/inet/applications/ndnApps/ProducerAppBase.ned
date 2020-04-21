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

package inet.applications.ndnApps;

simple ProducerAppBase
{
    parameters:        
        string prefix = default("*");        
        int dataLength @unit("B") = default(0);
        int dataFreshness @unit("ms") = default(0);
        double startTime @unit("s") = default(0s);
        double stopTime @unit("s") = default(-1s);
        
        // for binary tree
        int nProducers;
        int n = default(-1);
                                   
        @display("i=block/source");
    gates:
        input producerIn;
        output producerOut;
}
