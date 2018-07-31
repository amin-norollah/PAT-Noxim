/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the statistics
 */

#include "NoximStats.h"

// TODO: nan in averageDelay

void NoximStats::configure(const int node_id, const double _warm_up_time)
{
    id = node_id;
    warm_up_time = _warm_up_time;
}

void NoximStats::resetChist() {
	chist.resize(0);
}

void NoximStats::receivedFlit(const double arrival_time,      const NoximFlit & flit)
{
	/////////////////////////////////////////////////////
	////////////////	Warm_Up
	if (arrival_time - DEFAULT_RESET_TIME < warm_up_time) return;
	////////////////	End
	/////////////////////////////////////////////////////

	int i = searchCommHistory(flit.src_id);

	/////////////////////////////////////////////////////
	////////////////	Create History for Scurce Packet
	if (i == -1) {
		// first flit received from a given source
		// initialize CommHist structure
		CommHistory ch;

		ch.src_id = flit.src_id;
		ch.total_received_flits = 0;
		ch.num_tail = 0;
		chist.push_back(ch);

		i = chist.size() - 1;
	}
	////////////////	End
	/////////////////////////////////////////////////////

	if (flit.flit_type == FLIT_TYPE_HEAD) {
		chist[i].delays.push_back(arrival_time - flit.timestamp_ni);
		chist[i].ni_delays.push_back(arrival_time - flit.timestamp_ni);
		chist[i].nw_delays.push_back(arrival_time - flit.timestamp_nw);
		//cout<<arrival_time<<"\t"<<flit.timestamp<<"\t"<<flit.timestamp_ni<<"\t"<<flit.timestamp_nw<<endl;
	}
	if (flit.flit_type == FLIT_TYPE_TAIL)
		chist[i].num_tail++;

	chist[i].total_received_flits++;
	chist[i].last_received_flit_time = arrival_time - warm_up_time;
}

double NoximStats::getAverageDelay(const int src_id)
{
    double sum = 0.0;

    int i = searchCommHistory(src_id);

    assert(i >= 0);

    for (unsigned int j = 0; j < chist[i].delays.size(); j++)
	sum += chist[i].delays[j];

    return sum / (double) chist[i].delays.size();
}

double NoximStats::getAverageDelay()
{
    double avg = 0.0;
	unsigned int samples;

    for (unsigned int k = 0; k < chist.size(); k++) {
	samples = chist[k].delays.size();
	if (samples)
	    avg += (double) samples *getAverageDelay(chist[k].src_id);
    }

    return avg / (double) getReceivedPackets();
}

double NoximStats::getNiAverageDelay(const int src_id)
{
    double sum = 0.0;

    int i = searchCommHistory(src_id);

    assert(i >= 0);

    for (unsigned int j = 0; j < chist[i].ni_delays.size(); j++)
	sum += chist[i].ni_delays[j];

    return sum / (double) chist[i].ni_delays.size();
}

double NoximStats::getNiAverageDelay()
{
    double avg = 0.0;
	unsigned int samples;

    for (unsigned int k = 0; k < chist.size(); k++) {
	samples = chist[k].ni_delays.size();
	if (samples)
	    avg += (double) samples *getNiAverageDelay(chist[k].src_id);
    }

    return avg / (double) getReceivedPackets();
}

double NoximStats::getNwAverageDelay(const int src_id)
{
    double sum = 0.0;

    int i = searchCommHistory(src_id);

    assert(i >= 0);

    for (unsigned int j = 0; j < chist[i].nw_delays.size(); j++)
	sum += chist[i].nw_delays[j];

    return sum / (double) chist[i].nw_delays.size();
}

double NoximStats::getNwAverageDelay()
{
    double avg = 0.0;
	unsigned int samples;

    for (unsigned int k = 0; k < chist.size(); k++) {
	samples = chist[k].nw_delays.size();
	if (samples)
	    avg += (double) samples *getNwAverageDelay(chist[k].src_id);
    }

    return avg / (double) getReceivedPackets();
}

double NoximStats::getDelayHist(NoximCoord dst,double interval, double *DistHist, double *DelayHist)
{
	//record latency histogram
	for (unsigned int k = 0 ; k < chist.size() ; k++ )
		if(chist[k].delays.size()){
			//get dist histogram
			NoximCoord src = id2Coord( chist[k].src_id );
			int dist = abs( src.x - dst.x ) + abs( src.y - dst.y ) + abs( src.z - dst.z );
			DistHist[dist] += chist[k].delays.size();
			//get delay histogram
			for (unsigned int j = 0 ; j < chist[k].delays.size() ; j++ ){
				if(chist[k].delays[j] < 500)
					DelayHist[(unsigned int)chist[k].delays[j]]++;
				//unsigned int samples = (unsigned int)chist[k].delays[j]/interval;
				//DelayHist[samples]++;
			}
    }
    return 1.0;
}

double NoximStats::getMaxDelay(const int src_id)
{
    double maxd = -1.0;

    int i = searchCommHistory(src_id);

    assert(i >= 0);

    for (unsigned int j = 0; j < chist[i].delays.size(); j++)
	if (chist[i].delays[j] > maxd) {
	    //      cout << src_id << " -> " << id << ": " << chist[i].delays[j] << endl;
	    maxd = chist[i].delays[j];
	}
    return maxd;
}

double NoximStats::getMaxDelay()
{
    double maxd = -1.0;
	unsigned int samples;
	double m;

    for (unsigned int k = 0; k < chist.size(); k++) {
		samples = chist[k].delays.size();
		if (samples) {
			m = getMaxDelay(chist[k].src_id);
			if (m > maxd)
				maxd = m;
	}
    }

    return maxd;
}

double NoximStats::getAverageThroughput(const int src_id)
{
    int i = searchCommHistory(src_id);

    assert(i >= 0);

    if (chist[i].total_received_flits == 0)
	return -1.0;
    else
	return (double) chist[i].total_received_flits /
	    (double) chist[i].last_received_flit_time;
}

double NoximStats::getAverageThroughput()
{
    double sum = 0.0;
	double avg;

    for (unsigned int k = 0; k < chist.size(); k++) {
	avg = getAverageThroughput(chist[k].src_id);
	if (avg > 0.0)
	    sum += avg;
    }

    return sum;
}
unsigned int NoximStats::getReceivedPackets()
{
    int n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].delays.size();

    return n;
}

// added by Amin Norollah
unsigned int NoximStats::getReceivedPackets_real()
{
	int n = 0;

	for (unsigned int i = 0; i < chist.size(); i++)
		n += chist[i].num_tail;

	return n;
}

unsigned int NoximStats::getReceivedFlits()
{
    int n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].total_received_flits;

    return n;
}

unsigned int NoximStats::getTotalCommunications()
{
    return chist.size();
}

double NoximStats::getCommunicationEnergy(int src_id, int dst_id)
{
    // Assumptions: minimal path routing, constant packet size

    NoximCoord src_coord = id2Coord(src_id);
    NoximCoord dst_coord = id2Coord(dst_id);

    int hops =
	abs(src_coord.x - dst_coord.x) + abs(src_coord.y - dst_coord.y);

	//----- Chihhao
	double avg_flit_num_per_packet = (NoximGlobalParams::min_packet_size + NoximGlobalParams::max_packet_size)/2;
    double energy =	hops * avg_flit_num_per_packet * 
	( power.getFlitEnergyBufferWrite()	   + power.getFlitEnergyBufferRead() + power.getFlitEnergyVC()    +
	  power.getFlitEnergyArbiterNControl() + power.getFlitEnergyCrossbar()   + power.getFlitEnergyLinks() + power.getFlitEnergyClocking());
	//----- Chihhao <end>	  
	
    return energy;
}

int NoximStats::searchCommHistory(int src_id)
{
    for (unsigned int i = 0; i < chist.size(); i++)
	if (chist[i].src_id == src_id)
	    return i;

    return -1;
}

void NoximStats::showStats(int curr_node, std::ostream & out, bool header)
{
    if (header) {
	out << "%"
	    << setw(5) << "src"
	    << setw(5) << "dst"
	    << setw(10) << "delay avg"
	    << setw(10) << "delay max"
	    << setw(15) << "throughput"
	    << setw(13) << "energy"
	    << setw(12) << "received" << setw(12) << "received" << endl;
	out << "%"
	    << setw(5) << ""
	    << setw(5) << ""
	    << setw(10) << "cycles"
	    << setw(10) << "cycles"
	    << setw(15) << "flits/cycle"
	    << setw(13) << "Joule"
	    << setw(12) << "packets" << setw(12) << "flits" << endl;
    }
    for (unsigned int i = 0; i < chist.size(); i++) {
	out << " "
	    << setw(5) << chist[i].src_id
	    << setw(5) << curr_node
	    << setw(10) << getAverageDelay(chist[i].src_id)
	    << setw(10) << getMaxDelay(chist[i].src_id)
	    << setw(15) << getAverageThroughput(chist[i].src_id)
	    << setw(13) << getCommunicationEnergy(chist[i].src_id,
						  curr_node)
	    << setw(12) << chist[i].delays.size()
	    << setw(12) << chist[i].total_received_flits << endl;
    }

    out << "% Aggregated average delay (cycles): " << getAverageDelay() <<
	endl;
    out << "% Aggregated average throughput (flits/cycle): " <<
	getAverageThroughput() << endl;
}
