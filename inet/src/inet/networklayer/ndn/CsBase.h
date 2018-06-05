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

#ifndef __INET_CS_BASE_H_
#define __INET_CS_BASE_H_

#include <vector>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ndn/ICs.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"

using namespace omnetpp;
namespace inet {

class INET_API CsBase : public cSimpleModule, public ICs
{
public:
    CsBase();

    /* */
    virtual ~CsBase();

    /* */
    virtual Data* lookup(Interest* interest) override;

    /* */
    virtual bool add(Data *data) override;

    /* */
    virtual bool remove(const char* name) override;

    /* */
    virtual void cleanStaled() override;

    /* */
    virtual void refreshDisplay() const override;

    /* */
    virtual void finish() override;

    /* */
    static simsignal_t dataStaleSignal;

private:
    std::vector<CsEntry *> entries;
    cMessage *checkDataStale = new cMessage("ds");
    unsigned maxSize;

    int numHit = 0;
    int numMiss = 0;
    int numAdded = 0;
    int numRemoved = 0;
    int numStale = 0;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void print();
};

} //namespace

#endif
