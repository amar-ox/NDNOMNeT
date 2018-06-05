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

#include "Tools.h"


namespace inet {
using std::cout;

unsigned Tools::encodePacket(NdnPacket* packet, unsigned char* tlv)
{
    if (packet->getType() == INTEREST){
        tlv[0] = INTEREST;
        unsigned pos = encodeName(packet->getName(), tlv+2) + 2;
        Interest* interest = check_and_cast<Interest *>(packet);
        if (interest->getCanBePrefix()){
            tlv[pos] = CAN_BE_PREFIX;
            tlv[pos+1] = 0;
            pos+=2;
        }
        if (interest->getMustBeFresh()){
            tlv[pos] = MUST_BE_FRESH;
            tlv[pos+1] = 0;
            pos+=2;
        }
        unsigned fh_size = interest->getForwardingHintArraySize();
        if ( fh_size ){
            tlv[pos] = FORWARDING_HINT;
            tlv[pos+1] = fh_size;
            pos+=2;
            for(unsigned i = 0; i < fh_size; i++)
                tlv[pos+i] = interest->getForwardingHint(i);
            pos+=fh_size;
        }
        unsigned nonce_size = interest->getNonceArraySize();
        if ( nonce_size ){
            tlv[pos] = NONCE;
            tlv[pos+1] = nonce_size;
            pos+=2;
            for(unsigned i = 0; i < nonce_size; i++)
                tlv[pos+i] = interest->getNonce(i);
            pos+=nonce_size;
        }
        if (interest->getInterestLifetimeMs() != 4000){
            tlv[pos] = INTEREST_LIFETIME;
            tlv[pos+1] = sizeof(interest->getInterestLifetimeMs());
            pos+=2;
            unsigned interestLifetime = interest->getInterestLifetimeMs();
            memcpy(tlv+pos, &interestLifetime, sizeof(interest->getInterestLifetimeMs()));
            pos+=sizeof(interest->getInterestLifetimeMs());
        }
        if (interest->getHopLimit() != 256){
            tlv[pos] = HOP_LIMIT;
            tlv[pos+1] = sizeof(interest->getHopLimit());
            pos+=2;
            unsigned hopLimit = interest->getHopLimit();
            memcpy(tlv+pos, &hopLimit, sizeof(interest->getHopLimit()));
            pos+=sizeof(interest->getHopLimit());
        }
        unsigned parameters_size = interest->getParametersArraySize();
        if ( parameters_size ){
            tlv[pos] = PARAMETERS;
            tlv[pos+1] = parameters_size;
            pos+=2;
            for(unsigned i = 0; i < parameters_size; i++)
                tlv[pos+i] = interest->getParameters(i);
            pos+=parameters_size;
        }
        tlv[1] = pos - 2;
        return pos;
    }
    else if ( packet->getType() == DATA ){
      tlv[0] = DATA;
      unsigned pos = encodeName(packet->getName(), tlv+2) + 2;
      Data* data = check_and_cast<Data *>(packet);

      unsigned char metaInfoSize = 0;
      unsigned metaInfoPos = 0;
      if (data->getContentType() != contentType::OMITTED){
          if (!metaInfoPos){
              metaInfoPos = pos;
              tlv[metaInfoPos] = META_INFO;
              pos+=2;
          }
          tlv[pos] = CONTENT_TYPE;
          tlv[pos+1] = sizeof(data->getContentType());
          metaInfoSize+=(tlv[pos+1]+2);
          pos+=2;
          unsigned contentType = data->getContentType();
          memcpy(tlv+pos, &contentType, sizeof(data->getContentType()));
          pos+=sizeof(data->getContentType());
      }
      if (data->getFreshnessPeriod()){
          if (!metaInfoPos){
              metaInfoPos = pos;
              tlv[metaInfoPos] = META_INFO;
              pos+=2;
          }
          tlv[pos] = FRESHNESS_PERIOD;
          tlv[pos+1] = sizeof(data->getFreshnessPeriod());
          metaInfoSize+=(tlv[pos+1]+2);
          pos+=2;
          unsigned freshnessPeriod = data->getFreshnessPeriod();
          memcpy(tlv+pos, &freshnessPeriod, sizeof(data->getFreshnessPeriod()));
          pos+=sizeof(data->getFreshnessPeriod());
      }
      unsigned finalBlockId_size = data->getFinalBlockIdArraySize();
      if (finalBlockId_size){
          if (!metaInfoPos){
              metaInfoPos = pos;
              tlv[metaInfoPos] = META_INFO;
              pos+=2;
          }
          tlv[pos] = FINAL_BLOCK_ID;
          tlv[pos+1] = finalBlockId_size;
          metaInfoSize+=(tlv[pos+1]+2);
          pos+=2;
          for(unsigned i = 0; i < finalBlockId_size; i++)
              tlv[pos+i] = data->getFinalBlockId(i);
          pos+=finalBlockId_size;
      }
      if (metaInfoSize)
          tlv[metaInfoPos+1] = metaInfoSize;

      unsigned content_size = data->getContentArraySize();
      if ( content_size ){
          tlv[pos] = CONTENT;
          tlv[pos+1] = content_size;
          pos+=2;
          for(unsigned i = 0; i < content_size; i++)
              tlv[pos+i] = data->getContent(i);
          pos+=content_size;
      }

      unsigned char signatureInfoSize = 3;
      unsigned signatureInfoPos = pos;
      tlv[signatureInfoPos] = META_INFO;
      pos+=2;

      tlv[pos] = SIGNATURE_TYPE;
      tlv[pos+1] = sizeof(data->getSignatureType());
      pos+=2;
      unsigned signatureType = data->getSignatureType();
      memcpy(tlv+pos, &signatureType, sizeof(data->getSignatureType()));
      pos+=sizeof(data->getSignatureType());

      unsigned keyLocator_size = data->getKeyLocatorArraySize();
      if ( keyLocator_size ){
          tlv[pos] = KEY_LOCATOR;
          tlv[pos+1] = keyLocator_size;
          signatureInfoSize+=(tlv[pos+1]+2);
          pos+=2;
          for(unsigned i = 0; i < keyLocator_size; i++)
              tlv[pos+i] = data->getKeyLocator(i);
          pos+=keyLocator_size;
      }
      tlv[signatureInfoPos+1] = signatureInfoSize;

      tlv[1] = pos - 2;
      return pos;
    }
    return 0;
}

unsigned Tools::computeTlvPacketSize(NdnPacket* packet)
{
    if (packet->getType() == INTEREST){
        unsigned pos = 2;
        pos+=computeTlvNameSize(packet->getName());
        Interest* interest = check_and_cast<Interest *>(packet);
        if (interest->getCanBePrefix())
            pos+=2;
        if (interest->getMustBeFresh())
            pos+=2;
        unsigned fh_size = interest->getForwardingHintArraySize();
        if (fh_size)
            pos+=(fh_size+2);
        unsigned nonce_size = interest->getNonceArraySize();
        if (nonce_size)
            pos+=(nonce_size+2);
        if (interest->getInterestLifetimeMs() != 4000)
            pos+=(sizeof(interest->getInterestLifetimeMs())+2);
        if (interest->getHopLimit() != 256)
            pos+=(sizeof(interest->getHopLimit())+2);
        unsigned parameters_size = interest->getParametersArraySize();
        if ( parameters_size )
            pos+=(parameters_size+2);
        return pos;
    }
    else if ( packet->getType() == DATA ){
        unsigned pos = 2;
        pos+=computeTlvNameSize(packet->getName());
        Data* data = check_and_cast<Data *>(packet);
        bool hasMetaInfo = false;
        if (data->getContentType() != contentType::OMITTED){
            hasMetaInfo = true;
            pos+=(sizeof(data->getContentType())+2);
        }
        if (data->getFreshnessPeriod()){
            hasMetaInfo = true;
            pos+=(sizeof(data->getFreshnessPeriod())+2);
        }
        unsigned finalBlockId_size = data->getFinalBlockIdArraySize();
        if (finalBlockId_size){
            hasMetaInfo = true;
            pos+=(finalBlockId_size+2);
        }
        if (hasMetaInfo)
            pos+=2;
        unsigned content_size = data->getContentArraySize();
        if (content_size)
            pos+=(content_size+2);
        pos+=2;
        pos+=(sizeof(data->getSignatureType())+2);
        unsigned keyLocator_size = data->getKeyLocatorArraySize();
        if (keyLocator_size)
            pos+=(keyLocator_size+2);
        return pos;
    }
    return 0;
}

unsigned Tools::encodeName(const char* name, unsigned char* tlv)
{
    tlv[0] = NAME;
    unsigned pos = 2;
    cStringTokenizer tokenizer(name, "/");
    const char *component;
    while ((component = tokenizer.nextToken()) != nullptr){
        tlv[pos] = GENERIC_NAME_COMPONENT;
        tlv[pos+1] = strlen(component);
        pos+=2;
        memcpy(tlv+pos, component, strlen(component));
        pos+=strlen(component);
    }
    tlv[1] = pos - 2;
    return pos;
}

unsigned Tools::computeTlvNameSize(const char* name)
{
    unsigned pos = 2;
    cStringTokenizer tokenizer(name, "/");
    const char *component;
    while ((component = tokenizer.nextToken()) != nullptr)
        pos = pos+(strlen(component)+2);
    return pos;
}

void Tools::printPacket(NdnPacket* packet)
{
    if (packet->getType() == INTEREST){
        Interest* interest = check_and_cast<Interest *>(packet);
        cout << "Interest: " << endl;
        cout << "\tName: " << (unsigned char*)interest->getName() << endl;

        if (interest->getCanBePrefix())
            cout << "\tCanBePrefix: " << interest->getCanBePrefix() << endl;

        if (interest->getMustBeFresh())
            cout << "\tMustBeFresh: " << interest->getMustBeFresh() << endl;

        if ( interest->getForwardingHintArraySize() ){
                unsigned char forwardingHint[interest->getForwardingHintArraySize()];
                for(unsigned i = 0; i < interest->getForwardingHintArraySize(); i++)
                    forwardingHint[i] = interest->getForwardingHint(i);
                cout << "\tForwardingHint: " << (unsigned char*)forwardingHint << endl;
        }

        if ( interest->getNonceArraySize() ){
            unsigned char nonce[interest->getNonceArraySize()];
            for(unsigned i = 0; i < interest->getNonceArraySize(); i++)
                nonce[i] = interest->getNonce(i);
            cout << "\tNonce: " << (unsigned char*)nonce << endl;
        }

        if (interest->getInterestLifetimeMs() != 4000)
            cout << "\tInterestLifetime: " << interest->getInterestLifetimeMs() << endl;

        if (interest->getHopLimit() != 256)
            cout << "\tHopLimit: " << interest->getHopLimit() << endl;

        if ( interest->getParametersArraySize() ){
            unsigned char parameters[interest->getParametersArraySize()];
            for(unsigned i = 0; i < interest->getParametersArraySize(); i++)
                parameters[i] = interest->getParameters(i);
            cout << "\tParameters: " << (unsigned char*)parameters << endl;
        }
    }
    else if ( packet->getType() == DATA ){
        Data* data = check_and_cast<Data *>(packet);
        cout << "Data: " << endl;
        cout << "\tName: " << (unsigned char*)data->getName() << endl;

        bool hasMetaInfo = false;
        if (data->getContentType() != contentType::OMITTED){
            if (!hasMetaInfo){
                hasMetaInfo = true;
                cout << "\tMetaInfo: " << endl;
            }
            cout << "\t\tContentType: " << data->getContentType() << endl;
        }
        if (data->getFreshnessPeriod()){
            if (!hasMetaInfo){
                hasMetaInfo = true;
                cout << "\tMetaInfo: " << endl;
            }
            cout << "\t\tFreshnessPeriod: " << data->getFreshnessPeriod() << endl;
        }
        if (data->getFinalBlockIdArraySize()){
            if (!hasMetaInfo){
                hasMetaInfo = true;
                cout << "\tMetaInfo: " << endl;
            }
            unsigned char finalBlockId[data->getFinalBlockIdArraySize()];
            for(unsigned i = 0; i < data->getFinalBlockIdArraySize(); i++)
                finalBlockId[i] = data->getFinalBlockId(i);
            cout << "\t\tFinalBlockId: " << (unsigned char*)finalBlockId << endl;
        }
        if ( data->getContentArraySize() ){
            unsigned char content[data->getContentArraySize()];
            for(unsigned i = 0; i < data->getContentArraySize(); i++)
                content[i] = data->getContent(i);
            cout << "\tContent: " << (unsigned char*)content << endl;
        }
        cout << "\tSignatureInfo: " << endl;
        cout << "\t\tSignatureType: " << data->getSignatureType() << endl;
        if ( data->getKeyLocatorArraySize() ){
            unsigned char keyLocator[data->getKeyLocatorArraySize()];
            for(unsigned i = 0; i < data->getKeyLocatorArraySize(); i++)
                keyLocator[i] = data->getKeyLocator(i);
            cout << "\t\tKeyLocator: " << keyLocator << endl;
        }
    }
}

bool Tools::isCorrect(NdnPacket* packet)
{
    return true;
}

}    // namespace inet
