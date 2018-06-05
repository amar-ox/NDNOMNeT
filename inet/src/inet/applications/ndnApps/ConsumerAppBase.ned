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

simple ConsumerAppBase
{    
     parameters:        
        double startTime @unit("s") = default(1s);
        double stopTime @unit("s") = default(-1s);                
        string prefix = default("-");
        int interestLength @unit("B") = default(0);        
        int sendInterval @unit("s") = default(1s);
        int numInterests = default(-1);
        int interestReTx = default(1);
        int interestLifetime @unit("ms") = default(4000ms)
              
        @display("i=block/sink");
    gates:
        input consumerIn;
        output consumerOut;        
}
