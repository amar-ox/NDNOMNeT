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

#include "CsBase.h"

namespace inet {
using std::cout;

simsignal_t CsBase::dataStaleSignal = cComponent::registerSignal("dataStale");
Register_Abstract_Class(ICs::Notification);
Define_Module(CsBase);

CsBase::CsBase()
{
    size = 0;
    pageList = new DoublyLinkedList();
    pageMap = map<string, Node*>();
}

CsBase::~CsBase()
{
    cancelAndDelete(checkDataStale);
    /*for (unsigned int i(0); i < entries.size(); ++i){
        delete entries[i]->getData();
        entries.erase(entries.begin() + (i--));
    }*/
    map<std::string, Node*>::iterator i1;
    for(i1 = pageMap.begin(); i1 != pageMap.end(); i1++) {
        delete i1->second->value;
        delete i1->second;
    }
    delete pageList;
}

void CsBase::initialize()
{
    maxSize = par("maxSize");
    //entries.clear();
    //this->capacity = capacity;
}

void CsBase::handleMessage(cMessage *msg)
{
    /*if ( msg->isSelfMessage() ){
        cleanStaled();
        if (entries.size())
            scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkDataStale);
    }else*/
        delete msg;
}

Data* CsBase::lookup(Interest* interest)
{
    /* FIXME: looks for exact name matching (no suffixes...) */
    Enter_Method("lookup(...)");
    /*if (entries.empty())
            return nullptr;
    std::vector<CsEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        if ( !strcmp((*it)->getData()->getName(), interest->getName()) ){
            if ( interest->getMustBeFresh() ){
                if ((*it)->getFresh()){
                    cout << simTime() << "\t" << getFullPath() << ": Cache hit fresh ("<< interest->getName() << ")" << endl;
                    numHit++;
                    return (*it)->getData();
                }
                cout << simTime() << "\t" << getFullPath() << ": Cache hit stale ("<< (*it)->getData()->getName() << ")" << endl;
                numMiss++;
                return nullptr;
            }
            cout << simTime() << "\t" << getFullPath() << ": Cache hit ("<< interest->getName() << ")" << endl;
            numHit++;
            return (*it)->getData();
        }
    }
    numMiss++;
    return nullptr;*/

    //std::string key(interest->getName(),strlen(interest->getName()));
    std::string key = interest->getName();

    if(pageMap.find(key) == pageMap.end()) {
        cout << simTime() << "\t" << getFullPath() << ": Cache Miss" << endl;
        numMiss++;
        return nullptr;
    }
    numHit++;
    Data* val = pageMap[key]->value;
    cout << simTime() << "\t" << getFullPath() << ": Cache Hit (" << val->getName() << ")" << endl;
    // move the page to front
    pageList->move_page_to_head(pageMap[key]);
    return val;
}

/* LRU caching */
bool CsBase::add(Data* data)
{
    Enter_Method("add(...)");
    if (maxSize <= 0)
        return false;
    std::string key(data->getName(),strlen(data->getName()));
    //std::string key = data->getName();
    if(pageMap.find(key) != pageMap.end()) {
        // if key already present, update value and move page to head
        cout << simTime() << "\t" << getFullPath() << ": Update Data (" << key << ")" << endl;
        pageMap[key]->value = data->dup();
        pageList->move_page_to_head(pageMap[key]);
        return true;
    }

    if(size == (int)maxSize) {
        // remove rear page
        cout << simTime() << "\t" << getFullPath() << ": Remove LRU Data (" << key << ")" << endl;
        delete pageList->get_rear_page()->value;
        std::string k = pageList->get_rear_page()->key;
        pageMap.erase(k);
        pageList->remove_rear_page();
        size--;
    }

    // add new page to head to Queue
    Node *page = pageList->add_page_to_head(key, data->dup());
    cout << simTime() << "\t" << getFullPath() << ": Add Data (" << key << ")" << endl;
    size++;
    pageMap[key] = page;
    numAdded++;
    return true;

    /*if (!maxSize)
        return false;
    std::vector<CsEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        if ( !strcmp((*it)->getData()->getName(), data->getName()) ){
            if ( data->getFreshnessPeriod() != 0 ){
                cout << simTime() << "\t" << getFullPath() << ": Update Data freshness (" << data->getName() << ")" << endl;
                (*it)->setFresh(true);
                (*it)->setStaleAt(simTime() + SimTime(data->getFreshnessPeriod(), SIMTIME_MS));
            }
            if (! checkDataStale->isScheduled())
                scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkDataStale);
            numAdded++;
            return true;
        }
    }
    if (entries.size() < maxSize){
        cout << simTime() << "\t" << getFullPath() << ": Add new Data (" << data->getName() << ")" << endl;
        CsEntry *e = new CsEntry(data->dup());
        entries.push_back(e);
        if (! checkDataStale->isScheduled())
            scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkDataStale);
        numAdded++;
        return true;
    }

    for (it = entries.begin(); it != entries.end(); ++it){
        if (!(*it)->getFresh()){
            cout << simTime() << "\t" << getFullPath() << ": Replace stale Data (" << (*it)->getData()->getName() << ")" << endl;
            delete (*it)->getData();
            entries.erase(it);
            break;
        }
    }
    if (entries.size() == maxSize){
        delete entries[0]->getData();
        entries.erase(entries.begin());
    }
    cout << simTime() << "\t" << getFullPath() << ": Add new Data (" << data->getName() << ")" << endl;
    CsEntry *e = new CsEntry(data->dup());
    entries.push_back(e);

    if (! checkDataStale->isScheduled())
        scheduleAt(simTime() + SimTime(1000, SIMTIME_MS), checkDataStale);
    numAdded++;
    return true;*/
}

bool CsBase::remove(const char* name)
{
    return false;
}

void CsBase::cleanStaled()
{
    /*std::vector<CsEntry *>::iterator it;
    for (it = entries.begin(); it != entries.end(); ++it){
        if ( ((*it)->getStaleAt() > SIMTIME_ZERO) && (*it)->getStaleAt() <= simTime() ){
            numStale++;
            cout << simTime() << "\t" << getFullPath() << ": Data stale (" << (*it)->getData()->getName() << ")"<< endl;
            //Notification signal((*it)->getData());
            //emit(dataStaleSignal, &signal);
            (*it)->setFresh(false);
        }
    }*/
}

void CsBase::print()
{
}

void CsBase::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "Size: %d entries", (int)size);
    getDisplayString().setTagArg("t", 0, buf);
}

void CsBase::finish()
{
    recordScalar("added", numAdded);
    recordScalar("removed", numRemoved);
    recordScalar("hit", numHit);
    recordScalar("miss", numMiss);
    recordScalar("stale", numStale);
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    cout << getFullPath() << ": " << numAdded << " added, " << numStale << " stale, " << numHit << " hit, " << numMiss << " miss" << endl;
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
}

} //namespace
