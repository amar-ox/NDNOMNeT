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

#ifndef __INET_PRODUCER_APP_BASE_H_
#define __INET_PRODUCER_APP_BASE_H_

#include <vector>
#include <string>
#include <math.h>
#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"

#include "inet/networklayer/ndn/packets/NdnPackets_m.h"
#include "inet/networklayer/ndn/packets/Tools.h"

#define REGISTER_PREFIX 15

using namespace omnetpp;

namespace inet {
class ProducerAppBase : public cSimpleModule, public ILifecycle
{
protected:
    // state
    NodeStatus *nodeStatus = nullptr;
    bool isOperational = false;

    /* Params */
    const char *pPrefix;
    int dataLength;
    int dataFreshness;
    simtime_t startTime;
    simtime_t stopTime;

    /* producer stat */
    int numIntReceived = 0;
    int numDataSent = 0;

    /* for binary tree */
    int nProducers = 0;
    int n = 0;

    /* Omnet stuff */
    virtual bool isNodeUp();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void finish() override;

    /* */
    virtual void handleMessage(cMessage *msg) override;

    /* */
    virtual void processInterest(NdnPacket *interest);

    /* */
    virtual void registerPrefix();

public:
    ProducerAppBase();
    virtual ~ProducerAppBase();
};

} //namespace

#endif
