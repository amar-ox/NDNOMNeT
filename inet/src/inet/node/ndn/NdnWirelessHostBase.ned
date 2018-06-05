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

package inet.node.ndn;

import inet.node.inet.INetworkNode;
import inet.common.lifecycle.NodeStatus;
import inet.linklayer.contract.IWirelessNic;
import inet.mobility.contract.IMobility;
import inet.networklayer.ndn.NdnL3;
import inet.power.contract.IEnergyGenerator;
import inet.power.contract.IEnergyStorage;
import inet.networklayer.common.InterfaceTable;

module NdnWirelessHostBase like INetworkNode
{
    parameters:
        @display("i=device/pocketpc_s;bgb=339,425");
        @networkNode;
        @labels(wireless-NDN-node);
        bool hasStatus = default(true);
        int numRadios = default(1);
        string mobilityType = default("StationaryMobility");
        string energyStorageType = default("");
        string energyGeneratorType = default("");
        bool forwarding = default(true);
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        **.forwarding = forwarding;

    gates:
        input radioIn[numRadios] @directIn;
        inout pppg[] @labels(PPPFrame-conn);
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        status: NodeStatus if hasStatus {
            @display("p=63,142");
        }
        energyStorage: <energyStorageType> like IEnergyStorage if energyStorageType != "" {
            parameters:
                @display("p=63,220;i=block/plug;is=s");
        }
        energyGenerator: <energyGeneratorType> like IEnergyGenerator if energyGeneratorType != "" {
            parameters:
                @display("p=63,295;i=block/plug;is=s");
        }
        mobility: <mobilityType> like IMobility {
            parameters:
                @display("p=63,364");
        }
        ndnL3: NdnL3 {
            parameters:
                @display("p=216,159;q=queue");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=63,53;is=s");
        }
        wlan[numRadios]: <default("WirelessNic")> like IWirelessNic {
            parameters:
                @display("p=216,277,row,60;q=queue");
        }

    connections allowunconnected:
        for i=0..sizeof(radioIn)-1 {
            radioIn[i] --> { @display("m=s"); } --> wlan[i].radioIn;
            wlan[i].upperLayerOut --> ndnL3.ifIn++;
            wlan[i].upperLayerIn <-- ndnL3.ifOut++;
        }
}
