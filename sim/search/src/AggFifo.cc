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

#include "AggFifo.h"

Define_Module(AggFifo);

#include "SourcedMsg_m.h"

void AggFifo::endService(cMessage *msg)
{
    SourcedMsg* smsg=check_and_cast<SourcedMsg *>(msg);
    smsg->setTime_out(simTime());
#if LOG
    EV << "Completed service of " << smsg->getName() << " from port " << smsg->getSource() << endl;
#endif
    int destination=smsg->getSource();

    smsg->setSource(this->getIndex()+par("offset").intValue());
    send(smsg, "out", destination);
}
