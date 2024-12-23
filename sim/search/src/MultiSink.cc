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

#include "MultiSink.h"

Define_Module(MultiSink);

#include "SourcedMsg_m.h"
#include "DeadlineMsg_m.h"

#include<numeric>
#include<algorithm>
#include <iterator>

#include "Vector_m.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
/*void MultiSink::initialize()
{
    // TODO - Generated method body
}*/

#define LOG 0

std::ofstream* MultiSink::outfileAll=nullptr;
int MultiSink::pseudoMutex=-1;

void MultiSink::handleMessage(cMessage *msg)
{
    SourcedMsg* smsg=check_and_cast<SourcedMsg *>(msg);
    // Set reception time in msg
    smsg->setTime_recv(simTime());
    simtime_t lifetime = smsg->getTime_recv() - smsg->getTime_send();
    if(this->par("t4ctful").boolValue())
    {
        lifetime-=0.999*(smsg->getTime_serv()-smsg->getTime_in());
    }

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

void MultiSink::initialize()
{
    Sink::initialize();

    updateRatiosCount=par("updateRatiosCount");
    x1=par("x1");
    cluster_thresh=par("cluster_thresh");
    cluster_attrition=par("cluster_attrition");

    int nSPs = this->getParentModule()->getSubmoduleVectorSize("fifos")+this->getParentModule()->getSubmoduleVectorSize("malSPs");

    r3 = std::vector<double>(nSPs,1/double(nSPs));
    providerAvgLatency = std::vector<simtime_t>(nSPs,SimTime(-1));
    providerPrevAvgRelLatency = std::vector<double>(nSPs,0);
    providerRatios = std::vector<double>(nSPs,1/double(nSPs));
    if(this->par("hardcodedRatios").boolValue())
    {
        Vector* vec = (Vector*)par("hardcodedRatiosVec").objectValue(); // e.g. "aa bb cc";
        if(vec != nullptr)
        {
            for(int i=0; i<nSPs; i++)
            {
                providerRatios[i]=vec->getVal(i);
            }
        }
    }
    providerLatencies= std::vector<std::vector<simtime_t>>(nSPs,std::vector<simtime_t>());
    droppedJobs=std::vector<std::vector<simtime_t>>(nSPs,std::vector<simtime_t>());
    droppedJobsAvg = std::vector<double>(nSPs,0);
    prevDroppedJobsRelAvg = std::vector<double>(nSPs,0);
    providerDebt = std::vector<int>(nSPs,0);

    assert(providerLatencies.size()==nSPs);
    assert(providerAvgLatency.size()==nSPs);
    assert(providerRatios.size()==nSPs);
    assert(r3.size()==nSPs);
    assert(droppedJobs.size()==nSPs);
    assert(droppedJobsAvg.size()==nSPs);
    assert(providerDebt.size()==nSPs);

    pendingRequests=std::map<int,int>();
    sentRequests=std::vector<int>(nSPs,0);
    droppedRequests=std::vector<int>(nSPs,0);
    totalRequests=0;

    if(outfileAll == nullptr)
    {
        pseudoMutex=this->getVectorSize();

        char* file_name=new char[1000];
        sprintf(file_name,"%s/all/all-%s.csv", par("outputPathPrefix").stringValue(), par("runPath").stringValue());
        outfileAll=new std::ofstream();
        outfileAll->open(file_name, std::ios_base::app); // append instead of overwrite
        delete[] file_name;
    }
}

void MultiSink::finish()
{
    Sink::finish();
    if(outfileAll != nullptr && (--pseudoMutex)==0)
    {
        outfileAll->close();
        delete outfileAll;
    }
}

template<typename T>
constexpr const T& clamp(const T& value, const T& low, const T& high)
{
    return value < low ? low : (value > high ? high : value);
}

template <typename Iterator, typename T>
Iterator dichotomic_search(Iterator start, Iterator end, const T &threshold) {
  Iterator it = end;
  int count = std::distance(start, end);
  int step;
  while (count > 0) {
    it = start;
    step = count / 2;
    std::advance(it, step);
    if (*it <= threshold) {
      start = it;
      ++start;
      count -= step + 1;
    } else {
      count = step;
    }
  }
  return start;
}

void MultiSink::updateRatios()
{
    int nSPs = this->getParentModule()->getSubmoduleVectorSize("fifos")+this->getParentModule()->getSubmoduleVectorSize("malSPs");
    int nHonSPs = this->getParentModule()->getSubmoduleVectorSize("fifos");

    simtime_t bestLatency=SIMTIME_MAX;
    bool shouldOnlyUpdateMalSPs=this->par("hardcodedRatios").boolValue()
                                && this->getIndex() >= (int)(this->getParentModule()->getSubmoduleVectorSize("sources")*this->getAncestorPar("ratioRHM").doubleValue());

    for(int i=shouldOnlyUpdateMalSPs?nHonSPs:0; i<nSPs; i++)
    {
        if(providerAvgLatency[i]>SIMTIME_ZERO && providerAvgLatency[i]<bestLatency)
        {
            bestLatency=providerAvgLatency[i];
        }
    }
    if(bestLatency==SIMTIME_MAX)
    {
        return;
    }

    std::vector<int> bestProviders;
    std::vector<int> outlierProviders;
    for(int i=shouldOnlyUpdateMalSPs?nHonSPs:0; i<nSPs; i++)
    {
        if(providerAvgLatency[i]>SIMTIME_ZERO && providerAvgLatency[i]<=bestLatency+cluster_thresh)
        {
            bestProviders.push_back(i);
            //std::cout<<"BEST "<<i<<std::endl;
        }
        else
        {
            outlierProviders.push_back(i);
            //std::cout<<"OUTLIER "<<i<<std::endl;
        }
    }

    int windowSize=par("windowSize").intValue();//100;
    simtime_t sum=SIMTIME_ZERO;
    for (int provider:bestProviders)
    {
        simtime_t prov_sum=SIMTIME_ZERO;
        if(providerLatencies[provider].size()>=windowSize)
        {
            prov_sum=std::accumulate(providerLatencies[provider].end()-windowSize, providerLatencies[provider].end(), SIMTIME_ZERO);
        }
        else
        {
            prov_sum=std::accumulate(providerLatencies[provider].begin(), providerLatencies[provider].end(), SIMTIME_ZERO);
        }
        prov_sum=prov_sum/double(std::min(windowSize,int(providerLatencies[provider].size())));
        sum+=prov_sum;
    }
    simtime_t Dt=sum/double(bestProviders.size());

    simtime_t dropTimeWindow=simTime()-SimTime(60);
    std::vector<int> dropped = std::vector<int>(nSPs,0);
    int sumDropped =0;
    int i=0;
    for (int provider:bestProviders)
    {
        auto begin = dichotomic_search(droppedJobs[provider].cbegin(),droppedJobs[provider].cend(),dropTimeWindow);
        sumDropped+=std::distance(begin, droppedJobs[provider].cend());
        dropped[i]=std::distance(begin, droppedJobs[provider].cend());
        EV<<dropped[i]<<" ";
        i++;
    }
    double avgDropped = sumDropped / double(bestProviders.size());

    double kErr=par("kErr").doubleValue();
    double kD=par("kD").doubleValue();
    double dropSens=par("dropSens").doubleValue();
    for(int i:bestProviders)
    {
        double dropComponent = (avgDropped>0)?(avgDropped-dropped[i])/avgDropped:0;

        simtime_t Di = providerAvgLatency[i];
        double Drel = (Dt-Di)/Dt;
        double dRiP=(1-dropSens)*Drel + dropSens*dropComponent;

        double dRi=kErr*dRiP;

        double oldDrel=providerPrevAvgRelLatency[i];
        double oldDropComponent = prevDroppedJobsRelAvg[i];
        double dRiD=(1-dropSens)*(Drel-oldDrel) + dropSens*(dropComponent-oldDropComponent);

        dRi+=kD*dRiD;

        providerPrevAvgRelLatency[i]=Drel;
        prevDroppedJobsRelAvg[i]=dropComponent;

        r3[i]=clamp(r3[i]+dRi,0.,1.);
    }

    double sum_best_r3=0;
    for(int i:bestProviders)
    {
        sum_best_r3+=r3[i];
    }
    double sum_outlier_r3=0;
    for(int i:outlierProviders)
    {
        r3[i]*=(1-cluster_attrition);
        sum_outlier_r3+=r3[i];
    }
    if(shouldOnlyUpdateMalSPs)
    {
        for(int i=0; i<nHonSPs;i++)
        {
            sum_outlier_r3+=r3[i];
        }
    }
    for(int i:bestProviders)
    {
        r3[i]=(1-sum_outlier_r3)*r3[i]/sum_best_r3;
    }

    for(int i=0; i<nSPs; i++)
    {
        providerRatios[i]=x1*(1/double(nSPs))+(1-x1)*r3[i];
        emit(providerRatioSignals[i],r3[i]);
        //std::cout<<"RATIO "<<i<<" "<<providerRatios[i]<<std::endl;
    }
}
