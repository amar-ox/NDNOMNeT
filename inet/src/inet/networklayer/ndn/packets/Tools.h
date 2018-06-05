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

#ifndef __NDN_PACKET_TOOLS_H_
#define __NDN_PACKET_TOOLS_H_

#include <iostream>
#include <sstream>
#include "NdnPackets_m.h"

/*
 * Simple TLV packet deconding/parsing tools.
 * From NDN Packet Format Specification 0.3 documentation.
 * Not all fields/types supported yet!
 * */

//Packet types
#define INTEREST 5                      //0x05
#define DATA    6                       //0x06

// Common fields
#define NAME    7                       //0x07
#define GENERIC_NAME_COMPONENT    8     //0x08

// Interest packet
#define CAN_BE_PREFIX     33            //0x21
#define MUST_BE_FRESH     18            //0x12
#define FORWARDING_HINT  30             //0x1e
#define NONCE   10                      //0x0a
#define INTEREST_LIFETIME    12         //0x0c
#define HOP_LIMIT    34                 //0x22
#define PARAMETERS  35                  //0x23

// Data packet
#define META_INFO   20                  //0x14
#define CONTENT     21                  //0x15
#define SIGNATURE_INFO   22             //0x16
#define SIGNATURE_VALUE  23             //0x17

// Data/MetaInfo
#define CONTENT_TYPE     24             //0x18
#define FRESHNESS_PERIOD     25         //0x19
#define FINAL_BLOCK_ID    26            //0x1a

// Data/Signature
#define SIGNATURE_TYPE   27             //0x1b
#define KEY_LOCATOR  28                 //0x1c

// Link Object
#define DELEGATION  31                  //0x1f
#define PREFERENCE  30                  //0x1e

using namespace omnetpp;
namespace inet {

class Tools {
public:
    static unsigned encodePacket(NdnPacket* packet, unsigned char* tlv);
    static unsigned computeTlvPacketSize(NdnPacket* packet);
    static unsigned encodeName(const char* name, unsigned char* tlv);
    static unsigned computeTlvNameSize(const char* name);
    static bool isCorrect(NdnPacket* packet);
    static void printPacket(NdnPacket* packet);

private:
    Tools(){}
};

} // namespace inet

#endif
