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

#ifndef __INET_IPIT_H
#define __INET_IPIT_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ndn/packets/NdnPackets_m.h"

namespace inet {

class INET_API IPit
{
    public:
    class PitEntry
        {
        public:
            PitEntry(Interest* interest, cGate* face, MACAddress src): interest(interest), face(face)
            {
                expireAt = simTime() + SimTime(interest->getInterestLifetimeMs(), SIMTIME_MS);
                erased = false;
                macSrc = src;
            }

            Interest* getInterest(){ return interest; }
            cGate* getFace(){ return face; }
            bool isErased(){ return erased; }
            simtime_t getExpireAt(){ return expireAt; }
            MACAddress getMacSrc(){ return macSrc; }

            void markErased(){ erased = true; }
            void setExpireAt(simtime_t t){ expireAt = t; }

        private:
            MACAddress macSrc;
            Interest *interest;
            cGate* face;
            bool erased;
            simtime_t expireAt;
        };

    class Notification : public cObject
        {
        public:
            Interest *interest;
            cGate* face;
            MACAddress macSrc;

        public:
            Notification(Interest *interest, cGate *face, MACAddress src): interest(interest), face(face){ macSrc = src; }
        };

public:
    virtual ~IPit() {}

    /* */
    virtual bool create(Interest *interest, MACAddress src) = 0;

    /* */
    virtual IPit::PitEntry* lookup(const char* name) = 0;

    /* */
    virtual bool remove(const char* name) = 0;

    /* */
    virtual void cleanTimedout() = 0;
};

} // namespace inet

#endif // ifndef __INET_IPIT_H

