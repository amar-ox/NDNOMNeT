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

#ifndef __INET_PIT_BASE_H_
#define __INET_PIT_BASE_H_

#include <omnetpp.h>
#include <vector>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ndn/IPit.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"


using namespace omnetpp;

namespace inet {

class INET_API PitBase : public cSimpleModule, public IPit
{
public:
    PitBase();
    virtual ~PitBase();

    /* */
    virtual bool create(Interest *interest, MACAddress src) override;

    /* */
    virtual IPit::PitEntry* lookup(const char* name) override;

    /* */
    virtual bool remove(const char* name) override;

    /* */
    virtual void cleanTimedout() override;

    /* */
    virtual void refreshDisplay() const override;

    /* */
    static simsignal_t interestTimeoutSignal;

private:
    std::vector<IPit::PitEntry *> entries;
    unsigned maxSize;
    cMessage *checkTimeout = new cMessage("to");

    int interestLifetime;
    int numAdded = 0;
    int numRemoved = 0;
    int numMatched = 0;
    int numMissed = 0;
    int numTimeout = 0;
    cLongHistogram sizeStat;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();
    virtual void print();
};

} //namespace

#endif
