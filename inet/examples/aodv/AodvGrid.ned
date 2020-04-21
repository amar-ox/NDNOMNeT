//
// Copyright (C) 2014 OpenSim Ltd.
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

package inet.examples.aodv;

import inet.common.lifecycle.LifecycleController;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.aodv.AODVRouter;
import inet.physicallayer.idealradio.IdealRadioMedium;


network AodvGridNetwork
{
    parameters:        
        int numRouters;
        int numProducers;
        int numConsumers;
        @display("bgb=160,160;bgg=100,1,grey95");
    submodules:
        radioMedium: IdealRadioMedium {
            parameters:
                @display("p=5,10");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='145.236.x.x' netmask='255.255.0.0'/></config>");
                @display("p=5,50");
        }
        routingTableRecorder: RoutingTableRecorder {
            parameters:
                @display("p=5,100");
        }
        lifecycleController: LifecycleController {
            parameters:
                @display("p=5,150");
        }
        scenarioManager: ScenarioManager {
            parameters:
                script = default(xml("<scenario/>"));
                @display("p=5,30");
        }
        
		////////////////////////////////////////////////////  
        aodvP[numProducers] : AODVRouter {
            parameters:
                @display("p=80,80");
        }
                
        aodvR[numRouters]: AODVRouter {
            parameters:
                @display("p=0,0");
        }
              
        aodvC[numConsumers] : AODVRouter {
            parameters:
                @display("p=80,80");
        }
		///////////////////////////////////////////////////      
}
