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

#include "FibIlnfs.h"

namespace inet {
using std::cout;

simsignal_t FibIlnfs::entryExpiredSignal = cComponent::registerSignal("entryExpired");
Register_Abstract_Class(IFib::Notification);
Define_Module(FibIlnfs);

FibIlnfs::FibIlnfs()
{
}

FibIlnfs::~FibIlnfs()
{
    //cancelAndDelete(checkExpired);
    entries.erase(entries.begin(), entries.end());
}

void FibIlnfs::initialize()
{
    maxSize = par("maxSize");
    entryLifetime = par("entryLifetime");
    entries.clear();
}

void FibIlnfs::handleMessage(cMessage *msg)
{
    //if ( msg->isSelfMessage() ){
        //cleanExpired();
        //if (entries.size())
        //    scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkExpired);
    //}else
        delete msg;
}

BaseEntry* FibIlnfs::lookup(NdnPacket* packet)
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

bool FibIlnfs::registerPrefix(const char* prefix, cGate* face, MACAddress dest)
{
    Enter_Method("registerPrefix(...)");
    unsigned index = -1;
    if ( !match(prefix, &index) && (entries.size() < maxSize) ){
        std::string stringPrefix(prefix, strlen(prefix));
        cout << simTime() << "\t" << getFullPath() << ": Insert prefix: " << stringPrefix << " | " << "Face: " << face << " (local)" << endl;
        IlnfsEntry *e = new IlnfsEntry(stringPrefix, face, dest, 100000, 0, 1);
        entries.push_back(e);
        numAdded++;
        return true;
    }else{
        cout << simTime() << "\t" << getFullPath() << ": Cannot insert prefix from: "<< prefix << endl;
        return false;
    }
}

bool FibIlnfs::create(const char* name, short length, cGate* face, MACAddress dest, float mhc, float a)
{
    Enter_Method("create(...)");
    unsigned index = -1;
    if ( match(name, &index) ){
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
    IlnfsEntry *e = new IlnfsEntry(prefix, face, dest, entryLifetime, mhc, a);
    entries.push_back(e);
    numAdded++;
    //if (! checkExpired->isScheduled())
    //    scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkExpired);
    return true;
}

bool FibIlnfs::remove(const char* prefix)
{
    Enter_Method("remove(...)");
    if (entries.empty())
        return false;
    std::vector<IlnfsEntry *>::iterator it;
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

void FibIlnfs::cleanExpired()
{
    if (entries.empty())
        return;
    std::vector<IlnfsEntry *>::iterator it;
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

bool FibIlnfs::match(const char* name, unsigned* index)
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

void FibIlnfs::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "Size: %d entries", (int)entries.size());
    getDisplayString().setTagArg("t", 0, buf);
}

void FibIlnfs::finish()
{
    recordScalar("added", numAdded);
    recordScalar("removed", numRemoved);
    recordScalar("matched", numMatched);
    recordScalar("missed", numMissed);
    recordScalar("expired", numExpired);

    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    cout << getFullPath() << ": " << numAdded << " added, " << numRemoved << " removed, " << numMatched << " matched, " << numMissed << " missed, " << numExpired << " expired" << endl;
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
}

void FibIlnfs::print()
{
    Enter_Method_Silent();
    char buf[65];
    cout << getFullPath() << ": -- Forwarding Information Base --" << endl;
    sprintf (buf, "%-32s %-15s %-17s\n", "Prefix", "Face", "MAC@");
    cout << buf;
    std::vector<IlnfsEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        sprintf (buf, "%-32s %-15s %-17s\n",
                (*it)->getPrefix().c_str(),
                (*it)->getFace()->getName(),
                (*it)->getMacDest().str().c_str());
        cout << buf;
    }
    cout << endl;
}

float FibIlnfs::updateCost(const char* name, short length, cGate* face, MACAddress dest, float c, float a)
{
    Enter_Method("updateCost(...)");
    unsigned index = -1;
    if ( match(name, &index) ){
        cout << simTime() << "\t" << getFullPath() << ": Update prefix: " << name << ": " << dest.str().c_str() << endl;
        entries.at(index)->updateCost(c, a, dest);
        entries.at(index)->markNotErased();
        entries.at(index)->setExpireAt(simTime() + SimTime(entryLifetime, SIMTIME_MS));
        return entries.at(index)->getCost();
    }

    if ( entries.size() >= maxSize ){
        cout << simTime() << "\t" << getFullPath() << ": FIB full. Cannot create" << endl;
        return -1;
    }
    cout << simTime() << "\t" << getFullPath() << ": Create entry for: " << name << ": " << dest.str().c_str() << endl;
    std::string prefix(name, length);
    IlnfsEntry *e = new IlnfsEntry(prefix, face, dest, entryLifetime, c, a);
    entries.push_back(e);
    numAdded++;
    //if (! checkExpired->isScheduled())
    //    scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkExpired);
    return e->getCost();
}

bool FibIlnfs::resetCost(const char* name, float max_delta)
{
    Enter_Method("resetCost(...)");
    unsigned index = 0;
    if ( match(name, &index) ){
        cout << getFullPath() << ": Reset cost for: "<< entries.at(index)->getPrefix() << endl;
        return !entries.at(index)->resetCost(max_delta);
        //return true;
    }
    return false;
}


} //namespace
