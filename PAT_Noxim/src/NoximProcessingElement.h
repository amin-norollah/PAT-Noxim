/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the processing element
 *
 *
 * Edited by Amin Norollah @BALRUG (2017 May 25)
 *
 */

#ifndef __NOXIMPROCESSINGELEMENT_H__
#define __NOXIMPROCESSINGELEMENT_H__
#include <queue>
#include <systemc.h>
#include <cmath>
#include <cassert>
#include "NoximMain.h"
#include "NoximGlobalTrafficTable.h"
#include "NoximReservationTable.h"
using namespace std;

extern bool throttling[20][20][10];
extern double temp_budget[20][20][10]; //Derek 2012.12.21
extern int num_pkt;
extern float MTTT[20][20][10];
extern int gradient[20][20][4];

SC_MODULE(NoximProcessingElement)
{
    // I/O Ports
    sc_in_clk               clock;		// The input clock for the PE
    sc_in     < bool      > reset;		// The reset signal for the PE

	//////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////   added by Amin Norollah
    sc_in     < NoximFlit > flit_rx		;	// The input channel
    sc_in     < bool      > req_rx		[DEFAULT_NUM_VC];		// The request associated with the input channel
    sc_out    < bool      > ack_rx		[DEFAULT_NUM_VC];		// The outgoing ack signal associated with the input channel

	sc_in     < NoximFlit > flit_semi_rx;	// The input channel
    sc_in     < bool      > req_semi_rx [DEFAULT_NUM_VC];		// The request associated with the input channel
    sc_out    < bool      > ack_semi_rx [DEFAULT_NUM_VC];		// The outgoing ack signal associated with the input channel
	
    sc_out    < NoximFlit > flit_tx		;	// The output channel
    sc_out    < bool      > req_tx		[DEFAULT_NUM_VC];		// The request associated with the output channel
    sc_in     < bool      > ack_tx		[DEFAULT_NUM_VC];		// The outgoing ack signal associated with the output channel
	
	sc_out    < NoximFlit > flit_semi_tx;	// The output channel
    sc_out    < bool      > req_semi_tx [DEFAULT_NUM_VC];		// The request associated with the output channel
    sc_in     < bool      > ack_semi_tx [DEFAULT_NUM_VC];		// The outgoing ack signal associated with the output channel
	//////////////////////////////////////////////////////////////////////////////////
 
	sc_out <int >free_slots;
    sc_in < int >free_slots_neighbor;
	//OBL for beltway, 
	sc_in < int >free_slots_router[4];
	sc_in < int >RCA_router[8];
	sc_in < NoximNoP_data > NoP_router[4];


	NoximReservationTable  reservation_table_pe;
	NoximPacket temp_p[DEFAULT_NUM_VC];
	bool wait;
	int credit[DEFAULT_NUM_VC];
	int credit_semi;

    // Registers
	int start_from_vc, start_from_vc_2, start_from_message;
	bool _continue;
	bool per_vc;
    int  local_id;			            // Unique identification number
    bool transmittedAtPreviousCycle;	// Used for distributions with memory
	int refly_pkt;    //the numbers of redirective pkt in the queue (Jimmy added on 2011.10.25)
	int packet_queue_length					;
	queue < NoximPacket > message_queue		;
	queue < NoximPacket > packet_queue		;	// Local queue of packets
	queue < NoximFlit   > flit_queue;   // Local queue of flits for redirective pkt (Jimmy modofied on 2011.05.29)
	
	// Functions
    void rxProcess();					// The receiving process
    void txProcess();					// The transmitting process   
	bool canShot(NoximPacket & packet );// True when the packet must be shot
    void TraffThrottlingProcess();
	bool TLA    (NoximPacket & packet );// Transport Layer Assisted
	bool TAAR    (NoximPacket & packet );
	bool Beltway(NoximPacket & packet );// Beltway Routing
	int  sel_int_node(int source, int destination); //select a suitable intermedium node (Jimmy modified on 2011.06.08)
	int  sel_int_node_belt (int source, int destination, bool &beltway,int &routing);
	int  sel_int_node_Mcascade(int source, int  destination);
	bool inROC(NoximCoord s, NoximCoord d);
	bool inROC_S(NoximCoord s);
	int  inRing(NoximCoord d);
	void IntoEmergency();
	void OutOfEmergency();
	void IntoCleanStage();
	void OutOfCleanStage();
	void RTM_set_var(int non_throt_layer, int non_beltway_layer, int RoC_col_max, int RoC_col_min, int RoC_row_max, int RoC_row_min);
	void CalcDelay();
	void CalcMessageDelay();
	unsigned long  getNiDelay()      { return _packet_queue_ni_delay;};
	unsigned long  getPktQMsgDelay() { return _packet_queue_msg_delay;};
	unsigned long  getFlitsNum()     { return _packet_queue_num;  };
    unsigned long  getMsgQDelay()    { return _message_queue_delay;};
	unsigned long  getMsgFlitsNum()  { return _message_queue_num;  };  

	
	NoximFlit   nextFlit(int vc);		// Take the next flit of the current packet
    NoximPacket trafficRandom();		// Random destination distribution
    NoximPacket trafficTranspose1();	// Transpose 1 destination distribution
    NoximPacket trafficTranspose2();	// Transpose 2 destination distribution
    NoximPacket trafficBitReversal();	// Bit-reversal destination distribution
    NoximPacket trafficShuffle();		// Shuffle destination distribution
    NoximPacket trafficButterfly();		// Butterfly destination distribution
	NoximPacket trafficRandom_Tvar();
	NoximFlit   flit_redir();           // Redirect the flit while this PE is an intermedium node (Jimmy modified on 2011.10.25)
	
	//static functions
	void ResetTransient_Transmit();
	int getTransient_Total_Transmit       (){return _Transient_total_transmit;};
	int getTransient_Adaptive_Transmit    (){return _Transient_adaptive_transmit;};
	int getTransient_DOR_Transmit         (){return _Transient_dor_transmit;};
	int getTransient_DW_Transmit          (){return _Transient_dw_transmit;};
	int getTransient_Mid_Adaptive_Transmit(){return _Transient_mid_adaptive_transmit;};
	int getTransient_Mid_DOR_Transmit     (){return _Transient_mid_dor_transmit;};
	int getTransient_Mid_DW_Transmit      (){return _Transient_mid_dw_transmit;};
	int getTransient_Beltway_Transmit     (){return _Transient_beltway_transmit;};
	
	int getTotal_Transmit       (){return _total_transmit;};
	int getAdaptive_Transmit    (){return _adaptive_transmit;};
	int getDOR_Transmit         (){return _dor_transmit;};
	int getDW_Transmit          (){return _dw_transmit;};
	int getMid_Adaptive_Transmit(){return _mid_adaptive_transmit;};
	int getMid_DOR_Transmit     (){return _mid_dor_transmit;};
	int getMid_DW_Transmit      (){return _mid_dw_transmit;};
	int getBeltway_Transmit     (){return _beltway_transmit;};
	
	int     transmit;				
	int     not_transmit;			
	int	    cnt_local;	


	
    NoximGlobalTrafficTable *traffic_table;	// Reference to the Global traffic Table
    bool never_transmit;	// true if the PE does not transmit any packet 

    void   fixRanges             (const NoximCoord, NoximCoord &);	// Fix the ranges of the destination
    int    randInt               (int min, int max);				// Extracts a random integer number between min and max
    int    getRandomSize         ();								// Returns a random size in flits for the packet
    void   setBit                (int &x, int w, int v);
    int    getBit                (int x, int w);
    double log2ceil              (double x);

	
    // Constructor
    SC_CTOR(NoximProcessingElement) {
	SC_METHOD(rxProcess);
	sensitive << reset;
	sensitive << clock.pos();

	SC_METHOD(txProcess);
	sensitive << reset;
	sensitive << clock.pos();

	SC_METHOD(TraffThrottlingProcess);
    sensitive << reset;
    sensitive << clock.pos();
    }
	private:
		/*unsigned int DFS;
		unsigned int RST;*/
		bool    _emergency;
		int     _emergency_level;
		bool    _clean_all;		
		int     _non_throt_layer;
		int     _non_beltway_layer;
		int     _RoC_col_min;
		int     _RoC_col_max;
		int     _RoC_row_min;
		int     _RoC_row_max;
		unsigned int _round_MC;
		int     _total_transmit;
		int     _adaptive_transmit;
		int     _dor_transmit;
		int     _dw_transmit;
		int     _mid_adaptive_transmit;
		int     _mid_dor_transmit;
		int     _mid_dw_transmit;
		int     _beltway_transmit;
		int     _Transient_total_transmit;
		int     _Transient_adaptive_transmit;
		int     _Transient_dor_transmit;
		int     _Transient_dw_transmit;
		int     _Transient_mid_adaptive_transmit;
		int     _Transient_mid_dor_transmit;
		int     _Transient_mid_dw_transmit;
		int     _Transient_beltway_transmit;
		unsigned long _packet_queue_num;
		unsigned long _packet_queue_ni_delay;
		unsigned long _packet_queue_msg_delay;
		unsigned long _message_queue_num;
		unsigned long _message_queue_delay;
		int _throttling[20][20][10];
		
		void    _flit_static(NoximFlit flit);
		bool    _beltway_OBL(NoximCoord s,NoximCoord d);
		bool    _beltway_RCA(NoximCoord s,NoximCoord d);
		bool    _beltway_RND(NoximCoord s,NoximCoord d);
		bool    _beltway_NoP(NoximCoord s,NoximCoord d);	
		bool    _beltway_THERMAL(NoximCoord s,NoximCoord d);		// Derek 2012.12.21
		int     _NoPScore(const NoximNoP_data & nop_data, const vector < int >&nop_channels);
		void    _updateRMM( int ***throt_table );
};

#endif
