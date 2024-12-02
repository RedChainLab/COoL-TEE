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

#include "Environment.h"
#include "utils/utils.h"
#include "MultiSink.h"
#include "MultiSource.h"
#include "math.h"
#include <limits>
Define_Module(Environment);

int Environment::numInitStages() const
{
    return 2;
}

void Environment::initialize(int stage)
{
    if(stage==1)
    {
        assert(this->getParentModule()->getSubmoduleVectorSize("sinks")==this->getAncestorPar("n_sources").intValue());
        assert(this->getParentModule()->getSubmoduleVectorSize("fifos")+this->getParentModule()->getSubmoduleVectorSize("malSPs")==this->getAncestorPar("n_fifos").intValue());
        this->addModules();
        this->setSignals();
        if(this->par("deltaNet").boolValue())
        {
            this->setDelayChannels();
        }
        for(int i=0;i<par("naiveH").intValue();i++)
        {
            this->getParentModule()->getSubmodule("sinks",i)->par("kErr").setDoubleValue(0);
            this->getParentModule()->getSubmodule("sinks",i)->par("kD").setDoubleValue(0);
        }
        for(int i=0;i<par("naiveM").intValue();i++)
        {
            this->getParentModule()->getSubmodule("sinks",this->getParentModule()->par("n_RH").intValue()+i)->par("kErr").setDoubleValue(0);
            this->getParentModule()->getSubmodule("sinks",this->getParentModule()->par("n_RH").intValue()+i)->par("kD").setDoubleValue(0);
        }
        if(this->par("pingStart").boolValue())
        {
            this->assignClosestProviders();
        }
    }
}

void Environment::assignClosestProviders()
{
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("sinks");i++)
    {
        MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", i));
        MultiSource* source = check_and_cast<MultiSource*>(this->getParentModule()->getSubmodule("sources", i));
        std::vector<int> closestProvs=std::vector<int>();
        int closestProv=0;
        double minDelay=std::numeric_limits<double>::infinity();
        for(;closestProv<this->getAncestorPar("n_fifos").intValue();closestProv++)
        {
            sink->providerRatios[closestProv]=0;
            double delay=source->gate("out",closestProv)->getChannel()->par("delay").doubleValue();

            if(delay<minDelay)
            {
                closestProvs.clear();
            }
            if(delay<=minDelay)
            {
                closestProvs.push_back(closestProv);
                minDelay=delay;
            }
        }
        for(int provIdx: closestProvs)
        {
            sink->providerRatios[provIdx]=1./closestProvs.size();
        }
    }
}

void Environment::setDelayChannels()
{
    std::string csv_file=this->par("delays_file").stringValue();
    std::vector<std::vector<double>> matrix = Utils::csv_to_matrix(csv_file);

    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("sources");i++)
    {
        int loc_idx = this->getRNG(0)->intRand(matrix.size());

        cModule* source = this->getParentModule()->getSubmodule("sources", i);
        cModule* sink = this->getParentModule()->getSubmodule("sinks", i);
        source->par("location_index").setIntValue(loc_idx);
        sink->par("location_index").setIntValue(loc_idx);
    }
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("fifos");i++)
    {
        int loc_idx = this->getRNG(0)->intRand(matrix.size());

        cModule* fifo = this->getParentModule()->getSubmodule("fifos", i);
        fifo->par("location_index").setIntValue(loc_idx);

        for(int j=0; j<this->getParentModule()->getSubmoduleVectorSize("sinks");j++)
        {
            MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));

            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate("out", j)->getChannel());

            double delay2Sink=matrix[fifo->par("location_index").intValue()][sink->par("location_index").intValue()];

            outChan->setDelay(delay2Sink/1000);
        }
    }
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("malSPs");i++)
    {
        int loc_idx = this->getRNG(0)->intRand(matrix.size());

        cModule* fifo = this->getParentModule()->getSubmodule("malSPs", i);
        fifo->par("location_index").setIntValue(loc_idx);
        int g_size=this->getParentModule()->par("n_RH").intValue()+this->getParentModule()->par("n_RM").intValue();
        for(int j=0; j<g_size;j++)
        {
            MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));

            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate("out", j)->getChannel());

            double delay2Sink=matrix[fifo->par("location_index").intValue()][sink->par("location_index").intValue()];

            outChan->setDelay(delay2Sink/1000);
        }
        double addedWait= par("hon_wait_out").doubleValue();
        for(int j=0; j<this->getParentModule()->par("n_RH").intValue();j++)
        {
            MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));

            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate("out", j)->getChannel());

            double delay2Sink=matrix[fifo->par("location_index").intValue()][sink->par("location_index").intValue()]+addedWait;

            outChan->setDelay(std::max(0.0,delay2Sink/1000));
        }
    }
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("sources");i++)
    {
        cModule* source = this->getParentModule()->getSubmodule("sources", i);

        for(int j=0; j<this->getParentModule()->getSubmoduleVectorSize("fifos");j++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("fifos", j);

            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(source->gate("out", j)->getChannel());

            double delay2Sink=matrix[source->par("location_index").intValue()][fifo->par("location_index").intValue()];

            outChan->setDelay(delay2Sink/1000);
        }
        for(int j=0; j<this->getParentModule()->getSubmoduleVectorSize("malSPs");j++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("malSPs", j);

            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(source->gate("out", this->getParentModule()->getSubmoduleVectorSize("fifos")+j)->getChannel());

            double delay2Sink=matrix[source->par("location_index").intValue()][fifo->par("location_index").intValue()];

            outChan->setDelay(delay2Sink/1000);
        }
    }
    char* file_name=new char[1000];
    sprintf(file_name,"%s/topology-%s.csv", par("outputPathPrefix").stringValue(), par("runPath").stringValue());
    std::ofstream out;
    out.open(file_name); // append instead of overwrite
    out << this->par("delays_file").stringValue() << endl;
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

void Environment::setSignals()
{
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("fifos");i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule("fifos", i);

        for(int j=0; j<this->getParentModule()->par("n_RH").intValue();j++)
        {
            MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));

            {
                char signalName[32];
                sprintf(signalName, "lifetime-hon-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signal = registerSignal(signalName);

                sink->lifetimeThroughXSignals.push_back(signal);

                char statisticName[32];
                sprintf(statisticName, "lifetime:honest:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplate = sink->getProperties()->get("statisticTemplate", "lifetimeThroughX");
                getEnvir()->addResultRecorders(sink, signal, statisticName, statisticTemplate);
            }
            {
                char signalNameRatio[32];
                sprintf(signalNameRatio, "ratio-hon-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalRatio = registerSignal(signalNameRatio);

                sink->providerRatioSignals.push_back(signalRatio);

                char statisticNameRatio[32];
                sprintf(statisticNameRatio, "ratio:honest:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateRatio = sink->getProperties()->get("statisticTemplate", "providerRatio");
                getEnvir()->addResultRecorders(sink, signalRatio, statisticNameRatio, statisticTemplateRatio);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "dropped-hon-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->droppedSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "dropped:honest:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "droppedJobs");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "debt-hon-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->debtSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "debt:honest:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "providerDebt");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
        }
        for(int j=this->getParentModule()->par("n_RH").intValue(); j<this->getParentModule()->par("n_RH").intValue()+this->getParentModule()->par("n_RM").intValue();j++)
        {
            MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));

            {
                char signalName[32];
                sprintf(signalName, "lifetime-malicious-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signal = registerSignal(signalName);

                sink->lifetimeThroughXSignals.push_back(signal);

                char statisticName[32];
                sprintf(statisticName, "lifetime:mal:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplate = sink->getProperties()->get("statisticTemplate", "lifetimeThroughX");
                getEnvir()->addResultRecorders(sink, signal, statisticName, statisticTemplate);
            }
            {
                char signalNameRatio[32];
                sprintf(signalNameRatio, "ratio-malicious-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalRatio = registerSignal(signalNameRatio);

                sink->providerRatioSignals.push_back(signalRatio);

                char statisticNameRatio[32];
                sprintf(statisticNameRatio, "ratio:mal:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateRatio = sink->getProperties()->get("statisticTemplate", "providerRatio");
                getEnvir()->addResultRecorders(sink, signalRatio, statisticNameRatio, statisticTemplateRatio);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "dropped-malicious-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->droppedSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "dropped:mal:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "droppedJobs");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "debt-malicious-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->debtSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "debt:mal:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "providerDebt");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
        }
    }
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("malSPs");i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule("malSPs", i);

        for(int j=0; j<this->getParentModule()->par("n_RH").intValue();j++)
        {
            MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));
            {
                char signalName[32];
                sprintf(signalName, "lifetime-att-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signal = registerSignal(signalName);

                sink->lifetimeThroughXSignals.push_back(signal);

                char statisticName[32];
                sprintf(statisticName, "lifetime:attacking:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplate = sink->getProperties()->get("statisticTemplate", "lifetimeThroughX");
                getEnvir()->addResultRecorders(sink, signal, statisticName, statisticTemplate);
            }
            {
                char signalNameRatio[32];
                sprintf(signalNameRatio, "ratio-att-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalRatio = registerSignal(signalNameRatio);

                sink->providerRatioSignals.push_back(signalRatio);

                char statisticNameRatio[32];
                sprintf(statisticNameRatio, "ratio:attacking:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateRatio = sink->getProperties()->get("statisticTemplate", "providerRatio");
                getEnvir()->addResultRecorders(sink, signalRatio, statisticNameRatio, statisticTemplateRatio);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "dropped-att-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->droppedSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "dropped:attacking:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "droppedJobs");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "debt-att-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->debtSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "debt:attacking:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "providerDebt");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
        }
        for(int j=this->getParentModule()->par("n_RH").intValue(); j<this->getParentModule()->par("n_RH").intValue()+this->getParentModule()->par("n_RM").intValue();j++)
        {
            MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));
            {
                char signalName[32];
                sprintf(signalName, "lifetime-col-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signal = registerSignal(signalName);

                sink->lifetimeThroughXSignals.push_back(signal);

                char statisticName[32];
                sprintf(statisticName, "lifetime:colluding:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplate = sink->getProperties()->get("statisticTemplate", "lifetimeThroughX");
                getEnvir()->addResultRecorders(sink, signal, statisticName, statisticTemplate);
            }
            {
                char signalNameRatio[32];
                sprintf(signalNameRatio, "ratio-col-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalRatio = registerSignal(signalNameRatio);

                sink->providerRatioSignals.push_back(signalRatio);

                char statisticNameRatio[32];
                sprintf(statisticNameRatio, "ratio:colluding:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateRatio = sink->getProperties()->get("statisticTemplate", "providerRatio");
                getEnvir()->addResultRecorders(sink, signalRatio, statisticNameRatio, statisticTemplateRatio);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "dropped-col-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->droppedSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "dropped:colluding:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "droppedJobs");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
            {
                char signalNameDropped[32];
                sprintf(signalNameDropped, "debt-col-%d-%d", fifo->getIndex(), sink->getIndex());
                simsignal_t signalDropped = registerSignal(signalNameDropped);

                sink->debtSignals.push_back(signalDropped);

                char statisticNameDropped[32];
                sprintf(statisticNameDropped, "debt:colluding:%d:%d", fifo->getIndex(), sink->getIndex());
                cProperty *statisticTemplateDropped = sink->getProperties()->get("statisticTemplate", "providerDebt");
                getEnvir()->addResultRecorders(sink, signalDropped, statisticNameDropped, statisticTemplateDropped);
            }
        }
    }
    for(int j=0; j<this->getParentModule()->par("n_RH").intValue();j++)
    {
        MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));
        char signalName[32];
        sprintf(signalName, "firstLifetime-hon-%d", sink->getIndex());

        simsignal_t signal = registerSignal(signalName);
        sink->lifetimeFirstRespSignal = signal;

        char statisticName[32];
        sprintf(statisticName, "firstlifetime:hon:%d", sink->getIndex());
        cProperty *statisticTemplate = sink->getProperties()->get("statisticTemplate", "lifetimeFirstRespSignal");
        getEnvir()->addResultRecorders(sink, signal, statisticName, statisticTemplate);
    }
    for(int j=this->getParentModule()->par("n_RH").intValue(); j<this->getParentModule()->par("n_RH").intValue()+this->getParentModule()->par("n_RM").intValue();j++)
    {
        MultiSink* sink = check_and_cast<MultiSink*>(this->getParentModule()->getSubmodule("sinks", j));
        char signalName[32];
        sprintf(signalName, "firstLifetime-mal-%d", sink->getIndex());

        simsignal_t signal = registerSignal(signalName);
        sink->lifetimeFirstRespSignal = signal;

        char statisticName[32];
        sprintf(statisticName, "firstlifetime:mal:%d", sink->getIndex());
        cProperty *statisticTemplate = sink->getProperties()->get("statisticTemplate", "lifetimeFirstRespSignal");
        getEnvir()->addResultRecorders(sink, signal, statisticName, statisticTemplate);
    }
    for(int j=0; j<this->getParentModule()->par("n_RH").intValue();j++)
    {
        MultiSource* source = check_and_cast<MultiSource*>(this->getParentModule()->getSubmodule("sources", j));
        for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("fifos");i++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("fifos", i);
            char signalName[32];
            sprintf(signalName, "sentMsg-hon-%d-%d", fifo->getIndex(), source->getIndex());

            simsignal_t signal = registerSignal(signalName);
            source->sentMsgSignal.push_back(signal);

            char statisticName[32];
            sprintf(statisticName, "sentMsg:hon:%d:%d", fifo->getIndex(), source->getIndex());
            cProperty *statisticTemplate = source->getProperties()->get("statisticTemplate", "sentMsg");
            getEnvir()->addResultRecorders(source, signal, statisticName, statisticTemplate);
        }
        for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("malSPs");i++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("malSPs", i);
            char signalName[32];
            sprintf(signalName, "sentMsg-att-%d-%d", fifo->getIndex(), source->getIndex());

            simsignal_t signal = registerSignal(signalName);
            source->sentMsgSignal.push_back(signal);

            char statisticName[32];
            sprintf(statisticName, "sentMsg:att:%d:%d", fifo->getIndex(), source->getIndex());
            cProperty *statisticTemplate = source->getProperties()->get("statisticTemplate", "sentMsg");
            getEnvir()->addResultRecorders(source, signal, statisticName, statisticTemplate);
        }
    }
    for(int j=this->getParentModule()->par("n_RH").intValue(); j<this->getParentModule()->par("n_RH").intValue()+this->getParentModule()->par("n_RM").intValue();j++)
    {
        MultiSource* source = check_and_cast<MultiSource*>(this->getParentModule()->getSubmodule("sources", j));
        for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("fifos");i++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("fifos", i);
            char signalName[32];
            sprintf(signalName, "sentMsg-mal-%d-%d", fifo->getIndex(), source->getIndex());

            simsignal_t signal = registerSignal(signalName);
            source->sentMsgSignal.push_back(signal);

            char statisticName[32];
            sprintf(statisticName, "sentMsg:mal:%d:%d", fifo->getIndex(), source->getIndex());
            cProperty *statisticTemplate = source->getProperties()->get("statisticTemplate", "sentMsg");
            getEnvir()->addResultRecorders(source, signal, statisticName, statisticTemplate);
        }
        for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize("malSPs");i++)
        {
            cModule* fifo = this->getParentModule()->getSubmodule("malSPs", i);
            char signalName[32];
            sprintf(signalName, "sentMsg-col-%d-%d", fifo->getIndex(), source->getIndex());

            simsignal_t signal = registerSignal(signalName);
            source->sentMsgSignal.push_back(signal);

            char statisticName[32];
            sprintf(statisticName, "sentMsg:col:%d:%d", fifo->getIndex(), source->getIndex());
            cProperty *statisticTemplate = source->getProperties()->get("statisticTemplate", "sentMsg");
            getEnvir()->addResultRecorders(source, signal, statisticName, statisticTemplate);
        }
    }
}

void Environment::addModules()
{

}
