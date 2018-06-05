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

#ifndef __INET_IXU_H
#define __INET_IXU_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ndn/packets/NdnPackets_m.h"

namespace inet {

class INET_API IXu
{
    public:
    class Notification : public cObject
        {
        public:
            NdnPacket *packet;
            short opCode;
            short resCode;

        public:
            Notification(NdnPacket *packet, short oc, short rc)
            : packet(packet), opCode(oc), resCode(rc){}
        };

public:
    virtual ~IXu() {}

    /* */
    virtual bool processPacket(NdnPacket *packet) = 0;

    /* */
    virtual bool sendResponse(NdnPacket* packet) = 0;
};

} // namespace inet

#endif // ifndef __INET_XU_H

