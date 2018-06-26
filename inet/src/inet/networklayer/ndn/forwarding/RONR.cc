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

#include "RONR.h"

namespace inet {
using std::cout;

Define_Module(RONR);

enum NdnMacMappingCodes {
    IB_DB = 11, // Interest broadcast, Data broadcast
    IB_DU,      // Interest broadcast, Data unicast
    IU_DB,      // Interest unicast, Data broadcast
    IU_DU,      // Interest unicast, Data unicast
};

RONR::RONR()
{
}

RONR::~RONR()
{
}

void RONR::initialize(int stage)
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

void RONR::handleMessage(cMessage *msg)
{
    if ( msg->isPacket() ){
        if ( msg->getKind() == REGISTER_PREFIX_CODE){
            cout << simTime() << "\t" << getFullPath() << ": Register prefix (" << msg->getName() << ")" << endl;
            fib->registerPrefix(msg->getName(), msg->getArrivalGate(), myMacAddress);
            delete msg;
            return;
        }
        dispatchPacket(check_and_cast<NdnPacket *>(msg));
    }else
        delete msg;
}

void RONR::dispatchPacket(NdnPacket *packet)
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

void RONR::processLLInterest(Interest *interest, MACAddress macSrc)
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
                forwardInterestToRemote(interest, fe->getFace()->getIndex(), macSrc, fe->getMacDest());
                numIntFwd++;
            }
            else if ( fe->getFace()->isName("upperLayerIn") ){
                forwardInterestToLocal(interest, fe->getFace()->getIndex(), macSrc);
            }
            else
                delete interest;
        }
        else{
            if ( interest->getFlood() && forwarding ){
                cout << simTime() << "\t" << getFullPath() << ": Delayed forwarding (flooding flag)" << endl;
                forwardInterestToRemote(interest, DEFAULT_MAC_IF, macSrc, MACAddress("ff:ff:ff:ff:ff:ff"));
                numIntFwd++;
            }
            else{
                cout << simTime() << "\t" << getFullPath() << ": No forwarding (no flooding flag)" << endl;
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

void RONR::processLLData(Data *data, MACAddress macSrc)
{
    cout << simTime() << "\t" << getFullPath() << ": << Data from LL (" << data->getName() << ")" << endl;
    IPit::PitEntry *pe = pit->lookup(data->getName());
    if ( pe != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": Solicited Data" << endl;
        data->setHopCount(data->getHopCount()+1);
        numDataReceived++;
        fib->create(data->getName(), data->getPrefixLength(), data->getArrivalGate(), macSrc);
        if ( pe->getFace()->isName("lowerLayerIn") && forwarding ){
            cout << simTime() << "\t" << getFullPath() << ": Delay forward Data through NetDeviceFace" << endl;
            forwardDataToRemote(data, pe->getFace()->getIndex(), pe->getMacSrc());
            numDataFwd++;
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
        numDataUnsolicited++;
    }
}

void RONR::processHLInterest(Interest *interest)
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
                forwardInterestToRemote(interest, fe->getFace()->getIndex(), myMacAddress, fe->getMacDest());
            }
            else if ( fe->getFace()->isName("upperLayerIn") ){
                forwardInterestToLocal(interest, fe->getFace()->getIndex(), myMacAddress);
            }
        }
        else{
            forwardInterestToRemote(interest, DEFAULT_MAC_IF, myMacAddress, MACAddress("ff:ff:ff:ff:ff:ff"));
        }
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": Interest already in PIT" << endl;
        numIntDuplicated++;
        delete interest;
    }
}

void RONR::processHLData(Data *data)
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

void RONR::forwardInterestToRemote(Interest* interest, int face, MACAddress macSrc, MACAddress macDest)
{
    if ( pit->create(interest, macSrc) ){
        cout << simTime() << "\t" << getFullPath() << ": Send Interest through NetDeviceFace (" << interest->getName() << ")" << endl;
        ndnToMacMapping(interest, macDest);
        send(interest, "lowerLayerOut", face);
    }else
        delete interest;
}

void RONR::forwardDataToRemote(Data* data, int face, MACAddress macDest)
{
    cout << simTime() << "\t" << getFullPath() << ": Send Data through NetDeviceFace (" << data->getName() << ")" << endl;
    ndnToMacMapping(data, macDest);
    send(data, "lowerLayerOut", face);
}


void RONR::forwardInterestToLocal(Interest* interest, int face, MACAddress macSrc)
{
    if ( pit->create(interest, macSrc) ){
        cout << simTime() << "\t" << getFullPath() << ": Send Interest to AppFace (" << interest->getName() << ")" << endl;
        send(interest, "upperLayerOut", face);
    }
    else
        delete interest;
}

void RONR::forwardDataToLocal(Data* data, int face)
{
    cout << simTime() << "\t" << getFullPath() << ": Send Data to AppFace (" << data->getName() << ")" << endl;
    send(data, "upperLayerOut", face);
}


void RONR::ndnToMacMapping(NdnPacket *ndnPacket, MACAddress macDest)
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

void RONR::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "rcvd: %d D\nfwd: %d I", numDataReceived, numIntFwd);
    getDisplayString().setTagArg("t", 0, buf);
}

void RONR::finish()
{
    recordScalar("numIntRcvd", numIntReceived);
    recordScalar("numIntFwd", numIntFwd);
    recordScalar("numIntDuplicated", numIntDuplicated);
    recordScalar("numDataReceived", numDataReceived);
    recordScalar("numDataFwd", numDataFwd);
    recordScalar("numDataUnsolicited", numDataUnsolicited);
}

void RONR::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
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
    }
    else if (signalID == FibBase::entryExpiredSignal) {
        IFib::Notification* notification = check_and_cast<IFib::Notification *>(obj);
        cout << simTime() << "\t" << getFullPath() << ": Expired prefix (" << notification->prefix << ")" << endl;
    }
}

bool RONR::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    return true;
}

} // namespace inet

