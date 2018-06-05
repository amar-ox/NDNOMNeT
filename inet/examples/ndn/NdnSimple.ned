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

import inet.node.ndn.NdnWirelessHost;
import inet.node.ndn.NdnWirelessHostBase;
import inet.physicallayer.idealradio.IdealRadioMedium;


network NdnSimple
{
    parameters:
        @display("bgb=100,100;bgg=100,1,grey95");
    submodules:
        radioMedium: IdealRadioMedium {
            @display("p=86.666664,12.48");
        }
        ndnCons: NdnWirelessHost {
            @display("p=20,20");
        }
        ndnRt0: NdnWirelessHostBase {
            @display("p=46,34");
        }
        ndnRt1: NdnWirelessHostBase {
            @display("p=31,48");
        }
        ndnProd: NdnWirelessHost {
            @display("p=60,60");
        }
}

