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

#ifndef __NDN_ILNFS_H
#define __NDN_ILNFS_H

#include <vector>
#include <string>
#include <math.h>

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/linklayer/common/SimpleLinkLayerControlInfo.h"
#include "inet/linklayer/contract/IMACProtocolControlInfo.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/common/InterfaceTable.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/ndn/IPit.h"
#include "inet/networklayer/contract/ndn/IFib.h"
#include "inet/networklayer/contract/ndn/ICs.h"
#include "inet/networklayer/contract/ndn/IForwarding.h"

#include "inet/common/LayeredProtocolBase.h"
#include "inet/linklayer/base/MACProtocolBase.h"

#include "PitBase.h"
#include "FibBase.h"
#include "FibIlnfs.h"
#include "CsBase.h"
#include "Xu.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"

#define DW 127
#define DEFER_SLOT_TIME 0.032
#define TIMEOUT_CODE 100
#define REGISTER_PREFIX_CODE 15
#define DEFAULT_MAC_IF 0

#define M 5.0
#define N 3.5
#define DELTA_MAX 9.
#define TH 0.75
#define ALPHA 0.85

namespace inet {
class INET_API ILNFS : public cSimpleModule, public IForwarding, public ILifecycle, protected cListener
{
protected:
    SendDelayed* sendDelayedPacket = new SendDelayed("ft");
    bool waiting = false;
    MACAddress myMacAddress;
    IInterfaceTable *ift = nullptr;
    int ndnMacMapping;
    bool forwarding;
    bool cacheUnsolicited;
    bool delays;

    /* NDN tables */
    PitBase *pit;
    FibIlnfs *fib;
    CsBase *cs;
    MACProtocolBase *mac;

    /* router stat */
    int numDropped = 0;
    int numIntReceived = 0;
    int numIntFwd = 0;
    int numIntCanceled = 0;
    int numIntDuplicated = 0;
    int numDataCanceled = 0;
    int numDataReceived = 0;
    int numDataUnsolicited = 0;
    int numDataFwd = 0;
    cOutVector deltaStat;
    cOutVector nsStat;

    /* delay adjustment */
    int neighborI = 0;
    int neighborD = 0;

    /* reset Stats */
    cMessage *resetStatTimer = new cMessage("resetStat");

    /* Omnet stuff */
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual void finish() override;

    /* */
    virtual void handleMessage(cMessage *msg) override;

    /* */
    virtual void dispatchPacket(NdnPacket *packet) override;

    /* */
    virtual void processLLInterest(Interest *interest, MACAddress macSrc) override;

    /* */
    virtual void processLLData(Data *data, MACAddress macSrc) override;

    /* */
    virtual void processHLInterest(Interest *interest) override;

    /* */
    virtual void processHLData(Data *data) override;

    /* */
    virtual void forwardInterestToRemote(Interest* interest, int face, MACAddress macSrc, MACAddress macDest) override;

    /* */
    virtual void forwardDataToRemote(Data* data, int face, MACAddress macDest) override;

    /* */
    virtual void forwardInterestToLocal(Interest* interest, int face, MACAddress macSrc) override;

    /* */
    virtual void forwardDataToLocal(Data* data, int face) override;

    /* */
    virtual void ndnToMacMapping(NdnPacket *packet, MACAddress macDest) override;

    /* */
    virtual double computeInterestRandomDelay();

    /* */
    virtual double computeDataRandomDelay();

    /* */
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }

    /* */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;

    /* */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    /* */
    virtual float computeTheta();

    /* */
    virtual float computeDelay(float delta);

  public:
    ILNFS();
    virtual ~ILNFS();
};

} // namespace inet

#endif

