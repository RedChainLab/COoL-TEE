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

package cooltee;

//
// TODO auto-generated module
//
simple MultiSink extends AbstractSink
{
    parameters:
    	@class(MultiSink);
        int location_index @mutable;
    	int updateRatiosCount=default(1);
    	double x1;
    	double kErr @mutable=default(0);
    	double kD @mutable=default(0);
    	int windowSize;
    	double dropSens;
    	bool hardcodedRatios=default(false);
    	object hardcodedRatiosVec=default(nullptr);
    	
    	bool t4ctful=default(false);
    	
    	string outputPathPrefix;
    	string runPath;
    	
        @signal[lifetime-*](type=simtime_t); // note the wildcard
		@statisticTemplate[lifetimeThroughX](title="lifetime of arrived jobs from $namePart2 SP #$namePart3 to Sink $namePart4"; unit=s; record=vector,mean,max; interpolationmode=none);
		
		@signal[firstLifetime-*](type=simtime_t); // note the wildcard
		@statisticTemplate[lifetimeFirstRespSignal](title="lifetime of first arrived jobs to $namePart2 Sink $namePart3"; unit=s; record=vector,mean,max; interpolationmode=none);
		
		@signal[ratio-*](type=double); // note the wildcard
		@statisticTemplate[providerRatio](title="Share of sent jobs to $namePart2 SP #$namePart3 from Source $namePart4"; record=vector,mean,max; interpolationmode=none);
		
		@signal[dropped-*](type=bool); // note the wildcard
		@statisticTemplate[droppedJobs](title="Dropped jobs sent to $namePart2 SP #$namePart3 from Source $namePart4"; record=vector,sum, mean; interpolationmode=none);
		
		@signal[debt-*](type=int); // note the wildcard
		@statisticTemplate[providerDebt](title="Job debt of $namePart2 SP #$namePart3 wrt. Source $namePart4"; record=vector, mean, max; interpolationmode=sample-hold);
    gates:
        input in[];
}
