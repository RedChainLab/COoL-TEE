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

#include "LOolSource.h"
#include "MultiSink.h"

#include <iostream>
#include <iomanip>
#include <sstream>

Define_Module(LOolSource);

int LOolSource::chooseProvider()
{
    int choice=0;
    double random = uniform(0, 1);
    std::vector<double> providerRatios = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks",getIndex()))->providerRatios;

    double threshold=0;
    for(int i=0;i<providerRatios.size();i++)
    {
        if(threshold<=random && random <= threshold+providerRatios[i])
        {
            choice=i;
            break;
        }
        threshold+=providerRatios[i];
    }
    return choice;
}

void LOolSource::setDisplayTags()
{
    MultiSource::setDisplayTags();
    std::vector<double> providerRatios = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks",getIndex()))->providerRatios;

    std::stringstream ss;
    for(auto it=providerRatios.cbegin();it!=providerRatios.cend();it++)
    {
        ss << std::setw(6) << std::setfill(' ') << std::setprecision(4);
        ss << *it/par("interarrivalMeanTime").doubleValue()<<"rq/s ("<<*it*100<<"%)";
        if(it+1!=providerRatios.cend())
        {
            ss << " ";
        }
    }

    this->getDisplayString().setTagArg("t", 0, ss.str().c_str());
}
