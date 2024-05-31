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

#include "PoTSource.h"
#include "DeadlineMsg_m.h"
#include "PoTMsg_m.h"
#include "MultiSink.h"
Define_Module(PoTSource);

void PoTSource::handleMessage(cMessage *msg)
{
    MultiSink* sink=check_and_cast<MultiSink *>(this->getParentModule()->getSubmodule(this->par("sinkVectorName").stringValue(),getIndex()));
    if(!strcmp(msg->getName(),"deadline"))
    {
        DeadlineMsg* smsg=check_and_cast<DeadlineMsg *>(msg);
        auto elem = sink->pendingRequests.find(smsg->getID());
        if(elem!=sink->pendingRequests.end())
        {
            sink->droppedRequests[elem->second]++;
            sink->pendingRequests.erase(smsg->getID());
            sink->emit(sink->droppedSignals[smsg->getProvider()],true);
        }
        else
        {
            sink->emit(sink->droppedSignals[smsg->getProvider()],false);
        }
        //setDisplayTags();
        delete msg;
    }
    else if(!strcmp(msg->getName(),"sendMessageEvent"))
    {
        sink->totalRequests++;

        std::vector<int> chosenProviders;
        std::vector<bool> isChosen(this->getAncestorPar("n_fifos").intValue());
        for(int i=0;i<k_reqs && i< this->getAncestorPar("n_fifos").intValue();)
        {
            int chosenProvider=chooseProvider();
            if(!isChosen[chosenProvider])
            {
                isChosen[chosenProvider]=true;
                chosenProviders.push_back(chosenProvider);
                i++;
            }
        }

        for(int chosenProvider:chosenProviders)
        {
            sink->sentRequests[chosenProvider]++;

            PoTMsg *job = new PoTMsg("job");
            job->setSource(this->getIndex()+this->par("idx_offset").intValue());
            job->setID(sink->totalRequests+job->getSource()*100000);
            for(int otherProvider:chosenProviders)
            {
                if(otherProvider!=chosenProvider)
                {
                    job->setOtherProv(otherProvider);
                }
            }
            // Set sendtime in msg
            job->setTime_send(simTime());

            send(job, "out", chosenProvider);
            emit(sentMsgSignal[chosenProvider],true);
            sink->emit(sink->debtSignals[chosenProvider],++sink->providerDebt[chosenProvider]);
        }
        if(par("dropWaittime").doubleValue()>0)
        {
            DeadlineMsg* dmsg=new DeadlineMsg("deadline");
            dmsg->setID(sink->totalRequests);
            dmsg->setProvider(chosenProviders[0]);
            scheduleAt(simTime()+par("dropWaittime").doubleValue(), dmsg);
        }
        sink->pendingRequests[sink->totalRequests]=chosenProviders[0];

        scheduleAt(simTime()+par("interarrivalTime").doubleValue(), sendMessageEvent);
    }
    else
    {
        throw(std::logic_error("Unknown message"));
    }
}
