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

#ifndef __INET_CONSUMER_APP_BASE_H_
#define __INET_CONSUMER_App_BASE_H_

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#include "inet/common/INETDefs.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/common/lifecycle/NodeOperations.h"
#include "inet/common/ModuleAccess.h"

#include "inet/networklayer/ndn/packets/NdnPackets_m.h"
#include "inet/networklayer/ndn/packets/Tools.h"

#define HISTORY_SIZE 100
#define TIMEOUT_CODE 100

using namespace omnetpp;
namespace inet {

class ConsumerAppBase : public cSimpleModule, public ILifecycle
{
protected:
    // state
    NodeStatus *nodeStatus = nullptr;
    bool isOperational = false;

    /* Params */
    const char *cPrefix;
    int interestLength;
    cPar *sendIntervalPar = nullptr;
    int numInterests;
    int interestReTx;
    int interestLifetime;
    simtime_t startTime;
    simtime_t stopTime;

    /* self messages */
    cMessage *expressInterestTimer = new cMessage("expressInterest");

    /* consumer stat */
    simtime_t firstSendTimeHistory[HISTORY_SIZE];
    simtime_t lastSendTimeHistory[HISTORY_SIZE];
    short numRetxHistory[HISTORY_SIZE];
    short hopCountHistory[HISTORY_SIZE];
    cLongHistogram allRttStat;
    cLongHistogram lastRttStat;
    cLongHistogram retxStat;
    cLongHistogram hopCountStat;
    cOutVector lastRttVector;
    int numIntSent = 0;
    int numDataReceived = 0;
    int numUnsatisfied = 0;
    int intSeqNo = 0;

    std::string statFile = "/home/amar/pandas/data/BF-ML/statCons";

    /* Zipf */
    double c = 0;                  // Normalization constant
    double alpha;
    unsigned int nContent;         // Total nbr of content
    unsigned int nClasses;         // Number of classes
    unsigned int m;                // Content nbr per class
    std::vector<double> probVec;   // Cumulative probabilities
    virtual unsigned int pickClass();

    /* Omnet stuff */
    virtual bool isNodeUp();
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback) override;
    virtual void finish() override;

    // from AppBase //
    /* TODO: onInterest, onData, onTimeout, createNextInterest */

    /* */
    virtual void startApp();

    /* */
    virtual Interest* createNextInterest();

    /* */
    virtual void scheduleNextInterest(simtime_t previous);

    /* */
    virtual void expressInterest(Interest* interest);

    /* */
    virtual void onTimeout(Interest *interest);

    /* */
    virtual void handleMessage(cMessage *msg) override;

    /* */
    virtual void onData(Data *data);

    /* */
    virtual void writeLog(int seqN, const char* name, double data1, int data2);

public:
    ConsumerAppBase();
    virtual ~ConsumerAppBase();
};

} //namespace

#endif
