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

#ifndef __INET_ICS_H
#define __INET_ICS_H

#include "inet/common/INETDefs.h"
#include "inet/networklayer/ndn/packets/NdnPackets_m.h"

namespace inet {

class INET_API ICs
{
public:
    class CsEntry
    {
    public:
        CsEntry(Data* data) : data(data)
        {
            fresh = (data->getFreshnessPeriod() > 0);
            if (fresh)
                staleAt = simTime() + SimTime(data->getFreshnessPeriod(), SIMTIME_MS);
            else
                staleAt = SIMTIME_ZERO;
        }

        ~CsEntry()
        {
        }

        simtime_t getStaleAt() { return staleAt; }
        void setStaleAt(simtime_t t) { staleAt = t; }
        Data* getData() { return data; }
        bool getFresh() { return fresh; }
        void setFresh(bool f) { fresh = f; }
        void setData(Data* d) {data = d;}

    private:
        Data* data;
        bool fresh;
        simtime_t staleAt;
    };

    class Notification : public cObject
    {
    public:
        Notification(Data *data) : data(data){}
        Data *data;
    };

public:
    virtual ~ICs() {}

    /* */
    virtual bool add(Data *data) = 0;

    /* */
    virtual Data* lookup(Interest* interest) = 0;

    /* */
    virtual bool remove(const char* name) = 0;

    /* */
    virtual void cleanStaled() = 0;
};

} // namespace inet

#endif // ifndef __INET_ICS_H

