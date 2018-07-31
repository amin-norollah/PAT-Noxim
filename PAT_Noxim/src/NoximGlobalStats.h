/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the global statistics
 */

#ifndef __NOXIMGLOBALSTATS_H__
#define __NOXIMGLOBALSTATS_H__

#include <iostream>
#include <vector>
#include <iomanip>
#include "NoximNoC.h"
#include "NoximTile.h"
using namespace std;

class NoximGlobalStats {

  public:

    NoximGlobalStats(const NoximNoC * _noc);

    // Returns the aggragated average delay (cycles)
    double getAverageDelay();
	double getNiAverageDelay();
	double getNwAverageDelay();
	//Returns the  Layer Average Delay
	vector<double>   getLayerAverageDelay(); 
	//Returns the Layer Routed Flits
	vector<unsigned int>   getLayerRoutedFlits(); 
	vector<unsigned int>   getLayerRoutedFlits(int i); 
	//Returns the avg. waiting time in each buffer
	double getAverageWaitingTime(); 
    // Returns the aggragated average delay (cycles) for communication src_id->dst_id
    double getAverageDelay(const int src_id, const int dst_id);
	
	// Returns the delay histogram
	double getDelayHist();
	double getDelayHist(const int node_id, double interval, double *DistHist, double *DelayHist);
    // Returns the max delay
    double getMaxDelay();

    // Returns the max delay (cycles) experimented by destination
    // node_id. Returns -1 if node_id is not destination of any
    // communication
    double getMaxDelay(const int node_id);

    // Returns the max delay (cycles) for communication src_id->dst_id
    double getMaxDelay(const int src_id, const int dst_id);

    // Returns tha matrix of max delay for any node of the network
    vector<vector<vector<double> > > getMaxDelayMtx();   
	
    // Returns the aggragated average throughput (flits/cycles)
    double getAverageThroughput();

    // Returns the aggragated average throughput (flits/cycles) for
    // communication src_id->dst_id
    double getAverageThroughput(const int src_id, const int dst_id);

    // Returns the total number of received packets
    unsigned int getReceivedPackets();

	unsigned int getReceivedPackets_real();

    // Returns the total number of received flits
    unsigned int getReceivedFlits();

    // Returns the maximum value of the accepted traffic
    double getThroughput();

	int getFlitInBuffer();
	double getNwTotalAverageDelay();
	double getNiTotalAverageDelay();
	double getMsgTotalAverageDelay();
    // Returns the number of routed flits for each router
	vector<vector<vector<unsigned long> > > getRoutedFlitsMtx();////
	// Returns the layer power
    vector<double> getLayerPower();
	void showStats2(std::ostream& out = std::cout);
    // Returns the total power
    double getRouterPower();
	double getCorePower();
	double getPowerDynamic();
	double getPowerStatic();
	// Return area
	double getTotalArea();
	double getArea(int _switch);
	
	// Shows scratch buffer hist in matlab 
	unsigned long getMaxScratch();
	pair<double,double> getScratchHist();
    // Shows global statistics
    void showStats(bool detailed = false);

#ifdef TESTING
    unsigned int drained_total;
#endif

  private:
    const NoximNoC *noc;
};

#endif
