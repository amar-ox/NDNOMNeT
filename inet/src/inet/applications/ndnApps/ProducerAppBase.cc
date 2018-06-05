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

#include "ProducerAppBase.h"

namespace inet {
using std::cout;

Define_Module(ProducerAppBase);

ProducerAppBase::ProducerAppBase()
{
}

ProducerAppBase::~ProducerAppBase()
{
}

void ProducerAppBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        startTime = par("startTime");
        stopTime = par("stopTime");
        pPrefix = par("prefix").stringValue();
        dataLength = par("dataLength");
        dataFreshness = par("dataFreshness");

        WATCH(numIntReceived);
        WATCH(numDataSent);
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if ( ! strncmp(pPrefix, "/", 1) )
            registerPrefix();
    }
}

void ProducerAppBase::registerPrefix()
{
    Interest* regPrefix = new Interest(pPrefix);
    regPrefix->setKind(REGISTER_PREFIX);
    send(regPrefix,"producerOut");
}

void ProducerAppBase::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    else if (msg->isPacket())
        processInterest(check_and_cast<NdnPacket *>(msg));
}

void ProducerAppBase::processInterest(NdnPacket *packet)
{
   if ( packet->getType() == 5 ){
       Interest* interest = check_and_cast<Interest *>(packet);
       numIntReceived++;

       cout << getFullPath() << ":" << endl;
       Tools::printPacket(interest);

       if ( (simTime() >= startTime) && (stopTime < SIMTIME_ZERO || simTime() < stopTime) ){
           Data *data = new Data(interest->getName());
           data->setSeqNo(interest->getSeqNo());
           data->setPrefixLength(strlen(pPrefix));
           data->setType(6);
           data->setHopCount(0);
           data->setFreshnessPeriod(dataFreshness);
           data->setByteLength(Tools::computeTlvPacketSize(data));
           sendDelayed(data, SimTime(1.5, SIMTIME_MS), "producerOut");
           cout << simTime() << "\t" << getFullPath() << ": >> Data (" << data->getName() << ")" << endl;
           numDataSent++;
       }else
           cout << simTime() << "\t" << getFullPath() << ": Producer is down" << endl;
   }else
       cout << simTime() << "\t" << getFullPath() << ": Received non Interest packet" << endl;
   delete packet;
}

bool ProducerAppBase::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void ProducerAppBase::refreshDisplay() const
{
    char buf[50];
    sprintf(buf, "prefix: %s \nrcvd: %d I\nsent: %d D", pPrefix, numIntReceived, numDataSent);
    getDisplayString().setTagArg("t", 0, buf);
}

bool ProducerAppBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation)) {
        if ((NodeStartOperation::Stage)stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            return true;
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if ((NodeShutdownOperation::Stage)stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            return true;
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if ((NodeCrashOperation::Stage)stage == NodeCrashOperation::STAGE_CRASH)
            return true;
    }
    else
        throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}

void ProducerAppBase::finish()
{
    recordScalar("IntRvcd", numIntReceived);
    recordScalar("DataSent", numDataSent);

    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    cout << getFullPath() << ": " << numIntReceived << " IntRcvd, " << numDataSent << " DataSent, Prefix: " << pPrefix << endl;
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
}

} //namespace
