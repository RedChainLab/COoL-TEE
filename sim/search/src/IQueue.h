/*
 * OracleQueue.h
 *
 *  Created on: 29 juin 2023
 *      Author: DrilV
 */

#ifndef IQUEUE_H_
#define IQUEUE_H_

#include <omnetpp.h>
#include "SourcedMsg_m.h"

using namespace omnetpp;

class IQueue {
    virtual cMessage* front(cQueue q)=0;
    virtual bool isEmpty(cQueue q)=0;
    virtual void handled(SourcedMsg*)=0;
};

#endif /* IQUEUE_H_ */
