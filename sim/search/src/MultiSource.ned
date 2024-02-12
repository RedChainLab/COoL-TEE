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

simple MultiSource extends AbstractSource like IMultiSource
{
    parameters:
    	@class(MultiSource);
    	int idx_offset=default(0);
    	string sinkVectorName=default("sinks");
    	int location_index @mutable;
    	int kReqs;
    	double dropWaittime @unit(s) = default(1s);
    	
    	@signal[sentMsg-*](type=bool); // note the wildcard
		@statisticTemplate[sentMsg](title="Msg sent to $namePart2 SP #$namePart3 from Source $namePart4"; record=vector, sum, mean; interpolationmode=none);
    gates:
        output out[];
}
