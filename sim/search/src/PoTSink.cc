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

#include "PoTSink.h"
#include "PoTMsg_m.h"

Define_Module(PoTSink);

void PoTSink::handleMessage(cMessage *msg)
{
    PoTMsg* smsg=check_and_cast<PoTMsg *>(msg);
    // Set reception time in msg
    smsg->setTime_recv(simTime());
    simtime_t lifetime = simTime() - msg->getCreationTime();

    if(smsg->getTime_send().dbl()>=this->getAncestorPar("warmup").doubleValue())
    {
        (*outfileAll) << this->getIndex() << "," << smsg->getSource() << "," << smsg->getTime_send() << "," << smsg->getTime_in() << "," << smsg->getTime_queue() << "," << smsg->getTime_serv() << "," << smsg->getTime_out() << "," << smsg->getTime_recv() << std::endl;
    }

    int nbDataPoints=providerLatencies[smsg->getSource()].size();
    providerAvgLatency[smsg->getSource()]=(providerAvgLatency[smsg->getSource()]*nbDataPoints+lifetime)/(nbDataPoints+1);
    providerLatencies[smsg->getSource()].push_back(lifetime);

    static int msgCount=0;
    if(updateRatiosCount>0 && ++msgCount%updateRatiosCount==0)
    {
        updateRatios();
    }

    auto elem = pendingRequests.find(smsg->getID());
    if(elem!=pendingRequests.end())
    {
        pendingRequests.erase(smsg->getID());
        emit(lifetimeFirstRespSignal, lifetime);
    }
#if LOG
    EV << "Received " << msg->getName() << ", lifetime: " << lifetime << "s " << "from SP "<< smsg->getSource() << endl;
#endif
    emit(lifetimeSignal, lifetime);

    emit(lifetimeThroughXSignals[smsg->getSource()], lifetime);
    emit(debtSignals[smsg->getSource()], --providerDebt[smsg->getSource()]);
    delete msg;
}
