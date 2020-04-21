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

#include "PitBase.h"

#include <sstream>

namespace inet {
using std::cout;

simsignal_t PitBase::interestTimeoutSignal = cComponent::registerSignal("interestTimeout");
Register_Abstract_Class(IPit::Notification);
Define_Module(PitBase);

PitBase::PitBase()
{
}

PitBase::~PitBase()
{
    cancelAndDelete(checkTimeout);
    for (unsigned int i(0); i < entries.size(); ++i){
        delete entries[i]->getInterest();
        entries.erase(entries.begin() + (i--));
        numRemoved++;
    }
}

void PitBase::initialize()
{
    maxSize = par("maxSize");
    interestLifetime = par("interestLifetime");
    entries.clear();
}

void PitBase::handleMessage(cMessage *msg)
{
    if ( msg->isSelfMessage() ){
        cleanTimedout();
        if (entries.size())
            scheduleAt(simTime() + 1, msg);
    }else
        delete msg;
}

bool PitBase::create(Interest *interest, MACAddress src)
{
    Enter_Method("create(...)");
    if ( entries.size() == maxSize )
        return false;
    cout << simTime() << "\t" << getFullPath() << ": Add entry. Interest: " << interest->getName() << ": " << src.str().c_str() << endl;
    Interest* interstCopy = interest->dup();
    entries.push_back(new PitEntry(interstCopy, interest->getArrivalGate(), src));
    numAdded++;
    if (! checkTimeout->isScheduled())
        scheduleAt(simTime() + SimTime(10, SIMTIME_MS), checkTimeout);
    return true;
}

bool PitBase::remove(const char* name)
{
    Enter_Method("remove(...)");
    if (entries.empty())
        return false;
    std::vector<PitEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it)
        if ( !(*it)->isErased() && !strcmp((*it)->getInterest()->getName(), name) ){
            cout << simTime() << "\t" << getFullPath() << ": Remove entry (" << name << ")" << endl;
            delete (*it)->getInterest();
            entries.erase(it);
            numRemoved++;
            return true;
        }
    return false;
}

IPit::PitEntry* PitBase::lookup(const char* name)
{
    Enter_Method("lookup(...)");
    if (entries.empty())
        return nullptr;
    sizeStat.collect(entries.size());
    std::vector<PitEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it)
        if ( !(*it)->isErased() && !strcmp((*it)->getInterest()->getName(), name) ){
            numMatched++;
            return *it;
        }
    numMissed++;
    return nullptr;
}

void PitBase::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "Size: %d entries", (int)entries.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void PitBase::cleanTimedout()
{
    std::vector<PitEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        if ( !(*it)->isErased() && ((*it)->getExpireAt() <= simTime()) ){
            numTimeout++;
            cout << simTime() << "\t" << getFullPath() << ": Interest timeout (" << (*it)->getInterest()->getName() << ")"<< endl;
            Notification signal((*it)->getInterest(), (*it)->getFace(), (*it)->getMacSrc());
            emit(interestTimeoutSignal, &signal);
            (*it)->markErased();
        }
    }
    for (it = entries.begin(); it != entries.end();){
        if ((*it)->isErased()){
            delete (*it)->getInterest();
            it = entries.erase(it);
            numRemoved++;
        }
        else
            ++it;
    }
}

void PitBase::finish()
{
    recordScalar("added", numAdded);
    recordScalar("removed", numRemoved);
    recordScalar("matched", numMatched);
    recordScalar("missed", numMissed);
    recordScalar("timeout", numTimeout);
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    cout << getFullPath() << ": " << numAdded << " added, " << numRemoved << " removed, " << numMatched << " matched, " << numMissed << " missed, " << numTimeout << " timeout" << endl;
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    //sizeStat.recordAs("size");
}

void PitBase::print()
{
    Enter_Method("print()");
    char buf[70];
    cout << getFullPath() << ": -- Pending Interest Table --" << endl;
    sprintf (buf, "%-32s %-15s %-17s %-6s\n", "Interest", "Face", "MAC@", "Erased");
    cout << buf;
    std::vector<PitEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        sprintf (buf, "%-32s %-15s %-17s %-6d\n",
                (*it)->getInterest()->getName(),
                (*it)->getFace()->getName(),
                (*it)->getMacSrc().str().c_str(),
                (*it)->isErased());
        cout << buf;
    }
    cout << endl;
}

} //namespace
