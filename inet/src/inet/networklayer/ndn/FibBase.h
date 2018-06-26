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

#ifndef __INET_FIB_BASE_H_
#define __INET_FIB_BASE_H_

#include <vector>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ndn/IFib.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"

using namespace omnetpp;
namespace inet {


class INET_API FibBase : public cSimpleModule, public IFib
{
public:
    FibBase();

    /* */
    virtual ~FibBase();

    /* */
    virtual BaseEntry* lookup(NdnPacket* packet) override;

    /* */
    virtual bool registerPrefix(const char* prefix, cGate* face, MACAddress dest) override;

    /* */
    virtual bool create(const char* name, short length, cGate* face, MACAddress dest, float p1 = 0, float p2 = 0) override;

    /* */
    virtual bool remove(const char* prefix) override;

    /* */
    virtual void cleanExpired() override;

    /* */
    virtual bool setToBroadcast(const char* name);

    /* */
    virtual void refreshDisplay() const override;

    /* */
    virtual void finish() override;

    /* */
    static simsignal_t entryExpiredSignal;

protected:
    std::vector<BaseEntry *> entries;
    cMessage *checkExpired = new cMessage("pe");
    unsigned maxSize;
    int entryLifetime;
    int numAdded = 0;
    int numRemoved = 0;
    int numMatched = 0;
    int numMissed = 0;
    int numExpired = 0;
    cLongHistogram prefixLengthStat;

    /* */
    virtual bool match(const char* name, unsigned* index);

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void print();
};

} //namespace

#endif
