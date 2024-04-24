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

#include "PoTFifo.h"
#include "LenMsg_m.h"

Define_Module(PoTFifo);

PoTFifo::~PoTFifo() {
    potRQqueueLen.clear();
}

void PoTFifo::handleMessage(cMessage* msg)
{
    if(par("dropWaittime").doubleValueInUnit("s")>0){
        dropOverdue();
    }
    if (msg->arrivedOn("prov$i"))
    {
        // Check map and if msg->len < self RQ len, remove request
        LenMsg* lmsg=check_and_cast<LenMsg *>(msg);
        int twinLen = lmsg->getLen();
        if(potRQqueueLen.find(lmsg->getID())!=potRQqueueLen.end() && twinLen<potRQqueueLen[lmsg->getID()].second)
        {
            PoTMsg* rqMsg=potRQqueueLen[lmsg->getID()].first;
            queue.remove(rqMsg);
            // handle unserved RQ msg destruction
        }
        delete lmsg;
    }
    else if (msg == endServiceMsg) { // When a message has been served
        endService(msgServiced);
        if (isEmpty(queue)) {
            //this->handled(check_and_cast<PoTMsg*>(msgServiced));
            msgServiced = nullptr;
            emit(busySignal, false);
        }
        else {
            //this->handled(check_and_cast<PoTMsg*>(msgServiced));
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
        PoTMsg* sMsg= check_and_cast<PoTMsg*>(msg);
        sMsg->setTime_queue(simTime());
        int twinProv=sMsg->getOtherProv();
        LenMsg* lenMsg = new LenMsg;
        lenMsg->setID(sMsg->getID());
        lenMsg->setLen(queue.getLength());
        potRQqueueLen[sMsg->getID()]=std::pair<PoTMsg*, int>(sMsg, queue.getLength());
        send(lenMsg,"prov$o",twinProv);
    }
}

void PoTFifo::arrival(cMessage *msg)
{
    // Set arrivaltime in msg
    PoTMsg* sMsg= check_and_cast<PoTMsg*>(msg);
    sMsg->setTime_in(simTime());
}

simtime_t PoTFifo::startService(cMessage *msg)
{
    PoTMsg* smsg=check_and_cast<PoTMsg *>(msg);
    smsg->setTime_serv(simTime());
    EV << "Starting service of " << msg->getName() << endl;
    return par("serviceTime");
}

void PoTFifo::endService(cMessage *msg)
{
    PoTMsg* smsg=check_and_cast<PoTMsg *>(msg);
    smsg->setTime_out(simTime());
#if LOG
    EV << "Completed service of " << smsg->getName() << " from port " << smsg->getSource() << endl;
#endif
    int destination=smsg->getSource();

    smsg->setSource(this->getIndex()+par("offset").intValue());
    send(smsg, "out", destination);
}

