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

#ifndef ABSTRACTFIFO_H_
#define ABSTRACTFIFO_H_

#include <omnetpp.h>
#include "IQueue.h"

using namespace omnetpp;

/**
 * Abstract base class for single-server queues. Subclasses should
 * define startService() and endService(), and may override other
 * virtual functions.
 */
class AbstractFifo : public cSimpleModule, IQueue
{
  public:
    cMessage *msgServiced = nullptr;
    cMessage *endServiceMsg = nullptr;
    cQueue queue;
    simsignal_t qlenSignal;
    simsignal_t busySignal;
    simsignal_t queueingTimeSignal;
    simsignal_t droppedJob;

    simtime_t lastRequestTime;
    double effectiveThroughput;

  public:
    virtual ~AbstractFifo();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;


    virtual cMessage* dropOverdue();
    // hook functions to (re)define behaviour
    virtual void arrival(cMessage *msg);
    virtual simtime_t startService(cMessage *msg) = 0;
    virtual void endService(cMessage *msg) = 0;

    virtual cMessage* front(cQueue q) override;
    virtual bool isEmpty(cQueue q) override;
    virtual void handled(SourcedMsg*) override;
};

#endif /* ABSTRACTFIFO_H_ */
