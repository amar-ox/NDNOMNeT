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

#ifndef __INET_FIB_ILNFS_H_
#define __INET_FIB_ILNFS_H_

#include <vector>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ndn/IFib.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"

using namespace omnetpp;
namespace inet {

class INET_API IlnfsEntry : public BaseEntry
{
public:
    IlnfsEntry(std::string prefix, cGate* face, MACAddress dest, int entryLifetime, float mhc, float a)
    : BaseEntry(prefix, face, dest, entryLifetime), minHeardCost(mhc)
    {
        cost = 0.;
        cost = (float)(1. - (float)a) * (float)cost + (float)a * (float)(1. + (float)mhc);
    }

    float getCost() { return cost; }
    void resetCost(float max_delta)
    {
        minHeardCost = max_delta;
        cost = 0.;
    }
    void updateCost(float c, float a, MACAddress dest)
    {
        if ( c < minHeardCost ){
            minHeardCost = c;
            cost = (float)(1. - (float)a) * cost + (float)a * (float)(1. + (float)minHeardCost);
            macDest = dest;
        }
    }

private:
    float minHeardCost;
    float cost;
};


class INET_API FibIlnfs : public cSimpleModule, public IFib
{
public:
    FibIlnfs();

    /* */
    virtual ~FibIlnfs();

    /* */
    virtual BaseEntry* lookup(NdnPacket* packet) override;

    /* */
    virtual bool registerPrefix(const char* prefix, cGate* face, MACAddress dest) override;

    /* */
    virtual bool remove(const char* prefix) override;

    /* */
    virtual void cleanExpired() override;

    /* */
    virtual void refreshDisplay() const override;

    /* */
    virtual void finish() override;

    /* */
    virtual bool create(const char* name, short length, cGate* face, MACAddress dest, float mhc = 0, float a = 1) override;

    /* */
    virtual float updateCost(const char* name, short length, cGate* face, MACAddress dest, float c, float a);

    /* */
    virtual bool resetCost(const char* name, float max_delta);

public:
    /* */
    static simsignal_t entryExpiredSignal;

private:
    std::vector<IlnfsEntry *> entries;
    //cMessage *checkExpired = new cMessage("pe");
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

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void print();
};

} //namespace

#endif
