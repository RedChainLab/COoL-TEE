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

#ifndef __FAIRFETCHED_MULTISINK_H_
#define __FAIRFETCHED_MULTISINK_H_

#include <omnetpp.h>
#include "Sink.h"

#include <vector>
#include <map>

#include <fstream>

using namespace omnetpp;

/**
 * TODO - Generated class
 */
class MultiSink : public Sink
{
  public:
    std::vector<std::vector<simtime_t>> providerLatencies;
    std::vector<simtime_t> providerAvgLatency;
    std::vector<double> providerPrevAvgRelLatency;

    std::vector<double> providerRatios; // x1*r1+x3*r3
    std::vector<double> r3; // eXploit
    double x1; // eXplore ratio
    int updateRatiosCount;
    std::vector<std::vector<simtime_t>> droppedJobs;
    std::vector<double> droppedJobsAvg;
    std::vector<double> prevDroppedJobsRelAvg;
    std::vector<int> providerDebt;

    simsignal_t lifetimeFirstRespSignal;
    std::vector<simsignal_t> lifetimeThroughXSignals;
    std::vector<simsignal_t> providerRatioSignals;
    std::vector<simsignal_t> droppedSignals;
    std::vector<simsignal_t> debtSignals;

    std::map<int, int> pendingRequests;
    std::vector<int> droppedRequests;
    std::vector<int> sentRequests;
    int totalRequests;

    static std::ofstream* outfileAll;
    static int pseudoMutex;

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    virtual void updateRatios();
};

#endif
