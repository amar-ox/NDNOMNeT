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

cplusplus {{	
    #include "inet/linklayer/common/MACAddress.h"
}};


packet NdnPacket
{
    // name already in the Omnet++ message
    short type = 0;				// Interest (5) or Data (6)       
    int hopCount = 0;			// for stats only   
    int seqNo = 0; 				// for stats only
    short prefixLength = 1;		// for help :)
    bool flood = false;  
    float cost = 0;
};


enum contentType
{
	BLOB = 0; 					// payload identified by the data name; this is the default ContentType
	LINK = 1; 					// a list of delegations (see Link Object)
	KEY = 2; 					// public key
	NACK = 3; 					// application-level NACK	
	OMITTED = 255;				// content type is not specified
};

enum signatureType
{
	D_SHA256 = 0; 	 			// Integrity protection using SHA-256 digest
	RSA_SHA256 = 1; 	 		// Integrity and provenance protection using RSA signature over a SHA-256 digest
	ECDSA_SHA256 = 3; 			// Integrity and provenance protection using an ECDSA signature over a SHA-256 digest
	HMAC_SHA256 = 4;			// Integrity and provenance protection using SHA256 hash-based message authentication codes 
}; 


enum signatureLength
{
	S_D_SHA256 = 32; 			
	S_RSA_SHA256 = 128;			// smallest signature length
	S_ECDSA_SHA256 = 64;		// smallest signature length for a 224 bit key
	S_HMAC_SHA256 = 32; 
}; 


packet Interest extends NdnPacket
{
    bool canBePrefix = false;					// default omitted
    bool MustBeFresh = false;					// default omitted
    unsigned char ForwardingHint[];       		// default omitted
    char nonce[];  								// default omitted, length is 4 if present
    unsigned int interestLifetimeMs = 4000;		// default omitted
    unsigned int hopLimit = 256;				// 256 = omitted
    char parameters[];							// default omitted
};

packet Data extends NdnPacket
{       
    // implicit MetaInfo:
    	unsigned int contentType @enum(contentType);	// default omitted
   		unsigned int freshnessPeriod = 0;				// default omitted (ms)
    	char finalBlockId[];   							// default omitted
    char content[];										// default omitted
    // implicit SignatureInfo:
    	unsigned int signatureType @enum(signatureType);
    	char keyLocator[];    							// default omitted
};

cplusplus {{
    #include "NdnPackets_m.h"
	typedef NdnPacket *NDNPacketPtr;  	
}};

//class MACAddress;
class noncobject NDNPacketPtr;

message SendDelayed
{
    NDNPacketPtr delayedPacket;
    short type;
    int face = 0;
    string macDest;
    string macSrc;     
};

