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

#ifndef __FAIRFETCHED_OMNETPP_HARDCODEDENV_H_
#define __FAIRFETCHED_OMNETPP_HARDCODEDENV_H_

#include <omnetpp.h>

#include "Environment.h"
#include "Vector_m.h"
using namespace omnetpp;

/**
 * TODO - Generated class
 */
class HardCodedEnv : public Environment
{
  protected:
    virtual void setDelayChannels() override;
    virtual void setChannels(const char* main, const char* gate, Vector* vec, int offset=0, double added_wait=0);
    virtual void setChannels(const char* source, const char* target, const char* gate, Vector* vec, int offset=0);
    virtual void setChannels(const char* main, const char* gate, Vector* vec, int destStartIdx, int nbDest, int offset=0, double added_wait=0);
};

#endif
