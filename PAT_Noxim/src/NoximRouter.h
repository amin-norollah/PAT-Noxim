/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the router
 *
 *
 * Edit by Amin Norollah @BALRUG (2017 May 25)
 *
 */

#ifndef __NOXIMROUTER_H__
#define __NOXIMROUTER_H__

#include <systemc.h>
#include "NoximMain.h"
#include "NoximBuffer.h"
#include "NoximStats.h"
#include "NoximGlobalRoutingTable.h"
#include "NoximLocalRoutingTable.h"
#include "NoximReservationTable.h"
#include "NoximVCState.h"
using namespace std;

extern ofstream log_get_error;
extern bool throttling[20][20][10];
extern double temp_budget[20][20][10];
extern float MTTT[20][20][10];
extern int wait_cnt[200];

extern unsigned int drained_volume;
	
// NoximRouter - SystemC module representing a router in the Network-on-Chip
// This router implements a pipelined design with support for virtual channels,
// various routing algorithms, and adaptive selection strategies
SC_MODULE(NoximRouter)
{

    // I/O Ports
    sc_in_clk clock;		                  				// The input clock signal for synchronous operations
    sc_in <bool> reset;                           			// The reset signal to initialize all router state

	/////////////////////////////////////////
	////////////// Norollah - Virtual Channel Support
	// Network Interface: Handshake protocol with request/acknowledge signals
	// Each direction has multiple virtual channels for improved throughput and deadlock prevention
    sc_in  <NoximFlit> flit_rx      [DIRECTIONS + 2];	  	// Input flit channels (6 directions + local + semi-local = 8 total)
    sc_in  <bool     > req_rx       [DIRECTIONS + 2][DEFAULT_NUM_VC];	  	// Request signals from neighbors for each VC on each input port
    sc_out <bool     > ack_rx       [DIRECTIONS + 2][DEFAULT_NUM_VC];	  	// Acknowledge signals sent back to neighbors indicating buffer availability
                                    
    sc_out <NoximFlit> flit_tx      [DIRECTIONS + 2];   	// Output flit channels (8 total: 6 directions + local + semi-local)
    sc_out <bool     > req_tx       [DIRECTIONS + 2][DEFAULT_NUM_VC];	  	// Request signals to neighbors requesting to send flits
    sc_in  <bool     > ack_tx       [DIRECTIONS + 2][DEFAULT_NUM_VC];	  	// Acknowledge signals from neighbors indicating they can receive
	////////////// End
	/////////////////////////////////////////


    // Buffer status communication - used for adaptive routing decisions
    sc_out <int> free_slots         [DIRECTIONS + 1];              // Number of free buffer slots available for each output direction
    sc_in  <int> free_slots_neighbor[DIRECTIONS + 1];              // Free slots information received from neighboring routers
	             
	// Thermal-aware routing signals - throttle status communication
	sc_out <bool>on_off             [DIRECTIONS];				    // Throttle status signal to neighbors (1 = throttled, 0 = normal)
	sc_in  <bool>on_off_neighbor    [DIRECTIONS];	                // Throttle status received from neighbors (for routing decisions)
	
	// Thermal Budget signals - maximum time before temperature threshold is reached
	sc_out <float>TB				[DIRECTIONS];				    // Thermal Budget (MTTT) sent to neighbors
	sc_in  <float>TB_neighbor		[DIRECTIONS];	                // Thermal Budget received from neighbors

	// Predicted Delta Temperature - temperature difference prediction for routing
	sc_out <float>PDT				[DIRECTIONS];                  // Predicted Delta Temperature sent to neighbors
    sc_in  <float>PDT_neighbor      [DIRECTIONS];                  // Predicted Delta Temperature received from neighbors

	// Buffer utilization information - detailed buffer status for each direction and VC
	sc_out <float>buf[DIRECTIONS+2]	[DIRECTIONS];                  // Buffer utilization info sent to neighbors for each input port
    sc_in  <float>buf_neighbor[DIRECTIONS+2][DIRECTIONS];          // Buffer utilization received from neighbors	
/*
	sc_out <float>buf[1]             [DIRECTIONS];                                  // Information to neighbor router buf1
        sc_in  <float>buf_neighbor[1]    [DIRECTIONS];

	sc_out <float>buf[2]             [DIRECTIONS];                                  // Information to neighbor router buf2 
        sc_in  <float>buf_neighbor[2]    [DIRECTIONS];

	sc_out <float>buf[3]             [DIRECTIONS];                                  // Information to neighbor router buf3
        sc_in  <float>buf_neighbor[3]    [DIRECTIONS];

	sc_out <float>buf[4]             [DIRECTIONS];                                  // Information to neighbor router buf4 
        sc_in  <float>buf_neighbor[4]    [DIRECTIONS];

	sc_out <float>buf[5]             [DIRECTIONS];                                  // Information to neighbor router buf5 
        sc_in  <float>buf_neighbor[5]    [DIRECTIONS];

	sc_out <float>buf[6]             [DIRECTIONS];                                  // Information to neighbor router buf6 
        sc_in  <float>buf_neighbor[6]    [DIRECTIONS];

	sc_out <float>buf[7]             [DIRECTIONS];                                  // Information to neighbor router buf7 
        sc_in  <float>buf_neighbor[7]    [DIRECTIONS];
*/	
	/*******RCA (Region Congestion Awareness)******/
	// Derek - Congestion monitoring network signals
	// RCA provides fine-grained congestion information for better routing decisions
	sc_out<int>       RCA_data_out[8];                             // RCA congestion data sent to neighbors (8 channels: 4 directions × 2 sub-channels)
	sc_in<int>        RCA_data_in[8];                              // RCA congestion data received from neighbors
	// Output Buffer Level (OBL) for beltway routing
	sc_out<int> free_slots_PE[4];                                  // Free slots sent to Processing Element (4 lateral directions)
	sc_out<int> RCA_PE[8];                                         // RCA data sent to Processing Element for traffic generation decisions
	sc_out<NoximNoP_data> NoP_PE[4];                               // NoP data sent to Processing Element
	
	// Throttling monitor network - propagates emergency state information
	sc_out<double>	 monitor_out[DIRECTIONS];                       // Throttling monitor output to neighbors
	sc_in<double>	 monitor_in [DIRECTIONS];                       // Throttling monitor input from neighbors
	
	// Neighbor-on-Path (NoP) signals - congestion information exchange
	// NoP data contains buffer status and availability from neighboring routers
    sc_out < NoximNoP_data > NoP_data_out[DIRECTIONS];             // NoP data sent to each neighbor direction
    sc_in  < NoximNoP_data > NoP_data_in [DIRECTIONS];             // NoP data received from each neighbor direction
	// Vertical free slot information - for 3D NoC layer-to-layer communication
	sc_out < NoximNoP_data > vertical_free_slot_out;               // Vertical free slots broadcasted to all layers above/below
    sc_in  < NoximNoP_data > vertical_free_slot_in[DEFAULT_MESH_DIM_Z]; // Vertical free slots received from all layers
    // Internal registers and state variables
    bool em_t;                                                      // Emergency mode flag (local router state)
    // Pre-computed routing hints for speculative routing and optimization
    int south	[DIRECTIONS + 2][DEFAULT_NUM_VC];                   // Pre-computed south direction hint for each input VC
    int east	[DIRECTIONS + 2][DEFAULT_NUM_VC];                   // Pre-computed east direction hint for each input VC
    int west	[DIRECTIONS + 2][DEFAULT_NUM_VC];                   // Pre-computed west direction hint for each input VC
    int north	[DIRECTIONS + 2][DEFAULT_NUM_VC];                   // Pre-computed north direction hint for each input VC

	/////////////////////////////////////////
	////////////// Norollah - Router Pipeline State
	// Virtual Channel State Management - tracks state of each VC in pipeline stages
	NoximVCState		   vc_state;                                // Manages VC states (IDLE, ROUTING, VCALLOCATING, ACTIVE)
	NoximReservationTable  reservation_table;                      // Switch reservation table - prevents output port conflicts
	NoximBuffer            buffer[DIRECTIONS + 2][DEFAULT_NUM_VC]; // Input buffers: one per input port per VC (circular buffer)

	// Round-robin arbitration starting points - ensures fairness across ports/VCs
	int					start_from_port;                            // Starting port for Virtual Channel Allocation (VA) round-robin
	int					start_from_vc;                              // Starting VC for Routing Computation (RC) round-robin

	// Statistics and tracking
	unsigned long routed_flits[DIRECTIONS + 2][DEFAULT_NUM_VC];     // Count of flits routed through each input port/VC

	// Baseline router pipeline wait states - accounts for pipeline stages in baseline architectures
	int					baseline_wait[DIRECTIONS + 2][DEFAULT_NUM_VC];  // Cycles to wait for pipeline to complete (3/4/5 stage)
	int					id_recieve[DIRECTIONS + 2][DEFAULT_NUM_VC];     // Test/debug ID tracking for received flits
	int				 	pre_routing[DIRECTIONS + 2][DEFAULT_NUM_VC];     // Pre-computed routing output (for look-ahead routing)

	////////////// Switch Allocation (SA) - Two-stage arbitration
	// Step 1: Select winning VC from each input port
	// Step 2: Select winning input port for each output port
	bool				RC_ACTION[DIRECTIONS + 2];                  // Flag: RC unit available for this input port (one RC per input)
	bool				can_step1[DIRECTIONS + 2][DEFAULT_NUM_VC]; // Step 1 eligibility: VC can participate in SA
	int					can_step2[DIRECTIONS + 2][DIRECTIONS + 2];  // Step 2 eligibility: input port can request output port
	int					win_port[DIRECTIONS + 2];                   // Winning input port for each output port (final SA result)
	int					win_vc[DIRECTIONS + 2];                     // Winning VC for each output port (from winning input)
	int					per_step1[DIRECTIONS + 2];                  // Permitted VC from step 1 for each input port
	int					win_port_temp[DIRECTIONS + 2];              // Temporary winner port (for speculation)
	int					win_vc_temp[DIRECTIONS + 2];                // Temporary winner VC (for speculation)
	int					vc_out_va[DIRECTIONS + 2][DEFAULT_NUM_VC];  // Output VC allocated during VA stage
	int					temp_SA_stage, o_SA_stage, vc_SA_stage;     // Temporary variables for SA computation
	bool				wait_SA_stage;                              // Flag: waiting in SA stage due to contention
	int					start_from_port_SA;                         // Starting port for Switch Allocation round-robin
	// Speculative SA - allows SA to proceed before VA completes (reduces latency)
	bool				speculation_sa_enale[DIRECTIONS + 2][DEFAULT_NUM_VC];  // Enable speculative SA for this VC
	bool				can_step_2_speculation[DIRECTIONS + 2];     // Speculation eligibility for step 2
	void				delay_baseline(int input, int vc_in, int mode);  // Manage baseline pipeline delays

	// Baseline architecture support - simpler router without full pipeline
	int					start_from_port_baseline;                  // Starting port for baseline VC selection
	int					choose_vc_in_baseline[DIRECTIONS + 2];     // Selected VC for each input in baseline mode
	bool				change_vc_baseline[DIRECTIONS + 2];        // Flag: need to reselect VC for this input
	////////////// End 
	/////////////////////////////////////////

    //////taheri/end
    int                    local_id;                               // Unique router ID in the mesh
    int                    routing_type;                           // Type of routing algorithm configured for this router
    int                    selection_type;                         // Type of output port selection strategy
    NoximStats             stats;                                  // Statistics collection (power, delay, throughput, etc.)
    NoximLocalRoutingTable routing_table;                          // Local routing table (for table-based routing algorithms)
	int		               cnt_neighbor;                           // Counter: packets received from neighboring routers
	int		               cnt_received;                           // Counter: total packets received
	double                 buffer_util;                            // Buffer utilization percentage
	double                 buffer_used;                            // Total buffer space currently used
	unsigned int           local_drained;                          // Count of flits drained to local processing element
    // Functions
    unsigned long          getRoutedFlits        ();     // Returns the number of routed flits
	unsigned long          getRoutedFlits   (int i, int vc);     // Returns the number of routed flits
    unsigned long          getRoutedDWFlits      ();     // Returns the number of routed flits
	unsigned long          getWaitingTime   (int i , int vc);
	unsigned long          getTotalWaitingTime   ();
	unsigned long          getRoutedPackets      ();
    unsigned long          getFlitsCount         (){ return _buffer_pkt_count; };     // Returns the number of flits into the router
	unsigned long          getMsgDelay           (){ return _buffer_pkt_msg_delay; };
	unsigned long          getNiDelay            (){ return _buffer_pkt_ni_delay;  };
	unsigned long          getNwDelay            (){ return _buffer_pkt_nw_delay;  };
	void                   CalcDelay             ();
	int                    getFlitRoute     (int i);
	int                    getDirAvailable  (int i,int vc);
    double                 getRouterPower        ();     // Returns the total power dissipated by the router
	double                 getCorePower			 ();     // Returns the total power dissipated by the Processing element
	double                 getTotalPower		 ();     // Returns the total power dissipated
	double				   getDynamicPower		 ();
	double				   getStaticPower		 ();
	void                   TraffThrottlingProcess();
	void                   IntoEmergency         ();
	void                   OutOfEmergency        ();

	// Configuration function - called during NoC construction
	void configure(const int _id, const double _warm_up_time,
		   const unsigned int _max_buffer_size,
		   NoximGlobalRoutingTable & grt);
	// Thermal Budget Distribution - dynamically adjusts buffer sizes based on thermal gradients
	void TBDB(float consumption_rate);

	// Set emergency state based on NoC-level thermal conditions
	void setNoCEmergency( bool noc_emergency){ _noc_emergency = noc_emergency;};
    // Constructor - registers SystemC processes (methods)

	SC_CTOR(NoximRouter) {
	SC_METHOD(rxProcess);
	sensitive << reset;
	sensitive << clock.pos();

	SC_METHOD(txProcess);
	sensitive << reset;
	sensitive << clock.pos();

	SC_METHOD(bufferMonitor);
	sensitive << reset;
	sensitive << clock.pos();

	SC_METHOD(RCA_Aggregation);
	sensitive << reset;
	sensitive << clock.pos();

    SC_METHOD(TraffThrottlingProcess);
    sensitive << reset;
    sensitive << clock.pos();
    }

  private:

	// Main SystemC processes - executed every clock cycle
	void                   rxProcess             ();     // Receive process: accepts incoming flits and stores in buffers
    void                   txProcess             ();     // Transmit process: routing, VC allocation, switch allocation, flit transmission
    void                   bufferMonitor         ();     // Buffer monitoring: updates free slots and sends NoP data to neighbors

    // Routing computation functions
    // Main routing function: computes output port from candidate directions returned by routing algorithm
    int route(const NoximRouteData & route_data,int*,int*,int*);
	// Pre-routing: computes routing for next router (for look-ahead/pipelined routing)
	int pre_route(const NoximRouteData & route_data, int*, int*, int*);

    // Detour function: alternative routing path when primary path is congested/throttled
    int Detour(const NoximRouteData & route_data, int input, int waiting);
    // Wrapper functions for routing and selection
    int           selectionFunction  (const vector <int> &directions, const NoximRouteData & route_data);
    vector < int >routingFunction    (const NoximRouteData & route_data,int*,int*,int*);
	// Data Width Layer Selection - selects which layer to route in 3D NoC
	vector < int >DW_layerSelFunction(const int select_routing, const NoximCoord& current, const NoximCoord& destination, const NoximCoord& source, int dw_layer);

    // selection strategies
    int selectionRandom     (const vector <int> & directions                                   );
    int selectionBufferLevel(const vector <int> & directions                                   );
    int selectionNoP        (const vector <int> & directions, const NoximRouteData & route_data);
	int selectionProposed   (const vector <int> & directions, const NoximRouteData & route_data);
    int selectionRCA2D      (const vector <int> & directions, const NoximRouteData & route_data);
    int selectionThermal    (const vector <int> & directions, const NoximRouteData & route_data);
	// routing functions
    vector < int >routing_DTBR             (const NoximCoord & current                          ,const NoximCoord & destination, int*,int*, int *);
    vector < int >routing_off_on_xy2             (const NoximCoord & current                          ,const NoximCoord & destination, int*,int*,int*);
	vector < int >routingXYZ             (const NoximCoord & current                          ,const NoximCoord & destination);
	vector < int >routingZXY             (const NoximCoord & current                          ,const NoximCoord & destination);
	vector < int >routingWestFirst       (const NoximCoord & current                          ,const NoximCoord & destination);
    vector < int >routingNorthLast       (const NoximCoord & current                          ,const NoximCoord & destination);
    vector < int >routingNegativeFirst   (const NoximCoord & current                          ,const NoximCoord & destination);
	vector < int >routingLookAhead       (const NoximCoord & current                          ,const NoximCoord & destination);
    vector < int >routingFullyAdaptive   (const NoximCoord & current                          ,const NoximCoord & destination);
    vector < int >routingOddEven         (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
    vector < int >routingDyAD            (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingOddEven_Z       (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingOddEven_for_3D  (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingOddEven_3D      (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingDownward        (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingWF_Downward     (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingOddEven_Downward(const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination, const NoximRouteData& route_data); //Foster modified
    vector < int >routingTableBased      (const NoximCoord & current,const int dir_in         ,const NoximCoord & destination);
	vector < int >routingDLADR           (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination,const int select_routing, int dw_layer);
	vector < int >routingDLAR            (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination,const int select_routing, int dw_layer);
	vector < int >routingDLDR            (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination,const int select_routing, int dw_layer);
	vector < int >routingTLAR_DW         (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingTLAR_DW_VBDR    (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingTLAR_DW_IPD     (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingTLAR_DW_ADWL    (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination);
	vector < int >routingTLAR_DW_ODWL    (const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination, int dw_layer);
	vector < int >routingTLAR_DW_ODWL_IPD(const NoximCoord & current,const NoximCoord & source,const NoximCoord & destination, int dw_layer);

	unsigned long waiting	  [DIRECTIONS + 2][DEFAULT_NUM_VC];
	unsigned long routed_DWflits                              ;
	unsigned long routed_packets                              ;
	vector <NoximFlit> HeadFlit                               ;



    void          RCA_Aggregation                    ()      ;
	bool          inCongestion                       ()      ;
	NoximNoP_data getCurrentNoPData                  () const;
    void          NoP_report                         () const;
    int           reflexDirection       (int direction) const;
    int           getNeighborId(int _id, int direction) const;
	bool          Adaptive_ok(NoximCoord &sour,NoximCoord &dest);
	void          DBA(int outgoing,NoximFlit* head);
    int           NoPScore(const NoximNoP_data & nop_data, const vector <int> & nop_channels) const;
	//Run-time Thermal Management (RTM) state variables
	bool 	               _emergency;                        // Emergency mode flag: router is throttled due to thermal issues
	int 	               _emergency_level;                   // Emergency level: severity of throttling (0 to buffer_depth-1)
	bool	               _throttle_neighbor;                 // Flag: throttle incoming traffic from neighbor routers
	unsigned long          _total_waiting;                    // Total cycles flits spent waiting in buffers
	unsigned long          _buffer_pkt_count;                 // Count of packets currently in buffers
	unsigned long          _buffer_pkt_msg_delay;             // Total message delay (from packet generation)
	unsigned long          _buffer_pkt_ni_delay;              // Network interface delay (from NI entry)
	unsigned long          _buffer_pkt_nw_delay;              // Network delay (from first router entry)
	bool                   _noc_emergency;                    // NoC-wide emergency mode flag
	unsigned int           RST;                               // Router Speed Throttle: cycles to wait in throttled state
	unsigned int           DFS;                               // Data Forwarding Speed: cycles between flit transmissions
	unsigned int           buf_budget;                        // Available buffer budget for dynamic buffer allocation

};

#endif
