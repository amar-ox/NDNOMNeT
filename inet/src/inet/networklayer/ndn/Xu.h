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

#ifndef __INET_XU_H_
#define __INET_XU_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ndn/IXu.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"

using namespace omnetpp;

namespace inet {

class INET_API Xu : public cSimpleModule, public IXu
{
public:
    Xu();
    virtual ~Xu();

    /* */
    virtual bool processPacket(NdnPacket *packet) override;

    /* */
    virtual bool sendResponse(NdnPacket* packet) override;

    /* */
    static simsignal_t pakcetReceivedSignal;
    static simsignal_t pakcetProcessingBeginSignal;
    static simsignal_t pakcetProcessingEndSignal;
    static simsignal_t pakcetProcessingErrorSignal;
    static simsignal_t pakcetProcessingSuccessSignal;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void refreshDisplay() const;
    virtual void finish();
};

}  //namespace

#endif
