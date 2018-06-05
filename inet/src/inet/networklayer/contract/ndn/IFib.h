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

#ifndef __INET_IFIB_H
#define __INET_IFIB_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/ndn/packets/NdnPackets_m.h"

namespace inet {

class INET_API IFib
{
    public:
    class FibEntry
        {
        public:
            FibEntry(std::string prefix, cGate* face, MACAddress dest, int entryLifetime): prefix(prefix), face(face)
            {
                expireAt = simTime() + SimTime(entryLifetime, SIMTIME_MS);
                erased = false;
                macDest = dest;
            }

            std::string getPrefix(){ return prefix; }
            cGate* getFace(){ return face; }
            simtime_t getExpireAt(){ return expireAt; }
            MACAddress getMacDest(){ return macDest; }
            bool isErased(){ return erased; }
            void setFace(cGate* f){ face = f; }
            void setExpireAt(simtime_t t){ expireAt = t; }
            void setMacDest(MACAddress dest){ macDest = dest; }
            void markErased(){ erased = true; }
            void markNotErased() { erased = false; }

        private:
            MACAddress macDest;
            std::string prefix;
            cGate* face;
            simtime_t expireAt;
            bool erased;
        };

    class Notification : public cObject
        {
        public:
            const char *prefix;
            cGate* face;
            MACAddress macDest;

        public:
            Notification(const char *prefix, cGate *face, MACAddress dest): prefix(prefix), face(face) { macDest = dest; }
        };

public:
    virtual ~IFib() {}

    /* */
    virtual FibEntry* lookup(NdnPacket* packet) = 0;

    /* */
    virtual bool registerPrefix(const char* prefix, cGate* face, MACAddress dest) = 0;

    /* */
    virtual bool create(const char* prefix, short length, cGate* face, MACAddress dest) = 0;

    /* */
    virtual bool remove(const char* prefix) = 0;

    /* */
    virtual void cleanExpired() = 0;

};

} // namespace inet

#endif // ifndef __INET_IPIT_H

