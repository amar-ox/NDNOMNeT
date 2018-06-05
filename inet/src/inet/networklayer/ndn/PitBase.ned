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

import inet.networklayer.contract.ndn.IPit;

simple PitBase like IPit
{           
    parameters:
        int interestLifetime @unit("ms") = default(2000ms);
        int maxSize = default(100);             
        @signal[interestTimeout](type=IPit::Notification);   
        @display("i=block/cogwheel");
         
    gates:
        output pitOut;
        input pitIn;
}
