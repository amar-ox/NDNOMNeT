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

package inet.examples.ndn;

import inet.physicallayer.idealradio.IdealRadioMedium;
import inet.node.ndn.NdnWirelessHost;

network NdnGateway
{
    parameters:
        int numHosts;        
        @display("bgb=200,200;bgg=100,1,grey95");
    submodules:
        radioMedium: IdealRadioMedium {
            @display("p=649,75");
        }
        
        ndnLM: NdnWirelessHost {            
            @display("i=device/router;p=100,100");
        }
       
        ndnNED[numHosts]: NdnWirelessHost {            
            @display("p=100,100");
        }
}

