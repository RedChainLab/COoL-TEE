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

#include "AtlasEnv.h"
#include "utils/utils.h"
#include "MultiSink.h"
#include "MultiSource.h"
#include "math.h"
#include <limits>

Define_Module(AtlasEnv);

void AtlasEnv::setDelayChannels()
{
    IntVector* hon_rq_count_per_zone = (IntVector*)par("hon_rq_count_per_zone").objectValue(); // e.g. "aa bb cc";
    IntVector* mal_rq_count_per_zone = (IntVector*)par("mal_rq_count_per_zone").objectValue(); // e.g. "aa bb cc";
    IntVector* hon_prov_ids = (IntVector*)par("hon_prov_ids").objectValue(); // e.g. "aa bb cc";
    IntVector* mal_prov_ids = (IntVector*)par("mal_prov_ids").objectValue(); // e.g. "aa bb cc";

    int count_requesters=0;
    for(int i=0; i<hon_rq_count_per_zone->getValArraySize(); i++)
    {
        count_requesters+=hon_rq_count_per_zone->getVal(i);
    }
    for(int i=0; i<mal_rq_count_per_zone->getValArraySize(); i++)
    {
        count_requesters+=mal_rq_count_per_zone->getVal(i);
    }
    assert((count_requesters,this->getParentModule()->getSubmoduleVectorSize("sources"),count_requesters==this->getParentModule()->getSubmoduleVectorSize("sources")));

    assert((hon_prov_ids->getValArraySize(),this->getParentModule()->getSubmoduleVectorSize("fifos"),hon_prov_ids->getValArraySize()==this->getParentModule()->getSubmoduleVectorSize("fifos")));

    assert((mal_prov_ids->getValArraySize(),this->getParentModule()->getSubmoduleVectorSize("malSPs"),mal_prov_ids->getValArraySize()==this->getParentModule()->getSubmoduleVectorSize("malSPs")));

    std::string csv_file=this->par("delays_file").stringValue();
    std::string locations[]={std::string("EU"),std::string("NA"),std::string("SA"),std::string("As"),std::string("Oc")};

    for(int i=0; i<hon_prov_ids->getValArraySize();i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule("fifos", i);
        fifo->par("location_index").setIntValue(hon_prov_ids->getVal(i));
    }
    for(int i=0; i<mal_prov_ids->getValArraySize();i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule("malSPs", i);
        fifo->par("location_index").setIntValue(mal_prov_ids->getVal(i));
    }

    int positioned_hon_rq=0;
    int positioned_mal_rq=0;
    int zone_idx=0;
    for(std:: string location: locations)
    {
        std::vector<std::vector<double>> matrix = Utils::csv_to_matrix(csv_file+location+std::string(".csv"), true, true);
        for(int i=positioned_hon_rq; i<positioned_hon_rq+hon_rq_count_per_zone->getVal(zone_idx);i++)
        {
            int loc_idx = this->getRNG(0)->intRand(matrix.size());
            cModule* source = this->getParentModule()->getSubmodule("sources", i);
            cModule* sink = this->getParentModule()->getSubmodule("sinks", i);
            source->par("location_index").setIntValue(loc_idx);
            sink->par("location_index").setIntValue(loc_idx);
        }
        positioned_hon_rq+=hon_rq_count_per_zone->getVal(zone_idx);

        zone_idx++;
    }
    zone_idx=0;
    for(std:: string location: locations)
    {
        std::vector<std::vector<double>> matrix = Utils::csv_to_matrix(csv_file+location+std::string(".csv"), true, true);

        for(int i=positioned_mal_rq; i<positioned_mal_rq+mal_rq_count_per_zone->getVal(zone_idx);i++)
        {
            int loc_idx = this->getRNG(0)->intRand(matrix.size());

            cModule* source = this->getParentModule()->getSubmodule("sources", positioned_hon_rq + i);
            cModule* sink = this->getParentModule()->getSubmodule("sinks", positioned_hon_rq + i);
            source->par("location_index").setIntValue(loc_idx);
            sink->par("location_index").setIntValue(loc_idx);
        }
        positioned_mal_rq+=mal_rq_count_per_zone->getVal(zone_idx);
        zone_idx++;
    }

    zone_idx=0;
    int connected_hon_rq=0;
    int connected_mal_rq=0;
    for(std:: string location: locations)
    {
        std::vector<std::vector<double>> matrix = Utils::csv_to_matrix(csv_file+location+std::string(".csv"), true, true);
        std::cout<<"LOCATION: "<<location<<std::endl;
        for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("fifos");i++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("fifos", i);
            for(int j=0; j<hon_rq_count_per_zone->getVal(zone_idx);j++)
            {
                int sink_idx=connected_hon_rq+j;
                std::cout<<"SINK_HON_IDX "<<sink_idx<<"; SIZE "<<this->getParentModule()->getSubmoduleVectorSize("sinks")<<std::endl;
                MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", sink_idx));
                cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate("out", sink_idx)->getChannel());

                cModule* source = this->getParentModule()->getSubmodule("sources", sink_idx);
                cDelayChannel* srcOutChan=check_and_cast<cDelayChannel*>(source->gate("out", i)->getChannel());

                double delay2Sink=matrix[sink->par("location_index").intValue()][fifo->par("location_index").intValue()];

                srcOutChan->setDelay(delay2Sink/1000);
                outChan->setDelay(delay2Sink/1000);
            }

            for(int j=0; j<mal_rq_count_per_zone->getVal(zone_idx);j++)
            {
                int sink_idx=positioned_hon_rq+connected_mal_rq+j;
                std::cout<<"SINK_MAL_IDX "<<sink_idx<<"; SIZE "<<this->getParentModule()->getSubmoduleVectorSize("sinks")<<std::endl;
                MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", sink_idx));
                cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate("out", sink_idx)->getChannel());

                cModule* source = this->getParentModule()->getSubmodule("sources", sink_idx);
                cDelayChannel* srcOutChan=check_and_cast<cDelayChannel*>(source->gate("out", i)->getChannel());

                double delay2Sink=matrix[sink->par("location_index").intValue()][fifo->par("location_index").intValue()];

                srcOutChan->setDelay(delay2Sink/1000);
                outChan->setDelay(delay2Sink/1000);
            }

        }
        connected_hon_rq+=hon_rq_count_per_zone->getVal(zone_idx);
        connected_mal_rq+=mal_rq_count_per_zone->getVal(zone_idx);
        zone_idx++;
    }
    zone_idx=0;
    connected_hon_rq=0;
    connected_mal_rq=0;
    double addedWait= par("hon_wait_out").doubleValue();
    for(std:: string location: locations)
    {
        std::vector<std::vector<double>> matrix = Utils::csv_to_matrix(csv_file+location+std::string(".csv"), true, true);
        std::cout<<"LOCATION: "<<location<<std::endl;
        for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("malSPs");i++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("malSPs", i);
            for(int j=0; j<hon_rq_count_per_zone->getVal(zone_idx);j++)
            {
                int sink_idx=connected_hon_rq+j;
                std::cout<<"SINK_HON_IDX "<<sink_idx<<"; SIZE "<<this->getParentModule()->getSubmoduleVectorSize("sinks")<<std::endl;
                MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", sink_idx));
                cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate("out", sink_idx)->getChannel());

                cModule* source = this->getParentModule()->getSubmodule("sources", sink_idx);
                cDelayChannel* srcOutChan=check_and_cast<cDelayChannel*>(source->gate("out", this->getParentModule()->getSubmoduleVectorSize("fifos")+i)->getChannel());

                double delay2Sink=matrix[sink->par("location_index").intValue()][fifo->par("location_index").intValue()];
                double delay2SinkAtt=matrix[sink->par("location_index").intValue()][fifo->par("location_index").intValue()]+addedWait;

                srcOutChan->setDelay(delay2Sink/1000);
                outChan->setDelay(delay2SinkAtt/1000);
            }
            for(int j=0; j<mal_rq_count_per_zone->getVal(zone_idx);j++)
            {
                int sink_idx=positioned_hon_rq+connected_mal_rq+j;
                std::cout<<"SINK_MAL_IDX "<<sink_idx<<"; SIZE "<<this->getParentModule()->getSubmoduleVectorSize("sinks")<<std::endl;
                MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", sink_idx));
                cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate("out", sink_idx)->getChannel());

                cModule* source = this->getParentModule()->getSubmodule("sources", sink_idx);
                cDelayChannel* srcOutChan=check_and_cast<cDelayChannel*>(source->gate("out", this->getParentModule()->getSubmoduleVectorSize("fifos")+i)->getChannel());

                double delay2Sink=matrix[sink->par("location_index").intValue()][fifo->par("location_index").intValue()];

                srcOutChan->setDelay(delay2Sink/1000);
                outChan->setDelay(delay2Sink/1000);
            }
        }
        connected_hon_rq+=hon_rq_count_per_zone->getVal(zone_idx);
        connected_mal_rq+=mal_rq_count_per_zone->getVal(zone_idx);
        zone_idx++;
    }
    char* file_name=new char[1000];
    sprintf(file_name,"%s/topology-%s.csv", par("outputPathPrefix").stringValue(), par("runPath").stringValue());
    std::ofstream out;
    out.open(file_name); // append instead of overwrite
    out << csv_file+std::string("ALL.csv") << endl;
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("sinks");i++)
    {
        cModule* source = this->getParentModule()->getSubmodule("sinks", i);
        out << "CONSUMER," << source->getId() << "," << source->par("location_index").intValue() << endl;
    }
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("fifos");i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule("fifos", i);
        out << "PROVIDER," << fifo->getId() << "," << fifo->par("location_index").intValue() << endl;
    }
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("malSPs");i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule("malSPs", i);
        out << "PROVIDER," << fifo->getId() << "," << fifo->par("location_index").intValue() << endl;
    }
    out.close();
    delete[] file_name;
}


