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
import inet.node.ndn.NdnWirelessHostBase;

network NdnBinaryTree
{
    parameters:
        int label; 
        @display("bgb=200,200;bgg=100,1,grey95");
    submodules:
        radioMedium: IdealRadioMedium {
            @display("p=200,200");
        }
        /////////////////////////////////
        ndnGW: NdnWirelessHost {
            @display("i=device/router;p=100,0");
        }
        /////////////////////////////////
        n11: NdnWirelessHostBase {
            @display("p=85,30");
        }
        n12: NdnWirelessHostBase {
            @display("p=115,30");
        }
        /////////////////////////////////
        n112: NdnWirelessHostBase {
            @display("p=80,60");
        }
        n111: NdnWirelessHostBase {
            @display("p=55.08,45.56");
        }

        n121: NdnWirelessHostBase {
            @display("p=120,60");
        }
        n122: NdnWirelessHostBase {
            @display("p=144.16,45.56");
        }
        /////////////////////////////////
        p1111: NdnWirelessHost {
            @display("p=34.68,68");
        }
        p1112: NdnWirelessHost {
            @display("p=27.880001,45.56");
        }
        p1121: NdnWirelessHost {
            @display("p=89,89");
        }
        p1122: NdnWirelessHost {
            @display("p=69.36,89.76");
        }

        p1211: NdnWirelessHost {
            @display("p=114,89");
        }
        p1212: NdnWirelessHost {
            @display("p=135,90");
        }

        p1221: NdnWirelessHost {
            @display("p=167.28,61.2");
        }
        p1222: NdnWirelessHost {
            @display("p=176.8,45.56");
        }














}

