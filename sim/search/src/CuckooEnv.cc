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

#include "CuckooEnv.h"
#include "MultiSink.h"

#include <iostream>
#include <vector>
#include <algorithm>

Define_Module(CuckooEnv);

void CuckooEnv::initialize(int stage)
{
    Environment::initialize(stage);
    if(stage==1)
    {
        assignLoads();
    }
}

void CuckooEnv::assignLoads()
{
    std::cout << "Assigning loads" << std::endl;
    int nHonReqs=this->getParentModule()->par("n_RH").intValue();
    int nMalReqs=this->getParentModule()->par("n_RM").intValue();

    int nHonSPs=this->getParentModule()->par("n_SH").intValue();
    int nMalSPs=this->getParentModule()->par("n_SM").intValue();

    double perReqThroughput=this->par("reqThroughput").doubleValue();
    double perProvTargetLoad=this->par("cuckooLoad").doubleValue();

    std::vector<double> reqRemainingLoad(nMalReqs,perReqThroughput);
    std::vector<double> provRemainingLoad(nHonSPs, perProvTargetLoad);

    std::vector<std::vector<double>> reqProvLoads(nMalReqs, std::vector<double>(nHonSPs, 0));

    std::vector<std::vector<simtime_t>> reqProvLats(nMalReqs, std::vector<simtime_t>(nHonSPs, SIMTIME_MAX));
    std::vector<std::pair<simtime_t, std::pair<int, int>>> latsReqProv(nMalReqs*nHonSPs);

    // Retrieve all malReq->honProv Latencies
    for(int i=0; i<nMalReqs; i++)
    {
        cModule* source = this->getParentModule()->getSubmodule("sources", i+nHonReqs);
        for(int j=0; j<nHonSPs;j++)
        {
            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(source->gate("out", j)->getChannel());
            reqProvLats[i][j]=outChan->getDelay();
            latsReqProv[i*nHonSPs+j]=std::pair<simtime_t, std::pair<int, int>>(outChan->getDelay(),std::pair<int,int>(i,j));
        }
    }
    for(int i=0; i<nMalReqs; i++)
    {
        for(int j=0; j<reqProvLats[i].size();j++)
        {
            std::cout << "[" << i << ";" << j << "]:\t" <<reqProvLats[i][j].dbl()<<" "<< latsReqProv[i*nHonSPs+j].first.dbl() << " [" << latsReqProv[i*nHonSPs+j].second.first << ";" << latsReqProv[i*nHonSPs+j].second.second << "]" << std::endl;
            assert(i==latsReqProv[i*nHonSPs+j].second.first);
            assert(j==latsReqProv[i*nHonSPs+j].second.second);
            assert(reqProvLats[i][j].dbl()==latsReqProv[i*nHonSPs+j].first.dbl());
        }
    }

    //sort by malReq-honProv pairs by increasing net latencies
    std::stable_sort(latsReqProv.begin(), latsReqProv.end(), [](auto &left, auto &right) {
        return left.first < right.first;
    });
    //assign load to close providers first
    for(auto latReqProv: latsReqProv)
    {
        int currReq= latReqProv.second.first;
        int currProv= latReqProv.second.second;
        std::cout << latReqProv.first << "\t["<<currReq<<";"<<currProv<<"]"<<std::endl;
        if(provRemainingLoad[currProv]>0)
        {
            double addedLoad=std::min(provRemainingLoad[currProv],reqRemainingLoad[currReq]);
            provRemainingLoad[currProv]-=addedLoad;
            reqRemainingLoad[currReq]-=addedLoad;
            reqProvLoads[currReq][currProv]+=addedLoad;
        }
    }
    std::cout<<"Provider remaining load"<<std::endl;
    for(auto load: provRemainingLoad)
    {
        std::cout<<load<<" ";
    }
    std::cout<<std::endl;
    std::cout<<"Requester remaining load"<<std::endl;
    for(auto load: reqRemainingLoad)
    {
        std::cout<<load<<" ";
    }
    std::cout<<std::endl;
    for(int i=0; i<nMalReqs;i++)
    {
        MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", i+nHonReqs));
        for(int j=0; j<nHonSPs;j++)
        {
            sink->providerRatios[j]=reqProvLoads[i][j]/perReqThroughput;
        }
    }
    for(int i=0; i<nMalReqs;i++)
    {
        MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", i+nHonReqs));
        for(int j=0; j<nMalSPs;j++)
        {
            sink->providerRatios[nHonSPs+j]=reqRemainingLoad[i]/(nMalSPs*perReqThroughput);
        }
    }
    for(int i=0; i<nMalReqs;i++)
    {
        MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", i+nHonReqs));
        for(int j=0; j<nHonSPs+nMalSPs;j++)
        {
            std::cout<<sink->providerRatios[j]<<" ";
        }
        std::cout<<std::endl;
    }
}
