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

#include "MultiSource.h"

Define_Module(MultiSource);

#include "SourcedMsg_m.h"
#include "DeadlineMsg_m.h"
#include "MultiSink.h"

#include <iostream>
#include <iomanip>
#include <sstream>

void MultiSource::initialize()
{
    Source::initialize();
    k_reqs=this->par("kReqs").intValue();
}

void MultiSource::handleMessage(cMessage *msg)
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

            SourcedMsg *job = new SourcedMsg("job");
            job->setSource(this->getIndex()+this->par("idx_offset").intValue());
            job->setID(sink->totalRequests);

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

int MultiSource::chooseProvider()
{
    static int out_port=0;
    return out_port++%this->gateSize("out");
}

void MultiSource::setDisplayTags()
{
    MultiSink* sink=check_and_cast<MultiSink *>(this->getParentModule()->getSubmodule("sinks",getIndex()));
    std::stringstream ss;
    for(int i=0;i<sink->droppedRequests.size();i++)
    {
        ss << std::setw(6) << std::setfill(' ') << std::setprecision(4);
        ss << sink->droppedRequests[i]<<"rq ("<<((sink->sentRequests[i]!=0)?100*double(sink->droppedRequests[i])/sink->sentRequests[i]:0)<<"%)";
        if(i+1<sink->droppedRequests.size())
        {
            ss << " ";
        }
    }

    sink->getDisplayString().setTagArg("t", 0, ss.str().c_str());
}
