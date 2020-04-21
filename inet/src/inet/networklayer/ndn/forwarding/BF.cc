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

#include "BF.h"

namespace inet {
using std::cout;

Define_Module(BF);


BF::BF()
{
}

BF::~BF()
{
    if (waiting)
        delete sendDelayedPacket->getDelayedPacket();
    cancelAndDelete(sendDelayedPacket);
}

void BF::initialize(int stage)
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

        cModule *csModule = check_and_cast<cModule *>(cs);
        csModule->subscribe(CsBase::dataStaleSignal, this);

        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        forwarding = par("forwarding");
        cacheUnsolicited = par("cacheUnsolicited");
        delays = par("delays");

        /* for Blind Flooding */
        dw = par("dw");
        deferSlotTime = par("deferSlotTime");

        /* for binary tree */
        treeDepth = par("treeDepth");
        fatherId = par("fatherId");
        myLevel = par("myLevel");

        /* for trace file */
        long label = this->getSimulation()->getSystemModule()->par("label").longValue();
        statFile+=std::to_string(label);
        statFile+=".csv";
        std::ofstream  out;
        out.open(statFile.c_str(), std::ios_base::trunc);
        out << "time,seq,class,content,type" << endl;
        out.close();

        //naStat.setName("Na");
        fsrHist.setName("FSR");
        fsrVect.setName("FSR");

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

void BF::handleMessage(cMessage *msg)
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
        int hop = ndnPacket->getHopCount();

        /* for binary tree */
        if (ndnPacket->getArrivalGate()->isName("lowerLayerIn")){
            if (ndnPacket->getType() == INTEREST){
                neighborI++;
                if(hop >= myLevel){
                    if( ((int) ndnPacket->getCost() == fatherId) && waiting ){
                        cout << simTime() << "\t" << getFullPath() << ": Got Interest from my brother while waiting..." << endl;
                        if ( !strcmp(ndnPacket->getName(), sendDelayedPacket->getDelayedPacket()->getName()) ){
                            cout << simTime() << "\t" << getFullPath() << ": Cancel delayed transmission ("
                                    << sendDelayedPacket->getDelayedPacket()->getName() << "), Type: "
                                    << sendDelayedPacket->getDelayedPacket()->getType() << endl;
                            cancelEvent(sendDelayedPacket);
                            delete sendDelayedPacket->getDelayedPacket();
                            waiting = false;
                            numIntCanceled++;
                        }
                    }
                    cout << simTime() << "\t" << getFullPath() << ": Ignore Interest." << endl;
                    delete ndnPacket;
                }
                else{
                    cout << simTime() << "\t" << getFullPath() << ": Got Interest from above. Process it." << endl;
                    dispatchPacket(ndnPacket);
                }
            }
            else{   // Data
                neighborD++;
                if( (treeDepth-hop) > myLevel ){
                    if((int)ndnPacket->getCost() == fatherId){
                        if (waiting){  // TODO: see if there are ignored packets
                            cout << simTime() << "\t" << getFullPath() << ": Got Data from my brother while waiting..." << endl;
                            if ( !strcmp(ndnPacket->getName(), sendDelayedPacket->getDelayedPacket()->getName()) ){
                                cout << simTime() << "\t" << getFullPath() << ": Cancel delayed transmission ("
                                        << sendDelayedPacket->getDelayedPacket()->getName() << "), Type: "
                                        << sendDelayedPacket->getDelayedPacket()->getType() << endl;
                                cancelEvent(sendDelayedPacket);
                                delete sendDelayedPacket->getDelayedPacket();
                                waiting = false;
                                numDataCanceled++;
                            }
                        }
                        cout << simTime() << "\t" << getFullPath() << ": Ignore Data." << endl;
                        delete ndnPacket;
                    }else{
                        cout << simTime() << "\t" << getFullPath() << ": Got Data from below. Process it." << endl;
                        dispatchPacket(ndnPacket);
                    }
                }else
                    delete ndnPacket;
            }
        }else
            dispatchPacket(ndnPacket);

            /* if ( waiting ){
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
                }
                delete ndnPacket;
            }
            else
                dispatchPacket(ndnPacket); */

    }else
        delete msg;
}

void BF::dispatchPacket(NdnPacket *packet)
{
    if( packet->getArrivalGate()->isName("lowerLayerIn") ){
        //Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(packet->getControlInfo());
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

void BF::processLLInterest(Interest *interest, MACAddress macSrc)
{
    numIntReceived++;
    cout << simTime() << "\t" << getFullPath() << ": << Interest from LL (" << interest->getName() << ")" << endl;
    Data* cachedData = cs->lookup(interest);
    if ( cachedData != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": << Cached Data found (" << interest->getName() << ")" << endl;
        Data* data = cachedData->dup();
        /* if not binary tree: data->setHopCount(0); */
        data->setHopCount(0);
        data->setSeqNo(interest->getSeqNo());
        data->setPriority(0);

        if (delays){
            if( waiting )
                delete sendDelayedPacket->getDelayedPacket();
            sendDelayedPacket->setDelayedPacket(data);
            sendDelayedPacket->setType(DATA);
            sendDelayedPacket->setFace(interest->getArrivalGate()->getIndex());
            sendDelayedPacket->setMacDest("ff:ff:ff:ff:ff:ff");
            sendDelayedPacket->setMacSrc(macSrc.str().c_str());
            if (sendDelayedPacket->isScheduled())
                cancelEvent(sendDelayedPacket);
            scheduleAt(simTime() + SimTime(computeDataRandomDelay(),SIMTIME_MS), sendDelayedPacket);
            waiting = true;
            delete interest;
            return;
        }
        else{
            forwardDataToRemote(data, interest->getArrivalGate()->getIndex(), MACAddress("ff:ff:ff:ff:ff:ff"));
            delete interest;
            return;
        }
    }
    if( pit->lookup(interest->getName()) == nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": New Interest" << endl;
        BaseEntry *fe = (BaseEntry*) fib->lookup(interest);
        if ( fe != nullptr ){
            /* is producer */
            if ( fe->getFace() ){
                interest->setHopCount(interest->getHopCount() + 1);
                // fsrHist.collect(fib->getFSR(interest->getName()));
                // fsrVect.record(fib->getFSR(interest->getName()));
                forwardInterestToLocal(interest, fe->getFace()->getIndex(), macSrc);
                // fib->incNumIntFwd(interest->getName(), interest->getPrefixLength());
                return;
            }
        }
        if ( forwarding ){
            cout << simTime() << "\t" << getFullPath() << ": Blind flooding" << endl;
            interest->setHopCount(interest->getHopCount() + 1);
            interest->setPriority(1);
            if (delays){
                if( waiting )
                    delete sendDelayedPacket->getDelayedPacket();
                sendDelayedPacket->setDelayedPacket(interest);
                sendDelayedPacket->setType(INTEREST);
                sendDelayedPacket->setFace(DEFAULT_MAC_IF);
                sendDelayedPacket->setMacDest("ff:ff:ff:ff:ff:ff");
                sendDelayedPacket->setMacSrc(macSrc.str().c_str());
                if (sendDelayedPacket->isScheduled())
                    cancelEvent(sendDelayedPacket);
                scheduleAt(simTime() + SimTime(computeInterestRandomDelay(),SIMTIME_MS), sendDelayedPacket);
                waiting = true;
            }
            else{
                // interest->setNa(fib->getFSR(interest->getName())); // set FSR
                forwardInterestToRemote(interest, DEFAULT_MAC_IF, macSrc, MACAddress("ff:ff:ff:ff:ff:ff"));
                // fsrHist.collect(fib->getFSR(interest->getName()));
                // fsrVect.record(fib->getFSR(interest->getName()));
                // fib->incNumIntFwd(interest->getName(), interest->getPrefixLength());
                numIntFwd++;
            }
        }
        else
            delete interest;
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": Interest already in PIT" << endl;
        numIntDuplicated++;
        delete interest;
    }
}

void BF::processLLData(Data *data, MACAddress macSrc)
{
    cout << simTime() << "\t" << getFullPath() << ": << Data from LL (" << data->getName() << ")" << endl;
    data->setHopCount(data->getHopCount()+1);
    IPit::PitEntry *pe = pit->lookup(data->getName());
    if ( pe != nullptr ){
        //int seqNo = pe->getInterest()->getSeqNo();
        cout << simTime() << "\t" << getFullPath() << ": Solicited Data" << endl;
        numDataReceived++;
        if ( pe->getFace()->isName("lowerLayerIn") && forwarding ){
            cout << simTime() << "\t" << getFullPath() << ": Forward Data through NetDeviceFace" << endl;
            data->setPriority(0);

            if (delays){
                if( waiting )
                    delete sendDelayedPacket->getDelayedPacket();
                sendDelayedPacket->setDelayedPacket(data);
                sendDelayedPacket->setType(DATA);
                sendDelayedPacket->setFace(pe->getFace()->getIndex());
                sendDelayedPacket->setMacDest(pe->getMacSrc().str().c_str());
                if (sendDelayedPacket->isScheduled())
                    cancelEvent(sendDelayedPacket);
                scheduleAt(simTime() + SimTime(computeDataRandomDelay(),SIMTIME_MS), sendDelayedPacket);
                waiting = true;
            }
            else{
                forwardDataToRemote(data, pe->getFace()->getIndex(), pe->getMacSrc());
                numDataFwd++;
            }
        }
        else if ( pe->getFace()->isName("upperLayerIn") ){
            //data->setSeqNo(seqNo);
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

void BF::processHLInterest(Interest *interest)
{
    cout << simTime() << "\t" << getFullPath() << ": << HL Interest (" << interest->getName() << ")" << endl;
    Data* cachedData = cs->lookup(interest);
    if ( cachedData != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": << Cached Data found (" << interest->getName() << ")" << endl;
        Data* data = cachedData->dup();
        // if not binary tree: data->setHopCount(0);
        forwardDataToLocal(data, interest->getArrivalGate()->getIndex());
        delete interest;
        return;
    }
    if( pit->lookup(interest->getName()) == nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": New Interest" << endl;
        //BaseEntry *fe = fib->lookup(interest);
        //if ( fe != nullptr ){
        //    if ( fe->getFace()->isName("upperLayerIn") ){
                //forwardInterestToLocal(interest, fe->getFace()->getIndex(), myMacAddress);
              //}
        //}

        /* NDN-ML */
        interest->setPriority(0);
        // interest->setNa(-1);
        interest->setHopCount(0);
        forwardInterestToRemote(interest, DEFAULT_MAC_IF, myMacAddress, MACAddress("ff:ff:ff:ff:ff:ff"));
        /**********/
    }
    else{
        cout << simTime() << "\t" << getFullPath() << ": Interest already in PIT" << endl;
        numIntDuplicated++;
        delete interest;
    }
}

void BF::processHLData(Data *data)
{
    IPit::PitEntry *pe = pit->lookup(data->getName());
    cout << simTime() << "\t" << getFullPath() << ": << Data from UL (" << data->getName() << ")" << endl;
    if ( pe != nullptr ){
        cout << simTime() << "\t" << getFullPath() << ": Solicited Data from AppFace " << data->getArrivalGate()->getIndex() << endl;
        data->setHopCount(0);
        if ( pe->getFace()->isName("lowerLayerIn") ){
            data->setPriority(0);
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

void BF::forwardInterestToRemote(Interest* interest, int face, MACAddress macSrc, MACAddress macDest)
{
    if ( pit->create(interest, macSrc) ){
        cout << simTime() << "\t" << getFullPath() << ": Send Interest through NetDeviceFace (" << interest->getName() << ")" << endl;
        ndnToMacMapping(interest, macDest);
        /* if binary tree interest->setCost(fatherId); */
        send(interest, "lowerLayerOut", face);
        // writeLog(interest->getSeqNo(), interest->getName(), interest->getType());
    }
}

void BF::forwardDataToRemote(Data* data, int face, MACAddress macDest)
{
    cout << simTime() << "\t" << getFullPath() << ": Send Data through NetDeviceFace (" << data->getName() << ")" << endl;
    ndnToMacMapping(data, macDest);
    /* if binary tree data->setCost(fatherId); */

    // data->setNa(fib->getFSR(data->getName()));
    send(data, "lowerLayerOut", face);

    // fsrHist.collect(fib->getFSR(data->getName()));
    // fsrVect.record(fib->getFSR(data->getName()));
    // fib->incNumDataFwd(data->getName());
    // writeLog(data->getSeqNo(), data->getName(), data->getType());
}


void BF::forwardInterestToLocal(Interest* interest, int face, MACAddress macSrc)
{
    if ( pit->create(interest, macSrc) ){
        cout << simTime() << "\t" << getFullPath() << ": Send Interest to AppFace (" << interest->getName() << ")" << endl;
        /* if binary tree interest->setCost(fatherId); */
        send(interest, "upperLayerOut", face);
    }
}

void BF::forwardDataToLocal(Data* data, int face)
{
    cout << simTime() << "\t" << getFullPath() << ": Send Data to AppFace (" << data->getName() << ")" << endl;
    /* if binary tree data->setCost(fatherId); */
    send(data, "upperLayerOut", face);
}


void BF::ndnToMacMapping(NdnPacket *ndnPacket, MACAddress macDest)
{
    Ieee802Ctrl *controlInfo = new Ieee802Ctrl();
    //IMACProtocolControlInfo *controlInfo = new SimpleLinkLayerControlInfo();
    controlInfo->setSourceAddress(myMacAddress);
    controlInfo->setDestinationAddress(MACAddress("ff:ff:ff:ff:ff:ff"));
    ndnPacket->setControlInfo(check_and_cast<cObject *>(controlInfo));
}

double BF::computeInterestRandomDelay()
{
    return (dw + intuniform(0,dw)) * deferSlotTime;
}

double BF::computeDataRandomDelay()
{
    return intuniform(0,dw-1) * deferSlotTime;
}

float BF::computeNa()
{
    float Na;
    int unsolicitedData = neighborD - numDataFwd;
    int droppedI = neighborI - numIntFwd;
    if (droppedI == 0)
        Na = 0;
    else
        Na = (float) unsolicitedData / (float) droppedI;
    //naStat.record(Na);
    return Na;
}

void BF::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "rcvd: %d D\nfwd: %d I", numDataReceived, numIntFwd);
    getDisplayString().setTagArg("t", 0, buf);
}

void BF::finish()
{
    recordScalar("lsr:mean", fsrHist.getMean());
    recordScalar("lsr:min", fsrHist.getMin());
    recordScalar("lsr:max", fsrHist.getMax());
    recordScalar("lsr:stddev", fsrHist.getStddev());

    recordScalar("numIntRcvd", numIntReceived);
    recordScalar("numIntFwd", numIntFwd);
    recordScalar("numIntCanceled", numIntCanceled);
    recordScalar("numIntDuplicated", numIntDuplicated);
    recordScalar("numDataReceived", numDataReceived);
    recordScalar("numDataFwd", numDataFwd);
    recordScalar("numDataCanceled", numDataCanceled);
    recordScalar("numDataUnsolicited", numDataUnsolicited);

    //fsrHist.recordAs("fsr");
}

void BF::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
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

bool BF::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    return true;
}

void BF::writeLog(int seqN, const char* name, short type)
{
    cStringTokenizer tokenizer(name, "/");
    const char *content, *clss;
    while (tokenizer.hasMoreTokens()){
        clss = tokenizer.nextToken();
        content = tokenizer.nextToken();
    }

    std::ofstream  out;
    out.open(statFile, std::ios_base::app);
    out << simTime().inUnit(SIMTIME_S) << "," << seqN << "," << clss << "," << content << "," << type << endl;
    out.close();
}

} // namespace inet

