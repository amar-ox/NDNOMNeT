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

#include "DynamicMapping.h"

namespace inet {
using std::cout;

Define_Module(DynamicMapping);

enum NdnMacMappingCodes {
    IB_DB = 11, // Interest broadcast, Data broadcast
    IB_DU,      // Interest broadcast, Data unicast
    IU_DB,      // Interest unicast, Data broadcast
    IU_DU,      // Interest unicast, Data unicast
};

DynamicMapping::DynamicMapping()
{
}

DynamicMapping::~DynamicMapping()
{
    if (waiting)
        delete sendDelayedPacket->getDelayedPacket();
    cancelAndDelete(sendDelayedPacket);
}

void DynamicMapping::initialize(int stage)
    {
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        pit = getModuleFromPar<PitBase>(par("pitModule"), this);
        fib = getModuleFromPar<FibBase>(par("fibModule"), this);
        cs = getModuleFromPar<CsBase>(par("csModule"), this);

        cModule *pitModule = check_and_cast<cModule *>(pit);
        pitModule->subscribe(PitBase::interestTimeoutSignal, this);

        cModule *fibModule = check_and_cast<cModule *>(fib);
        fibModule->subscribe(FibBase::entryExpiredSignal, this);

        cModule *csModule = check_and_cast<cModule *>(fib);
        csModule->subscribe(CsBase::dataStaleSignal, this);

        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

        ndnMacMapping = par("ndnMacMapping");
        forwarding = par("forwarding");
        cacheUnsolicited = par("cacheUnsolicited");

        WATCH(numIntReceived);
        WATCH(numIntFwd);
        WATCH(numIntCanceled);
        WATCH(numDataCanceled);
        WATCH(numDataReceived);
        WATCH(numDataUnsolicited);
        WATCH(numDataFwd);

    }else if(stage == INITSTAGE_NETWORK_LAYER){
        for (int i = 0; i < ift->getNumInterfaces(); i++)
        {
            myMacAddress = ift->getInterface(i)->getMacAddress();
            if ( myMacAddress.equals(MACAddress("00-00-00-00-00-01")) )
                break;
        }
        cout << getFullPath() << ": MAC address: " << myMacAddress.str().c_str() << endl;
    }
}

void DynamicMapping::handleMessage(cMessage *msg)
{
    if ( msg == sendDelayedPacket ){
        SendDelayed *sd  = check_and_cast<SendDelayed *>(msg);
        if( sd->getType() == INTEREST ){
            Interest * interest = check_and_cast<Interest *>(sd->getDelayedPacket());
            forwardInterestToRemote(interest->dup(), sd->getFace(), MACAddress(sd->getMacSrc()), MACAddress(sd->getMacDest()));
            numIntFwd++;
        }
        else{
            Data *data = check_and_cast<Data *>(sd->getDelayedPacket());
            forwardDataToRemote(data->dup(), sd->getFace(), MACAddress(sd->getMacDest()));
            numDataFwd++;
        }
        delete sd->getDelayedPacket();
        waiting = false;
    }
    else if ( msg->isPacket() ){
        if ( msg->getKind() == REGISTER_PREFIX_CODE){
            cout << simTime() << "\t" << getFullPath() << ": Register prefix (" << msg->getName() << ")" << endl;
            fib->registerPrefix(msg->getName(), msg->getArrivalGate(), myMacAddress);
            delete msg;
            return;
        }
        NdnPacket* ndnPacket = check_and_cast<NdnPacket *>(msg);
        if ( waiting ){
            if ( !strcmp(ndnPacket->getName(), sendDelayedPacket->getDelayedPacket()->getName()) ){
                cout << simTime() << "\t" << getFullPath() << ": Cancel delayed transmission ("
                        << sendDelayedPacket->getDelayedPacket()->getName() << "), Type: "
                        << sendDelayedPacket->getDelayedPacket()->getType() << endl;
                cancelEvent(sendDelayedPacket);
                delete sendDelayedPacket->getDelayedPacket();
                waiting = false;
                if (ndnPacket->getType() == INTEREST)
                    numIntCanceled++;
                else
                    numDataCanceled++;
                delete ndnPacket;
            }
        }
        else
            dispatchPacket(check_and_cast<NdnPacket *>(msg));
    }else
        delete msg;
}

void DynamicMapping::dispatchPacket(NdnPacket *packet)
{
    if( packet->getArrivalGate()->isName("lowerLayerIn") ){
        IMACProtocolControlInfo *ctrl = check_and_cast<IMACProtocolControlInfo *>(packet->getControlInfo());
        packet->removeControlInfo();
        if ( packet->getType() == INTEREST ){
            processLLInterest(check_and_cast<Interest *>(packet), ctrl->getSourceAddress());
        }
        else if ( packet->getType() == DATA ){
            processLLData(check_and_cast<Data *>(packet), ctrl->getSourceAddress());
        }
    }
    else if( packet->getArrivalGate()->isName("upperLayerIn") ){
        if ( packet->getType() == INTEREST )
            processHLInterest(check_and_cast<Interest *>(packet));
        else if ( packet->getType() == DATA )
            processHLData(check_and_cast<Data *>(packet));
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": << Unexpected packet origin (" << packet->getName() << ")" << endl;
        delete packet;
    }
}

void DynamicMapping::processLLInterest(Interest *interest, MACAddress macSrc)
{
    numIntReceived++;
    cout << simTime() << "\t" << getFullPath() << ": << Interest from LL (" << interest->getName() << ")" << endl;
    Data* cachedData = cs->lookup(interest);
    if ( cachedData != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": << Cached Data found (" << interest->getName() << ")" << endl;
        forwardDataToRemote(cachedData->dup(), interest->getArrivalGate()->getIndex(), macSrc);
        delete interest;
        return;
    }
    if( pit->lookup(interest->getName()) == nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": New Interest" << endl;
        interest->setHopCount(interest->getHopCount() + 1);
        BaseEntry *fe = fib->lookup(interest);
        if ( fe != nullptr ){
            if ( fe->getFace()->isName("lowerLayerIn") && forwarding ){
                if ( fe->getMacDest().isBroadcast() ){
                    interest->setFlood(true);
                    cout << simTime() << "\t" << getFullPath() << ": Broadcast" << endl;
                    if( waiting )
                        delete sendDelayedPacket->getDelayedPacket();
                    sendDelayedPacket->setDelayedPacket(interest);
                    sendDelayedPacket->setType(INTEREST);
                    sendDelayedPacket->setFace(fe->getFace()->getIndex());
                    sendDelayedPacket->setMacDest("ff:ff:ff:ff:ff:ff");
                    sendDelayedPacket->setMacSrc(macSrc.str().c_str());
                    scheduleAt(simTime() + SimTime(computeDataRandomDelay(),SIMTIME_MS), sendDelayedPacket);
                    waiting = true;
                }
                else{
                    forwardInterestToRemote(interest, fe->getFace()->getIndex(), macSrc, fe->getMacDest());
                    numIntFwd++;
                }
            }
            else if (fe->getFace()->isName("upperLayerIn") ){
                forwardInterestToLocal(interest, fe->getFace()->getIndex(), macSrc);
            }
            else
                delete interest;
        }
        else{
            if ( interest->getFlood() && forwarding ){
                cout << simTime() << "\t" << getFullPath() << ": Flood" << endl;
                if( waiting )
                    delete sendDelayedPacket->getDelayedPacket();
                sendDelayedPacket->setDelayedPacket(interest);
                sendDelayedPacket->setType(INTEREST);
                sendDelayedPacket->setFace(DEFAULT_MAC_IF);
                sendDelayedPacket->setMacDest("ff:ff:ff:ff:ff:ff");
                sendDelayedPacket->setMacSrc(macSrc.str().c_str());
                scheduleAt(simTime() + SimTime(computeDataRandomDelay(),SIMTIME_MS), sendDelayedPacket);
                waiting = true;
            }
            else{
                cout << simTime() << "\t" << getFullPath() << ": No forwarding (no Flood flag)" << endl;
                delete interest;
            }
        }
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": Interest already in PIT" << endl;
        numIntDuplicated++;
        delete interest;
    }
}

void DynamicMapping::processLLData(Data *data, MACAddress macSrc)
{
    cout << simTime() << "\t" << getFullPath() << ": << Data from LL (" << data->getName() << ")" << endl;
    IPit::PitEntry *pe = pit->lookup(data->getName());
    if ( pe != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": Solicited Data" << endl;
        numDataReceived++;
        fib->create(data->getName(), data->getPrefixLength(), data->getArrivalGate(), macSrc);
        if ( pe->getFace()->isName("lowerLayerIn") && forwarding ){
            cout << simTime() << "\t" << getFullPath() << ": Delay forward Data through NetDeviceFace" << endl;
            data->setHopCount(data->getHopCount()+1);
            forwardDataToRemote(data, pe->getFace()->getIndex(), pe->getMacSrc());
            numDataFwd++;
        }
        else if ( pe->getFace()->isName("upperLayerIn") ){
            data->setHopCount(data->getHopCount()+1);
            forwardDataToLocal(data, pe->getFace()->getIndex());
        }
        pit->remove(data->getName());
        cs->add(data);
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": Unsolicited Data" << endl;
        //if ( ndnMacMapping == IU_DB )
        //    fib->create(data->getName(), data->getPrefixLength(), data->getArrivalGate(), macSrc);
        if (cacheUnsolicited)
            cs->add(data);
        delete data;
        numDataUnsolicited++;
    }
}

void DynamicMapping::processHLInterest(Interest *interest)
{
    cout << simTime() << "\t" << getFullPath() << ": << HL Interest (" << interest->getName() << ")" << endl;
    Data* cachedData = cs->lookup(interest);
    if ( cachedData != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": << Cached Data found (" << interest->getName() << ")" << endl;
        forwardDataToLocal(cachedData->dup(), interest->getArrivalGate()->getIndex());
        delete interest;
        return;
    }
    if( pit->lookup(interest->getName()) == nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": New Interest" << endl;
        BaseEntry *fe = fib->lookup(interest);
        if ( fe != nullptr ){
            if ( fe->getFace()->isName("lowerLayerIn") ){
                interest->setFlood(fe->getMacDest().isBroadcast());
                forwardInterestToRemote(interest, fe->getFace()->getIndex(), myMacAddress, fe->getMacDest());
            }
            else if ( fe->getFace()->isName("upperLayerIn") ){
                forwardInterestToLocal(interest, fe->getFace()->getIndex(), myMacAddress);
            }
        }
        else{
            interest->setFlood(true);
            forwardInterestToRemote(interest, DEFAULT_MAC_IF, myMacAddress, MACAddress("ff:ff:ff:ff:ff:ff"));
        }
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": Interest already in PIT" << endl;
        numIntDuplicated++;
        delete interest;
    }
}

void DynamicMapping::processHLData(Data *data)
{
    IPit::PitEntry *pe = pit->lookup(data->getName());
    cout << simTime() << "\t" << getFullPath() << ": << Data from UL (" << data->getName() << ")" << endl;
    if ( pe != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": Solicited Data from AppFace " << data->getArrivalGate()->getIndex() << endl;
        if ( pe->getFace()->isName("lowerLayerIn") ){
            forwardDataToRemote(data, pe->getFace()->getIndex(), pe->getMacSrc());
        }
        else if ( pe->getFace()->isName("upperLayerIn") ){
            forwardDataToLocal(data, pe->getFace()->getIndex());
        }
        pit->remove(data->getName());
        cs->add(data);
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": Unsolicited Data" << endl;
        if (cacheUnsolicited)
            cs->add(data);
        delete data;
    }
}

void DynamicMapping::forwardInterestToRemote(Interest* interest, int face, MACAddress macSrc, MACAddress macDest)
{
    if ( pit->create(interest, macSrc) ){
        cout << simTime() << "\t" << getFullPath() << ": Send Interest through NetDeviceFace (" << interest->getName() << ")" << endl;
        ndnToMacMapping(interest, macDest);
        send(interest, "lowerLayerOut", face);
    }else
        delete interest;
}

void DynamicMapping::forwardDataToRemote(Data* data, int face, MACAddress macDest)
{
    cout << simTime() << "\t" << getFullPath() << ": Send Data through NetDeviceFace (" << data->getName() << ")" << endl;
    ndnToMacMapping(data, macDest);
    send(data, "lowerLayerOut", face);
}


void DynamicMapping::forwardInterestToLocal(Interest* interest, int face, MACAddress macSrc)
{
    if ( pit->create(interest, macSrc) ){
        cout << simTime() << "\t" << getFullPath() << ": Send Interest to AppFace (" << interest->getName() << ")" << endl;
        send(interest, "upperLayerOut", face);
    }
    else
        delete interest;
}

void DynamicMapping::forwardDataToLocal(Data* data, int face)
{
    cout << simTime() << "\t" << getFullPath() << ": Send Data to AppFace (" << data->getName() << ")" << endl;
    send(data, "upperLayerOut", face);
}


void DynamicMapping::ndnToMacMapping(NdnPacket *ndnPacket, MACAddress macDest)
{
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    //IMACProtocolControlInfo *controlInfo = new SimpleLinkLayerControlInfo();
    controlInfo->setSourceAddress(myMacAddress);

    if( ndnPacket->getType() == INTEREST ){
        if( ndnMacMapping == IB_DB || ndnMacMapping == IB_DU )
            controlInfo->setDestinationAddress(MACAddress("ff:ff:ff:ff:ff:ff"));
        else
            controlInfo->setDestinationAddress(macDest);
    }
    else if( ndnPacket->getType() == DATA ){
        if( ndnMacMapping == IB_DB || ndnMacMapping == IU_DB )
            controlInfo->setDestinationAddress(MACAddress("ff:ff:ff:ff:ff:ff"));
        else
            controlInfo->setDestinationAddress(macDest);
    }
    ndnPacket->setControlInfo(check_and_cast<cObject *>(controlInfo));
}

double DynamicMapping::computeInterestRandomDelay()
{
    return (DW + uniform(0,DW)) * DEFER_SLOT_TIME;
}

double DynamicMapping::computeDataRandomDelay()
{
    return uniform(0,DW-1) * DEFER_SLOT_TIME;
}

void DynamicMapping::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "rcvd: %d D\nfwd: %d I", numDataReceived, numIntFwd);
    getDisplayString().setTagArg("t", 0, buf);
}

void DynamicMapping::finish()
{
    recordScalar("numIntRcvd", numIntReceived);
    recordScalar("numIntFwd", numIntFwd);
    recordScalar("numIntCanceled", numIntCanceled);
    recordScalar("numIntDuplicated", numIntDuplicated);
    recordScalar("numDataReceived", numDataReceived);
    recordScalar("numDataFwd", numDataFwd);
    recordScalar("numDataCanceled", numDataCanceled);
    recordScalar("numDataUnsolicited", numDataUnsolicited);
}

void DynamicMapping::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == PitBase::interestTimeoutSignal) {
        IPit::Notification* notification = check_and_cast<IPit::Notification *>(obj);
        cGate* g = notification->face;
        if (g->isName("upperLayerIn")){
            Interest* interest = notification->interest->dup();
            interest->setKind(TIMEOUT_CODE);
            cout << simTime() << "\t" << getFullPath() << ": Send timeout. Face: " << g->getIndex() << " | Interest: " << interest->getName() << endl;
            send(interest, "upperLayerOut", g->getIndex());
        }
        fib->setToBroadcast(notification->interest->getName());
    }
    else if (signalID == FibBase::entryExpiredSignal) {
        IFib::Notification* notification = check_and_cast<IFib::Notification *>(obj);
        cout << simTime() << "\t" << getFullPath() << ": Expired prefix (" << notification->prefix << ")" << endl;
        //fib->setToBroadcast(notification->prefix);
    }
}


void DynamicMapping::checkPrefix(const char* prefix)
{
    Interest *interest = new Interest(prefix);
    interest->setType(5);
    interest->setKind(CHECK_PREFIX_CODE);
    //...
}

bool DynamicMapping::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    return true;
}

} // namespace inet

