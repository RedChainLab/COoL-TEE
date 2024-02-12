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

void MultiSink::initialize()
{
    Sink::initialize();

    updateRatiosCount=par("updateRatiosCount");
    x1=par("x1");

    int nSPs = this->getParentModule()->getSubmoduleVectorSize("fifos")+this->getParentModule()->getSubmoduleVectorSize("malSPs");

    r3 = std::vector<double>(nSPs,1/double(nSPs));
    providerAvgLatency = std::vector<simtime_t>(nSPs,SIMTIME_ZERO);
    providerPrevAvgRelLatency = std::vector<double>(nSPs,0);
    providerRatios = std::vector<double>(nSPs,1/double(nSPs));
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
#if LOG
    EV<<"AvgLat ";
    std::copy(providerAvgLatency.begin(), providerAvgLatency.end(), std::ostream_iterator<simtime_t>(EV, " "));
    EV<<std::endl;
#endif
    int windowSize=par("windowSize").intValue();//100;
    auto itBegin = providerAvgLatency.size()>=windowSize?providerAvgLatency.end()-9:providerAvgLatency.begin();
    simtime_t sum = std::accumulate(itBegin, providerAvgLatency.end(), SIMTIME_ZERO);
    simtime_t Dt = sum / std::min(int(providerAvgLatency.size()),windowSize);//providerAvgLatency.size();
#if LOG
    EV<<"AvgLat Avg "<<Dt<<std::endl;

    EV<<"Drop ";
#endif
    simtime_t dropTimeWindow=simTime()-SimTime(60);

    std::vector<int> dropped = std::vector<int>(nSPs,0);
    int sumDropped =0;
    int i=0;
    for (auto it=droppedJobs.cbegin();it!=droppedJobs.cend();it++)
    {
        auto begin = dichotomic_search(it->cbegin(),it->cend(),dropTimeWindow);
        sumDropped+=std::distance(begin, it->cend());
        dropped[i]=std::distance(begin, it->cend());
        EV<<dropped[i]<<" ";
        i++;
    }
#if LOG
    EV<<std::endl;
#endif
    double avgDropped = sumDropped / double(droppedJobs.size());
#if LOG
    EV<<"Drop Avg "<<avgDropped<<std::endl;
#endif
    double kErr=par("kErr").doubleValue();
    double kD=par("kD").doubleValue();
    double dropSens=par("dropSens").doubleValue();
    for(int i=0; i<nSPs; i++)
    {
        simtime_t Di = providerAvgLatency[i];
        double dropComponent = (avgDropped>0)?(avgDropped-dropped[i])/avgDropped:0;
#if LOG
        EV<<"dropComponent "<<dropComponent;
#endif
        double Drel = (Dt-Di)/Dt;
        double dRiP=(1-dropSens)*Drel + dropSens*dropComponent;
#if LOG
        EV<<" dRiP "<<dRiP<<std::endl;
#endif
        double dRi=kErr*dRiP;

        static bool firstRun=true;
        if(firstRun)
        {
            firstRun=false;
        }
        else
        {
            double oldDrel=providerPrevAvgRelLatency[i];
            double oldDropComponent = prevDroppedJobsRelAvg[i];
            double dRiD=(1-dropSens)*(Drel-oldDrel) + dropSens*(dropComponent-oldDropComponent);
#if LOG
            EV<<"dRiD"<<i<<" "<<dRiD<<std::endl;
#endif
            dRi+=kD*dRiD;
        }
        providerPrevAvgRelLatency[i]=Drel;
        prevDroppedJobsRelAvg[i]=dropComponent;

        r3[i]=clamp(r3[i]+dRi,0.,1.);
    }
#if LOG
    EV<<"R3s ";
    std::copy(r3.begin(), r3.end(), std::ostream_iterator<double>(EV, " "));
    EV<<std::endl;
#endif
    double sumR3=std::accumulate(r3.begin(), r3.end(), 0.);
#if LOG
    EV<<"Sum R3 "<<sumR3<<std::endl;
#endif
    for(int i=0; i<nSPs; i++)
    {
        r3[i]=r3[i]/sumR3;
        providerRatios[i]=x1*(1/double(nSPs))+(1-x1)*r3[i];
        emit(providerRatioSignals[i],r3[i]);
    }
#if LOG
    EV<<"R3s ";
    std::copy(r3.begin(), r3.end(), std::ostream_iterator<double>(EV, " "));
    EV<<std::endl;
    EV<<"Ratios ";
    std::copy(providerRatios.begin(), providerRatios.end(), std::ostream_iterator<double>(EV, " "));
    EV<<std::endl;
#endif
}
