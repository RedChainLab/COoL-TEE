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

bool resolveConflict(int self, int twin)
{
    return (((self%2)==(twin%2)) && (self < twin)) || (((self%2)!=(twin%2)) && (self > twin));
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
        EV << "RQ ID " << lmsg->getID()<<std::endl;
        EV << "TwinLen " << twinLen << std::endl;
        EV << "Found | ownlen "<<(potRQqueueLen.find(lmsg->getID())!=potRQqueueLen.end()) << " " << ((potRQqueueLen.find(lmsg->getID())!=potRQqueueLen.end())?potRQqueueLen[lmsg->getID()].second:-1) << std::endl;
        EV << "Conflict "<< ((potRQqueueLen.find(lmsg->getID())!=potRQqueueLen.end())?resolveConflict(lmsg->getSource(),this->getIndex()+par("offset").intValue()):-1) << std::endl;
        EV << "IDs " << lmsg->getSource() << " " << this->getIndex()+par("offset").intValue() << std::endl;
        if(potRQqueueLen.find(lmsg->getID())!=potRQqueueLen.end())
        {
            if(
                  (twinLen<potRQqueueLen[lmsg->getID()].second)
                  ||
                  (twinLen==potRQqueueLen[lmsg->getID()].second && resolveConflict(lmsg->getSource(),this->getIndex()+par("offset").intValue()))
              )
            {
                PoTMsg* rqMsg=potRQqueueLen[lmsg->getID()].first;
                EV << "Bef " << queue.getLength() << std::endl;
                queue.remove(rqMsg);
                EV << "Aft " << queue.getLength() << std::endl;
                // handle unserved RQ msg destruction
            }
        }
        else
        {
            EV << "Writing twinLen " <<twinLen<<std::endl;
            potRQTwinQueueLen[lmsg->getID()]=twinLen;
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
        // Set queue entry time in msg
        PoTMsg* sMsg= check_and_cast<PoTMsg*>(msg);
        if(
                potRQTwinQueueLen.find(sMsg->getID())==potRQTwinQueueLen.end()
                ||
                (
                        potRQTwinQueueLen[sMsg->getID()]>queue.getLength()+(msgServiced?1:0)
                        ||
                        (
                                potRQTwinQueueLen[sMsg->getID()]==queue.getLength()+(msgServiced?1:0)
                                &&
                                !resolveConflict(sMsg->getOtherProv(),this->getIndex()+par("offset").intValue())
                        )
                )
          )
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
            sMsg->setTime_queue(simTime());
            int twinProv=sMsg->getOtherProv();
            LenMsg* lenMsg = new LenMsg;
            lenMsg->setSource(this->getIndex()+par("offset").intValue());
            lenMsg->setID(sMsg->getID());
            lenMsg->setLen(queue.getLength());
            potRQqueueLen[sMsg->getID()]=std::pair<PoTMsg*, int>(sMsg, queue.getLength());
            send(lenMsg,"prov$o",(twinProv>lenMsg->getSource()?twinProv-1:twinProv));
        }
        else
        {
            EV << "TwinQLen " << potRQTwinQueueLen[sMsg->getID()] << std::endl;
            delete msg;
        }
        EV << "TwinLen at arrival " << (potRQTwinQueueLen.find(sMsg->getID())!=potRQTwinQueueLen.end()?potRQTwinQueueLen[sMsg->getID()]:-1) << " " << resolveConflict(sMsg->getOtherProv(),this->getIndex()+par("offset").intValue()) <<std::endl;
        EV << "RQ ID " << sMsg->getID()<<std::endl;
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

