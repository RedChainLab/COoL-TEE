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

#include "AbstractFifo.h"

#include "MultiSink.h"
#include "SourcedMsg_m.h"

#include <iostream>
#include <iomanip>
#include <sstream>

AbstractFifo::~AbstractFifo()
{
    delete msgServiced;
    cancelAndDelete(endServiceMsg);
}

void AbstractFifo::initialize()
{
    endServiceMsg = new cMessage("end-service");
    queue.setName("queue");

    droppedJob = registerSignal("droppedJob");
    qlenSignal = registerSignal("qlen");
    busySignal = registerSignal("busy");
    queueingTimeSignal = registerSignal("queueingTime");
    emit(qlenSignal, queue.getLength());
    emit(busySignal, false);
}

cMessage* AbstractFifo::front(cQueue q)
{
    return (cMessage*)q.front();
}
bool AbstractFifo::isEmpty(cQueue q)
{
    return q.isEmpty();
}
void AbstractFifo::handled(SourcedMsg*)
{
    //do nothing
}

cMessage* AbstractFifo::dropOverdue()
{ // Returns an honest job ready to be served and not overdue. Nullptr if the queue is empty (i.e. all jobs overdue)
    cMessage* served=nullptr;

    while(!isEmpty(queue) && !served)
    { // Drop honest jobs that waited more than the time that enables them to be served in time
        cMessage* honestQueueFront=front(queue);
        simtime_t queuingTime=simTime() - honestQueueFront->getTimestamp();
        if(queuingTime <= par("dropWaittime").doubleValueInUnit("s") - par("serviceTime").doubleValueInUnit("s"))
        {
            served= front(queue);
        }
        else
        {
            SourcedMsg* droppedMsg= check_and_cast<SourcedMsg*>(queue.pop());
            emit(droppedJob, queuingTime);
            MultiSink* sink;
            sink=check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", droppedMsg->getSource()));

            sink->droppedJobs[this->getIndex()].push_back(droppedMsg->getCreationTime()+par("dropWaittime").doubleValueInUnit("s"));
            delete droppedMsg;
        }
    }
    return served;
}

void AbstractFifo::handleMessage(cMessage *msg)
{
    if(par("dropWaittime").doubleValueInUnit("s")>0){
        dropOverdue();
    }
    if (msg == endServiceMsg) { // When a message has been served
        endService(msgServiced);
        if (isEmpty(queue)) {
            this->handled(check_and_cast<SourcedMsg*>(msgServiced));
            msgServiced = nullptr;
            emit(busySignal, false);
        }
        else {
            this->handled(check_and_cast<SourcedMsg*>(msgServiced));
            msgServiced = (cMessage *)queue.pop();
            emit(qlenSignal, queue.getLength());
            emit(queueingTimeSignal, simTime() - msgServiced->getTimestamp());
            simtime_t serviceTime = startService(msgServiced);
            scheduleAt(simTime()+serviceTime, endServiceMsg);
        }
    }
    else
    {
        if (!msgServiced) { // If the queue is empty, serve the new mesage directly
            arrival(msg);
            msgServiced = msg;
            emit(queueingTimeSignal, SIMTIME_ZERO);
            simtime_t serviceTime = startService(msgServiced);
            scheduleAt(simTime()+serviceTime, endServiceMsg);

            emit(busySignal, true);
        }
        else { // If the queue is not empty, add the message to the queue
            arrival(msg);
            queue.insert(msg);
            msg->setTimestamp();
            emit(qlenSignal, queue.getLength());
        }
        // Set queue entry time in msg
        SourcedMsg* sMsg= check_and_cast<SourcedMsg*>(msg);
        sMsg->setTime_queue(simTime());
    }
}

void AbstractFifo::arrival(cMessage *msg)
{
    // Set arrivaltime in msg
    SourcedMsg* sMsg= check_and_cast<SourcedMsg*>(msg);
    sMsg->setTime_in(simTime());
}
