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

#include "Xu.h"

namespace inet {
using std::cout;

simsignal_t Xu::pakcetReceivedSignal = cComponent::registerSignal("pakcetReceived");
simsignal_t Xu::pakcetProcessingBeginSignal = cComponent::registerSignal("pakcetProcessingBegin");
simsignal_t Xu::pakcetProcessingEndSignal = cComponent::registerSignal("pakcetProcessingEnd");
simsignal_t Xu::pakcetProcessingErrorSignal = cComponent::registerSignal("pakcetProcessingError");
simsignal_t Xu::pakcetProcessingSuccessSignal = cComponent::registerSignal("pakcetProcessingSuccess");
Register_Abstract_Class(IXu::Notification);
Define_Module(Xu);

Xu::Xu()
{
}

Xu::~Xu()
{
}

void Xu::initialize()
{
}

void Xu::handleMessage(cMessage *msg)
{
}


void Xu::refreshDisplay() const
{
}

void Xu::finish()
{
}

bool Xu::processPacket(NdnPacket *packet)
{
    return false;
}

bool Xu::sendResponse(NdnPacket* packet)
{
    return false;
}


} //namespace
