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

package cooltee.simulations;

import cooltee.MultiSink;
import cooltee.LOolSource;
import cooltee.IMultiSource;
import cooltee.AggFifo;
import cooltee.Environment;
network TwoSpeedService
{
    parameters:
        int n_sources = default(10);
        int n_fifos = default(3);
        double ratioRHM;
        double ratioSHM;
        double ratioSR;
        string req_type;
        double warmup @unit(s);
        
        // dependent variables
        int n_RH = int(ceil(n_sources*ratioRHM));
        int n_RM = int(floor(n_sources*(1-ratioRHM)));
        int n_SH = int(ceil(n_fifos*ratioSHM));
        int n_SM = int(floor(n_fifos*(1-ratioSHM)));
    types:
        channel malChan extends ned.DelayChannel
        {
            @display("ls=red");
            delay = 0ms;
        }
        channel collChan extends ned.DelayChannel
        {
            @display("ls=darkorange");
            delay = 0ms;
        }
        channel honChan extends ned.DelayChannel
        {
            @display("ls=black");
            delay = 0ms;
        }
    submodules:
        sources[n_RH+n_RM]: <req_type> like IMultiSource {
            @display("p=100,100,c,100,;t=,l,");
        }
        sinks[n_RH+n_RM]: MultiSink {
            @display("p=500,100,c,100,;t=,r,");
        }
        fifos[n_SH]: AggFifo {
            @display("p=300,250,c,125,");
        }
        malSPs[n_SM]: AggFifo {
            @display("p=300,900,c,125,");
            offset=parent.n_SH;
        }
        env: Environment { //HardCodedEnv
            @display("p=300,100");
        }
    connections:
        for i=0..n_RH-1, for j=0..sizeof(fifos)-1 {
            sources[i].out++ --> honChan --> fifos[j].in++;
        }
        for i=0..n_RH-1, for j=0..sizeof(malSPs)-1 {
            sources[i].out++ --> honChan --> malSPs[j].in++;
        }
        for i=n_RH..n_RH+n_RM-1, for j=0..sizeof(fifos)-1 {
            sources[i].out++ --> malChan --> fifos[j].in++;
        }
        for i=n_RH..n_RH+n_RM-1, for j=0..sizeof(malSPs)-1 {
            sources[i].out++ --> collChan --> malSPs[j].in++;
        }
        for i=0..sizeof(sinks)-1, for j=0..sizeof(fifos)-1 {
            fifos[j].out++ --> honChan --> sinks[i].in++;
        }
        for i=0..n_RH-1, for j=0..sizeof(malSPs)-1 {
            malSPs[j].out++ --> malChan --> sinks[i].in++;
        }
        for i=n_RH..n_RH+n_RM-1, for j=0..sizeof(malSPs)-1 {
            malSPs[j].out++ --> collChan --> sinks[i].in++;
        }
}
