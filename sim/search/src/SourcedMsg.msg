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

//
// TODO generated message class
//
message SourcedMsg {
    int source;
    int ID;
    simtime_t time_send=-1;
    simtime_t time_in=-1;
    simtime_t time_queue=-1;
    simtime_t time_serv=-1;
    simtime_t time_out=-1;
    simtime_t time_recv=-1;
}
