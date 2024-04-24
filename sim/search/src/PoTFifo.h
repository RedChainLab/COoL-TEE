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

#ifndef POTFIFO_H_
#define POTFIFO_H_

#include <map>

#include "PoTMsg_m.h"
#include "AggFifo.h"

class PoTFifo: public AggFifo {
public:
    std::map<PoTMsg*, int> potRQqueueLen;
public:
    virtual ~PoTFifo();
    void handleMessage(cMessage*) override;
    virtual void arrival(cMessage *msg) override;
    simtime_t startService(cMessage *msg) override;
    void endService(cMessage *msg) override;
};

#endif /* POTFIFO_H_ */
