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

#include "FibBase.h"

namespace inet {
using std::cout;

simsignal_t FibBase::entryExpiredSignal = cComponent::registerSignal("entryExpired");
Register_Abstract_Class(IFib::Notification);

Define_Module(FibBase);

FibBase::FibBase()
{
}

FibBase::~FibBase()
{
    cancelAndDelete(checkExpired);
    entries.erase(entries.begin(), entries.end());
}

void FibBase::initialize()
{
    maxSize = par("maxSize");
    entryLifetime = par("entryLifetime");
    entries.clear();
}

void FibBase::handleMessage(cMessage *msg)
{
    if ( msg->isSelfMessage() ){
        cleanExpired();
        if (entries.size())
            scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkExpired);
    }else
        delete msg;
}

IFib::FibEntry* FibBase::lookup(NdnPacket* packet)
{
    Enter_Method("lookup(...)");
    unsigned index = -1;
    if ( match(packet->getName(), &index) ){
        if ( entries.at(index)->isErased() ){
            cout << simTime() << "\t" << getFullPath() << ": Obsolete entry found ("<< packet->getName() << ")" << endl;
            return nullptr;
        }
        cout << simTime() << "\t" << getFullPath() << ": Matching found ("<< packet->getName() << ")" << endl;
        numMatched++;
        return entries.at(index);
    }
    cout << simTime() << "\t" << getFullPath() << ": No matching found ("<< packet->getName() << ")" << endl;
    numMissed++;
    return nullptr;
}

bool FibBase::registerPrefix(const char* prefix, cGate* face, MACAddress dest)
{
    Enter_Method("registerPrefix(...)");
    unsigned index = -1;
    if ( !match(prefix, &index) && (entries.size() < maxSize) ){
        std::string stringPrefix(prefix, strlen(prefix));
        cout << simTime() << "\t" << getFullPath() << ": Insert prefix: " << stringPrefix << " | " << "Face: " << face << " (local)" << endl;
        FibEntry *e = new FibEntry(stringPrefix, face, dest, 100000);
        entries.push_back(e);
        numAdded++;
        return true;
    }else{
        cout << simTime() << "\t" << getFullPath() << ": Cannot insert prefix from: "<< prefix << endl;
        return false;
    }
}

bool FibBase::create(const char* name, short length, cGate* face, MACAddress dest)
{
    Enter_Method("create(...)");
    unsigned index = -1;
    if ( match(name, &index) ){ // TODO: improve matching lookup
        cout << simTime() << "\t" << getFullPath() << ": Update prefix: " << name << ": " << dest.str().c_str() << endl;
        entries.at(index)->setMacDest(dest);
        entries.at(index)->markNotErased();
        entries.at(index)->setExpireAt(simTime() + SimTime(entryLifetime, SIMTIME_MS));
        return true;
    }
    if ( entries.size() >= maxSize ){
        cout << simTime() << "\t" << getFullPath() << ": FIB full. Cannot insert (" << name << ")" << endl;
        return false;
    }
    cout << simTime() << "\t" << getFullPath() << ": Insert prefix: " << name << ": " << dest.str().c_str() << endl;
    std::string prefix(name, length);
    FibEntry *e = new FibEntry(prefix, face, dest, entryLifetime);
    entries.push_back(e);
    numAdded++;
    if (! checkExpired->isScheduled())
        scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkExpired);
    print();
    return true;
}

bool FibBase::remove(const char* prefix)
{
    Enter_Method("remove(...)");
    if (entries.empty())
        return false;
    std::vector<FibEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        if ( ! strcmp((*it)->getPrefix().c_str(), prefix) ){
            cout << simTime() << "\t" << getFullPath() << ": Remove expired entry (" << prefix << ")" << endl;
            entries.erase(it);
            numRemoved++;
            return true;
        }
    }
    return false;
}

void FibBase::cleanExpired()
{
    if (entries.empty())
        return;
    std::vector<FibEntry *>::iterator it; /* FIXME: use auto*/
    for (it = entries.begin(); it != entries.end(); ++it){
        if ( !(*it)->isErased() && ((*it)->getExpireAt() <= simTime()) ){
            numExpired++;
            cout << simTime() << "\t" << getFullPath() << ": Update expired entry (" << (*it)->getPrefix().c_str() << ")" << endl;
            Notification signal((*it)->getPrefix().c_str(), (*it)->getFace(), (*it)->getMacDest());
            emit(entryExpiredSignal, &signal);
            (*it)->markErased();
        }
    }
}

bool FibBase::match(const char* name, unsigned* index)
{
    if ( entries.empty() )
        return false;
    for (unsigned i = 0; i < entries.size(); i++){
        size_t prefixLength = entries.at(i)->getPrefix().length();
        if ( ! strncmp(entries.at(i)->getPrefix().c_str(), name, prefixLength) ){
            *index = i;
            return true;
        }
    }
    return false;
}

void FibBase::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "Size: %d entries", (int)entries.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void FibBase::finish()
{
    recordScalar("added", numAdded);
    recordScalar("removed", numRemoved);
    recordScalar("matched", numMatched);
    recordScalar("missed", numMissed);
    recordScalar("expired", numExpired);

    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    cout << getFullPath() << ": " << numAdded << " added, " << numRemoved << " removed, " << numMatched << " matched, " << numMissed << " missed, " << numExpired << " expired" << endl;
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    //prefixLengthStat.recordAs("prefixLength");
}

void FibBase::print()
{
    Enter_Method("print()");
    char buf[65];
    cout << getFullPath() << ": -- Forwarding Information Base --" << endl;
    sprintf (buf, "%-32s %-15s %-17s\n", "Prefix", "Face", "MAC@");
    cout << buf;
    std::vector<FibEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        sprintf (buf, "%-32s %-15s %-17s\n",
                (*it)->getPrefix().c_str(),
                (*it)->getFace()->getName(),
                (*it)->getMacDest().str().c_str());
        cout << buf;
    }
    cout << endl;
}

} //namespace
