/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the statistics
 */

#ifndef __NOXIMSTATS_H__
#define __NOXIMSTATS_H__

#include <iostream>
#include <iomanip>
#include <vector>
#include "NoximMain.h"
#include "NoximPower.h"
using namespace std;

struct CommHistory {
    int src_id;
	vector < double >delays;
	vector < double >ni_delays;
	vector < double >nw_delays;
    unsigned int total_received_flits;
    double last_received_flit_time;
	int num_tail;
};

class NoximStats {

  public:

    NoximStats() {
    } void configure(const int node_id, const double _warm_up_time);

    // Access point for stats update
    void receivedFlit(const double arrival_time, const NoximFlit & flit);

    // Returns the average delay (cycles) for the current node as
    // regards to the communication whose source is src_id
    double getAverageDelay(const int src_id);
    // Returns the average delay (cycles) for the current node
    double getAverageDelay();
	
	double getNiAverageDelay(const int src_id);
	double getNiAverageDelay();
	double getNwAverageDelay(const int src_id);
	double getNwAverageDelay();
	
	// Returns the delay histogram
	double getDelayHist(NoximCoord dst,double interval, double *DistHist, double *DelayHist);
	
    // Returns the max delay for the current node as regards the
    // communication whose source node is src_id
    double getMaxDelay(const int src_id);

    // Returns the max delay (cycles) for the current node
    double getMaxDelay();

    // Returns the average throughput (flits/cycle) for the current node
    // and for the communication whose source is src_id
    double getAverageThroughput(const int src_id);

    // Returns the average throughput (flits/cycle) for the current node
    double getAverageThroughput();

    // Returns the number of received packets from current node
    unsigned int getReceivedPackets();

	unsigned int getReceivedPackets_real();

    // Returns the number of received flits from current node
    unsigned int getReceivedFlits();

    // Returns the number of communications whose destination is the
    // current node
    unsigned int getTotalCommunications();

    // Returns the energy consumed for communication src_id-->dst_id
    // under the following assumptions: (i) Minimal routing is
    // considered, (ii) constant packet size is considered (as the
    // average between the minimum and the maximum packet size).
    double getCommunicationEnergy(int src_id, int dst_id);

    // Shows statistics for the current node
    void showStats(int curr_node, std::ostream & out =
		   std::cout, bool header = false);

	void resetChist();
	
  public:
    unsigned int recive_vc1;//taheri
	unsigned int recive_vc2;//taheri
	unsigned int recive_vc3;//taheri
    double T_neighbor [6];//taheri
    NoximPower  power;
	double	    temperature;
	double	    last_temperature;	//record the last temperature for comparison to decision throttled quota
    double      last_pre_temperature1; //the last prediciton result (Jimmy modified on 2012.04.12)
	double      last_pre_temperature2; 
	double      last_pre_temperature3; 
	double      last_pre_temperature4; 
	double      last_pre_temperature5; 
	double      last_pre_temperature6; 	
	double      pre_temperature1; 
	double      pre_temperature2;
	double      pre_temperature3;
	double      pre_temperature4; 
	double      pre_temperature5;
	double      pre_temperature6; 		
	unsigned int received[ DIRECTIONS + 1]; // for counting 7 port arrival rate
	
  private:

    int id;
    vector < CommHistory > chist;
    double warm_up_time;

    int searchCommHistory(int src_id);
};

#endif
