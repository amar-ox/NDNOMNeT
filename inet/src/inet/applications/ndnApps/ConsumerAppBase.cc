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

#include "ConsumerAppBase.h"

namespace inet {
using std::cout;

Define_Module(ConsumerAppBase);

ConsumerAppBase::ConsumerAppBase()
{
}

ConsumerAppBase::~ConsumerAppBase()
{
    cancelAndDelete(expressInterestTimer);
}

void ConsumerAppBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        startTime = par("startTime");
        stopTime = par("stopTime");

        cPrefix = par("prefix").stringValue();
        numInterests = par("numInterests");
        interestReTx = par("interestReTx");
        interestLength = par("interestLength");
        interestLifetime = par("interestLifetime");
        sendIntervalPar = &par("sendInterval");

        lastRttVector.setName("RTT");

        /* Zipf */
        alpha = par("alpha");
        nContent = (unsigned) par("nContent");
        nClasses = (unsigned) par("nClasses");
        m = (unsigned) (nContent/nClasses);

        unsigned int k;
        for (k=1; k <= nClasses; k++)
            c = c + (1.0 / pow((double) k, alpha));
        c = 1.0 / c;

        double val = 0;
        for (k=1; k <= nClasses; k++){
            val+= (c / pow((double) k, alpha));
            probVec.push_back(val);
        }
        /*------*/

        /* for trace file */
        //long label = this->getSimulation()->getSystemModule()->par("label").longValue();
        //statFile+=std::to_string(label);
        statFile+=".csv";
        std::ofstream  out;
        out.open(statFile.c_str(), std::ios_base::trunc);
        //out << "time,seq,class,rtt,hops" << endl;
        out << "pktSeq,pkName,rtt,hops" << endl;
        out.close();
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (isNodeUp())
            startApp();
    }
}

void ConsumerAppBase::handleMessage(cMessage *msg)
{
    if (!isNodeUp())
        throw cRuntimeError("Application is not running");
    if (msg == expressInterestTimer){
        //cout << simTime() << "\t" << getFullPath() << ": Express Interest..." << endl;
        expressInterest(createNextInterest());
        if (numIntSent < numInterests || numInterests == -1)
            scheduleNextInterest(simTime());
    }
    else if (msg->isPacket()){
        NdnPacket* packet = check_and_cast<NdnPacket *>(msg);
        if ( packet->getKind() == TIMEOUT_CODE )
            onTimeout(check_and_cast<Interest *>(packet));
        else if ( packet->getType() == 6 )
            onData(check_and_cast<Data *>(packet));
        else
            delete packet;
    }
}

Interest* ConsumerAppBase::createNextInterest()
{
    std::string interestName(cPrefix, strlen(cPrefix));
    size_t prefixLength = strlen(cPrefix);

    interestName+="/";
    interestName+=std::to_string(pickClass());
    interestName+="/";
    interestName+=std::to_string(intuniform(1,m));
    //size_t prefixLength = interestName.length();

    /* NDN-ML */
    // interestName+="/";
    // interestName+=std::to_string(intSeqNo);
    /**********/

    Interest *interest = new Interest(interestName.c_str());

    // set simulation fields
    interest->setType(5);
    interest->setPrefixLength(prefixLength);
    interest->setKind(0);
    interest->setHopCount(0);
    interest->setSeqNo(intSeqNo);
    interest->setFlood(!intSeqNo);

    // set NDN fields
    interest->setCanBePrefix(false);
    interest->setMustBeFresh(false);
    interest->setInterestLifetimeMs(interestLifetime);
    interest->setNonceArraySize(4);
    interest->setNonce(0,intuniform(0,255));
    interest->setNonce(1,intuniform(0,255));
    interest->setNonce(2,intuniform(0,255));
    interest->setNonce(3,intuniform(0,255));

    int iLength = interestLength ? interestLength : Tools::computeTlvPacketSize(interest);
    interest->setByteLength(iLength);

    firstSendTimeHistory[intSeqNo % HISTORY_SIZE] = simTime();
    lastSendTimeHistory[intSeqNo % HISTORY_SIZE] = simTime();
    numRetxHistory[intSeqNo % HISTORY_SIZE] = 0;

    return interest;
}

void ConsumerAppBase::onData(Data *data)
{
    cout << getFullPath() << ":" << endl;
    //Tools::printPacket(data);
    simtime_t allRtt = intSeqNo - data->getSeqNo() > HISTORY_SIZE ?
            0 : simTime() - firstSendTimeHistory[data->getSeqNo() % HISTORY_SIZE];
    simtime_t lastRtt = intSeqNo - data->getSeqNo() > HISTORY_SIZE ?
            0 : simTime() - lastSendTimeHistory[data->getSeqNo() % HISTORY_SIZE];
    short numRetx  = intSeqNo - data->getSeqNo() > HISTORY_SIZE ? 0 : numRetxHistory[data->getSeqNo() % HISTORY_SIZE];

    /* for model */
    //if ( (lastRtt.dbl() >= 0.) && (lastRtt.dbl()*1000. < interestLifetime) ){

    /* for nd-csma */
    if ( (lastRtt.dbl() > 0.) && (lastRtt.dbl()*1000. < interestLifetime) ){
        allRttStat.collect(allRtt * 1000);
        lastRttStat.collect(lastRtt * 1000);
        lastRttVector.record(lastRtt * 1000);
        retxStat.collect(numRetx);
        hopCountStat.collect(data->getHopCount());
    }
    if ( lastRtt.dbl()*1000 < interestLifetime )
        writeLog(data->getSeqNo(), data->getName(), lastRtt.dbl()*1000, data->getHopCount());

    numDataReceived++;
    delete data;
}

void ConsumerAppBase::startApp()
{
    if ( numInterests == -1 || intSeqNo < numInterests ){
        cout << simTime() << "\t" << getFullPath() << ": Starting up consumer to request content: " << cPrefix << "  with: " << numInterests << " Interests" << endl;
        scheduleNextInterest(-1);
    }
}

void ConsumerAppBase::scheduleNextInterest(simtime_t previous)
{
    simtime_t next;
    if (previous == -1) {
        next = simTime() <= startTime ? startTime : simTime();
    }
    else {
        next = previous + sendIntervalPar->doubleValue();
    }
    if (stopTime < SIMTIME_ZERO || next < stopTime)
        scheduleAt(next, expressInterestTimer);
}

unsigned int ConsumerAppBase::pickClass()
{
    double r;
    do
        r = uniform(0,1);
    while ((r == 0) || (r == 1));

    for (unsigned int k=1; k <= nClasses; k++)
        if (r < probVec[k])
          return k;

    throw cRuntimeError("Error in Zipf distribution.");
    return 0;
}

void ConsumerAppBase::expressInterest(Interest* interest)
{
    send(interest, "consumerOut");
    cout << simTime() << "\t" << getFullPath() << ": >> Express Interest (" << interest->getName() << ")" << endl;
    intSeqNo++;
    numIntSent++;
}

void ConsumerAppBase::onTimeout(Interest *interest)
{
    cout << simTime() << "\t" << getFullPath() << ": << Interest Timeout (" << interest->getName() << ")" << endl;
    if (numRetxHistory[interest->getSeqNo() % HISTORY_SIZE] < interestReTx){
        lastSendTimeHistory[interest->getSeqNo() % HISTORY_SIZE] = simTime();
        numRetxHistory[interest->getSeqNo() % HISTORY_SIZE]++;
        interest->setHopCount(0);
        interest->setKind(0);
        interest->setFlood(true);
        sendDelayed(interest, SimTime(uniform(1,2), SIMTIME_MS), "consumerOut");
        cout << simTime() << "\t" << getFullPath() << ": >> ReTx Interest (" << interest->getName() << ")" << endl;
    }
    else{
        writeLog(interest->getSeqNo(), interest->getName(), -1., -1);
        numUnsatisfied++;
        delete interest;
    }
}

bool ConsumerAppBase::isNodeUp()
{
    return !nodeStatus || nodeStatus->getState() == NodeStatus::UP;
}

void ConsumerAppBase::refreshDisplay() const
{
    char buf[40];
    sprintf(buf, "prefix: %s \nrcvd: %d D\nsent: %d I", cPrefix, numDataReceived, numIntSent);
    getDisplayString().setTagArg("t", 0, buf);
}

bool ConsumerAppBase::handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback)
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

void ConsumerAppBase::finish()
{
    recordScalar("IntSent", numIntSent);
    recordScalar("DataRcvd", numDataReceived);

    cout << "---------------------------------------------------------------------------------------------------------------" << endl;
    cout << getFullPath() << ": " << numIntSent << " IntSent, " << numDataReceived << " DataRcvd" << endl;
    cout << "---------------------------------------------------------------------------------------------------------------" << endl;

    allRttStat.recordAs("allRtt");
    lastRttStat.recordAs("lastRtt");
    retxStat.recordAs("reTx");
    hopCountStat.recordAs("hopCount");
}

void ConsumerAppBase::writeLog(int seqN, const char* name, double data1, int data2)
{
    /* header: pktSeq,pkName,rtt,hops */

    /*cStringTokenizer tokenizer(name, "/");
    const char *content, *clss;
    while (tokenizer.hasMoreTokens()){
        clss = tokenizer.nextToken();
        content = tokenizer.nextToken();
    }
    std::ofstream  out;
    out.open(statFile, std::ios_base::app);
    out << simTime().inUnit(SIMTIME_S)  << "," << seqN << "," << clss << "," << data1 << "," << data2 << endl;
    out.close();*/

    std::ofstream  out;
    out.open(statFile, std::ios_base::app);
    out << seqN  << "," << name << "," << data1 << "," << data2 << endl;
    out.close();
}
} //namespace
