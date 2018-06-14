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

#ifndef __NDN_BF_H
#define __NDN_BF_H

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

#include "PitBase.h"
#include "FibBase.h"
#include "CsBase.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"

#define DW 1023
#define DEFER_SLOT_TIME 0.028
#define HISTORY_SIZE 10
#define TIMEOUT_CODE 100
#define REGISTER_PREFIX_CODE 15
#define DEFAULT_MAC_IF 0

namespace inet {
class INET_API BF : public cSimpleModule, public IForwarding, public ILifecycle, protected cListener
{
protected:
    SendDelayed* sendDelayedPacket = new SendDelayed("ft");
    bool waiting = false;
    MACAddress myMacAddress;
    IInterfaceTable *ift = nullptr;
    int ndnMacMapping;
    bool forwarding;
    bool cacheUnsolicited;

    /* NDN tables */
    PitBase *pit;
    FibBase *fib;
    CsBase *cs;

    /* router stat */
    int numIntReceived = 0;
    int numIntFwd = 0;
    int numIntCanceled = 0;
    int numIntDuplicated = 0;
    int numDataCanceled = 0;
    int numDataReceived = 0;
    int numDataUnsolicited = 0;
    int numDataFwd = 0;

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

    /**
     * ILifecycle method
     */
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    /* */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

  public:
    BF();
    virtual ~BF();
};

} // namespace inet

#endif // ifndef __NDN_FORWARDING_H

