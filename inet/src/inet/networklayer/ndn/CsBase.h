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

#ifndef __INET_CS_BASE_H_
#define __INET_CS_BASE_H_

#include <map>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/networklayer/contract/ndn/ICs.h"
#include "packets/NdnPackets_m.h"
#include "packets/Tools.h"

using namespace std;
using namespace omnetpp;

class Node {
  public:
  std::string key;
  Data *value;
  Node *prev, *next;
  Node(std::string k, Data *v): key(k), value(v), prev(nullptr), next(nullptr) {}
  Data* getValue(){return value;}
};

class DoublyLinkedList {
  Node *front, *rear;

  bool isEmpty() {
      return rear == nullptr;
  }

  public:
  DoublyLinkedList(): front(nullptr), rear(nullptr) {}

  Node* add_page_to_head(std::string key, Data* value) {
      Node *page = new Node(key, value);
      if(!front && !rear) {
          front = rear = page;
      }
      else {
          page->next = front;
          front->prev = page;
          front = page;
      }
      return page;
  }

  void move_page_to_head(Node *page) {
      if(page==front) {
          return;
      }
      if(page == rear) {
          rear = rear->prev;
          rear->next = nullptr;
      }
      else {
          page->prev->next = page->next;
          page->next->prev = page->prev;
      }

      page->next = front;
      page->prev = nullptr;
      front->prev = page;
      front = page;
  }

  void remove_rear_page() {
      if(isEmpty()) {
          return;
      }
      if(front == rear) {
          delete rear;
          front = rear = nullptr;
      }
      else {
          Node *temp = rear;
          rear = rear->prev;
          rear->next = nullptr;
          delete temp;
      }
  }
  Node* get_rear_page() {
      return rear;
  }
};


namespace inet {

class INET_API CsBase : public cSimpleModule, public ICs
{
public:
    CsBase();

    /* */
    virtual ~CsBase();

    /* */
    virtual Data* lookup(Interest* interest) override;

    /* */
    virtual bool add(Data *data) override;

    /* */
    virtual bool remove(const char* name) override;

    /* */
    virtual void cleanStaled() override;

    /* */
    virtual void refreshDisplay() const override;

    /* */
    virtual void finish() override;

    /* */
    static simsignal_t dataStaleSignal;

private:
    //std::vector<CsEntry *> entries;
    int size;
    DoublyLinkedList *pageList;
    map<std::string, Node*> pageMap;

    cMessage *checkDataStale = new cMessage("ds");
    unsigned maxSize;

    int numHit = 0;
    int numMiss = 0;
    int numAdded = 0;
    int numRemoved = 0;
    int numStale = 0;

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void print();
};

} //namespace

#endif
