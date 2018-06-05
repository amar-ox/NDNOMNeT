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

#ifndef __INET_IFORWARDING_H
#define __INET_IFORWARDING_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ndn/packets/NdnPackets_m.h"

namespace inet {

class INET_API IForwarding
{
public:
    virtual ~IForwarding() {}

    /* */
    virtual void dispatchPacket(NdnPacket *packet) = 0;

    /* */
    virtual void processLLInterest(Interest *interest, MACAddress macSrc) = 0;

    /* */
    virtual void processLLData(Data *data, MACAddress macSrc) = 0;

    /* */
    virtual void processHLInterest(Interest *interest) = 0;

    /* */
    virtual void processHLData(Data *data) = 0;

    /* */
    virtual void forwardInterestToRemote(Interest* interest, int face, MACAddress macSrc, MACAddress macDest) = 0;

    /* */
    virtual void forwardDataToRemote(Data* data, int face, MACAddress macDest) = 0;

    /* */
    virtual void forwardInterestToLocal(Interest* interest, int face, MACAddress macSrc) = 0;

    /* */
    virtual void forwardDataToLocal(Data* data, int face) = 0;

    /* */
    virtual void ndnToMacMapping(NdnPacket *packet, MACAddress macDest) = 0;
};

} // namespace inet

#endif // ifndef __INET_IFORWARDING_H

