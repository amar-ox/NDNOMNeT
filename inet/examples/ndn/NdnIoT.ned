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
import inet.physicallayer.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import inet.node.ndn.NdnWirelessHost;
import inet.node.ndn.NdnWirelessHostBase;


// NDN over IEEE802.15.4 in IoT network
network NdnIoT
{
    parameters:
        int numRouters;
        int numProducers;
        int numConsumers;
        @display("bgb=300,300;bgg=100,1,grey95");
    submodules:
        radioMedium: IdealRadioMedium {
            @display("p=649,75");
        }
        ndnP[numProducers] : NdnWirelessHost {
            parameters:
                @display("p=90,90");
        }
                
        ndnR[numRouters]: NdnWirelessHostBase {
            parameters:
                @display("p=0,0");
        }
              
        ndnC[numConsumers] : NdnWirelessHost {
            parameters:
                @display("p=90,90");
        } 
}

