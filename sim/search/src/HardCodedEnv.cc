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

#include "HardCodedEnv.h"

Define_Module(HardCodedEnv);

void HardCodedEnv::setChannels(const char* main, const char* gate, Vector* vec, int offset, double added_wait)
{
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize(main);i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule(main, i);

        for(int j=0; j<fifo->gateSize(gate);j++)
        {
            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate(gate, j)->getChannel());

            double delay2Sink=vec->getVal(i+offset);

            outChan->setDelay((added_wait+delay2Sink)/1000);
        }
    }
}

void HardCodedEnv::setChannels(const char* main, const char* gate, Vector* vec, int destStartIdx, int nbDest, int offset, double added_wait)
{
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize(main);i++)
    {
        cModule* fifo = this->getParentModule()->getSubmodule(main, i);

        for(int j=destStartIdx; j<destStartIdx+nbDest;j++)
        {
            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(fifo->gate(gate, j)->getChannel());

            double delay2Sink=vec->getVal(i+offset);

            outChan->setDelay((added_wait+delay2Sink)/1000);
        }
    }
}

void HardCodedEnv::setChannels(const char* owner, const char* target, const char* gate, Vector* vec, int offset)
{
    for(int i=0; i<this->getParentModule()->getSubmoduleVectorSize(owner);i++)
    {
        cModule* source = this->getParentModule()->getSubmodule(owner, i);

        for(int j=0; j<this->getParentModule()->getSubmoduleVectorSize(target);j++)
        {
            cDelayChannel* outChan=check_and_cast<cDelayChannel*>(source->gate(gate, j+offset)->getChannel());

            double delay2Sink=vec->getVal(j+offset);

            outChan->setDelay(delay2Sink/1000);
        }
    }
}

void HardCodedEnv::setDelayChannels()
{
    Vector* vec = (Vector*)par("net_delays").objectValue(); // e.g. "aa bb cc";
    double addedWait= par("hon_wait_out").doubleValue();

    setChannels("fifos", "out", vec);
    setChannels("sources","fifos", "out", vec);
    setChannels("malSPs", "out", vec, this->getParentModule()->getSubmoduleVectorSize("fifos"));//mal_out
    setChannels("malSPs", "out", vec, 0, this->getParentModule()->par("n_RH").intValue(),this->getParentModule()->getSubmoduleVectorSize("fifos"),addedWait);
    setChannels("sources","malSPs", "out", vec, this->getParentModule()->getSubmoduleVectorSize("fifos"));

}


