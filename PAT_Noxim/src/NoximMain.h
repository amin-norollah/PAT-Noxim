/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the declaration of the top-level of Noxim
 *
 */

#ifndef __NOXIMMAIN_H__
#define __NOXIMMAIN_H__

#include <cassert>
#include <systemc.h>
#include <vector>

/* parameter definition of traffic-thermal co-sim */
//#include "define.h"
#include "NoximParameters.h"

using namespace std;    
 
// NoximGlobalParams -- Global configuration parameters structure
// This structure contains all simulation parameters that are accessible throughout the simulation
// All members are static to allow global access without instantiation
struct NoximGlobalParams {
    static int                          verbose_mode;          // Verbosity level for console output
    static int                          trace_mode;            // Whether to enable signal tracing
    static char                         trace_filename[128];   // Filename for trace output
    static int                          mesh_dim_x;            // X dimension of the 3D mesh NoC
    static int                          mesh_dim_y;            // Y dimension of the 3D mesh NoC
	static int                          mesh_dim_z;            // Z dimension of the 3D mesh NoC (layers)
	static int                          num_vcs;               // Number of virtual channels per physical channel 
    static int                          buffer_depth;              // Depth of input buffers in flits
    static int                          min_packet_size;           // Minimum packet size in flits
    static int                          max_packet_size;           // Maximum packet size in flits
    static int                          routing_algorithm;         // Routing algorithm identifier (XY, WestFirst, etc.)
    static char                         routing_table_filename[128]; // Filename for table-based routing configuration
    static int                          selection_strategy;        // Output port selection strategy (random, buffer level, NoP, etc.)
    static float                        packet_injection_rate;     // Probability of packet injection per cycle per node
    static float                        probability_of_retransmission; // Probability of retransmitting a packet
    static int                          traffic_distribution;      // Traffic pattern type (uniform, hotspot, transpose, etc.)
    static char                         traffic_table_filename[128];  // Filename for table-based traffic configuration
    static int                          simulation_time;           // Total simulation time in cycles
    static int                          stats_warm_up_time;        // Warm-up period before statistics collection starts
    static int                          rnd_generator_seed;        // Random number generator seed for reproducibility
    static bool                         detailed;                  // Whether to show detailed statistics
    static vector <pair <int, double> > hotspots;                  // List of hotspot nodes with their injection rates
    static float                        dyad_threshold;            // Threshold for DyAD (Dynamic XY) routing algorithm
    static unsigned int                 max_volume_to_be_drained;  // Maximum volume of flits to be drained before stopping

	static int                          dw_layer_sel;
	static int                          burst_length;
	static int                          down_level;
	static int                          throt_type;
	static float                        throt_ratio;
	static double                       max_temp;
	static char                         max_temp_r[40];
	static bool                         pir_is_local_random;
	static bool                         buffer_alloc;
	static int                          vertical_link;
	static bool                         cascade_node;
	static bool                         Mcascade;
	static int                          Mcascade_step;
	static bool                         beltway;
	static int                          br_sel;
	static float                        beltway_trigger;
	static bool                         Mbeltway;
	static bool                         Sbeltway;
	static int                          Sbeltway_ring;
	static int                          ROC_UP;
	static int                          ROC_DOWN;
	static float                        beltway_ratio;
	static bool                         Log_all_Temp;
	static int                          clean_stage_time;
	static bool                         cal_temp;
	
	static bool                         message_level;
	static int                          dynamic_throt_case;

	static bool							arch_router;
	static bool							arch_rc;
	static bool							arch_sa;
	static bool							arch_with_credit;
};



// NoximCoord -- 3D coordinate structure for tiles in the mesh
// Represents the position (x, y, z) of a tile/node in the 3D Network-on-Chip mesh
class NoximCoord {
  public:
    int x;			// X coordinate (horizontal position in the mesh)
    int y;			// Y coordinate (vertical position in the mesh)
	int z;          // Z coordinate (layer/vertical position in 3D stack)
	
    // Equality operator to compare two coordinates
    inline bool operator ==(const NoximCoord & coord) const {
		return (coord.x == x && coord.y == y && coord.z == z);
}};

// NoximFlitType -- Enumeration defining the types of flits in a packet
// Packets are divided into flits: HEAD (first), BODY (middle), and TAIL (last)
enum NoximFlitType {
    FLIT_TYPE_HEAD,  // First flit of a packet - contains routing information
    FLIT_TYPE_BODY,  // Middle flit(s) of a packet - contains payload data
    FLIT_TYPE_TAIL   // Last flit of a packet - marks end of packet
};

// NoximPayload -- Data payload structure carried by flits
// Contains the actual data being transmitted through the network
struct NoximPayload {
    sc_uint<32> data;	// 32-bit data bus for the payload information

    // Equality operator to compare payloads
    inline bool operator ==(const NoximPayload & payload) const {
	return (payload.data == data);
}};

// NoximPacket -- Packet structure containing all information about a network packet
// Packets are the logical units of data that are broken into flits for transmission
struct NoximPacket {
    int vc;                  // Virtual channel assigned to this packet
    int south;               // Routing hint for south direction
    int east;                // Routing hint for east direction
	int test_id;             // Test/debugging identifier
    int    src_id;           // Source node ID in the mesh
    int    dst_id;           // Final destination node ID
	int    mid_id;           // Intermediate node ID (for multi-path routing)
    double timestamp;        // SystemC timestamp when packet was generated
	double timestamp_ni;     // Timestamp when packet entered network interface
	double timestamp_nw;     // Timestamp when packet entered network (router)
    int    size;             // Total size of packet in flits
    int    flit_left;        // Number of remaining flits inside the packet (countdown)
	int    routing;          // Routing algorithm used for this packet
	int    DW_layer;         // Data width layer selection (for 3D NoCs)
	bool   arr_mid;          // Flag: whether this packet has arrived at intermediate node (0: No, 1: Yes)
	bool   beltway;          // Flag: whether this packet uses beltway routing
    
    // Constructors
    NoximPacket() {
		arr_mid   = false;
		beltway   = false;
	}
    NoximPacket(const int s, const int d, const double ts, const int sz) {
	make(s, d, ts, sz);
    }

    void make(const int s, const int d, const double ts, const int sz) {
	src_id    = s;
	dst_id    = d;
	mid_id    = d;
	timestamp = ts;
	size      = sz;
	flit_left = sz;
	routing   = NoximGlobalParams::routing_algorithm;
	DW_layer  = NoximGlobalParams::mesh_dim_z - 1;
	arr_mid   = false;
	beltway   = false;
    }
};

// NoximRouteData -- Routing decision data structure
// Contains all information needed by a router to make routing decisions
struct NoximRouteData {
    int  current_id;     // ID of the current router making the routing decision
    int  src_id    ;     // Source node ID where the packet originated
    int  dst_id    ;     // Final destination node ID
	int  mid_id    ;     // Intermediate node ID (for multi-path routing)
    int  dir_in    ;     // Input direction from which the packet entered this router
	int  routing   ;     // Routing algorithm type to be used
	int  DW_layer  ;     // Data width layer selection (for 3D routing)
	bool arr_mid   ;     // Flag: whether this flit has arrived at the intermediate node
};

// NoximChannelStatus -- Status information about a communication channel
// Used to communicate buffer availability and channel state between neighboring routers
struct NoximChannelStatus {
    int    free_slots;      // Number of free buffer slots available in the channel
    int f_slots;            // Alternative free slots representation
    bool   available;       // Whether the channel is currently available for transmission
	bool   throttle;        // Throttling flag (indicates if channel is throttled due to thermal/emergency)
    // Equality operator for channel status comparison
    inline bool operator ==(const NoximChannelStatus & bs) const {
		return (free_slots == bs.free_slots && available == bs.available);
    };
};

// NoximNoP_data -- Neighbor-on-Path (NoP) data structure
// Contains congestion information from neighboring routers used for adaptive routing decisions
// Routers exchange this data to make informed routing choices based on network conditions
struct NoximNoP_data {
    int sender_id;                                          // ID of the router sending this NoP data
    NoximChannelStatus channel_status_neighbor[DIRECTIONS]; // Buffer status for each neighboring direction

    inline bool operator ==(const NoximNoP_data & nop_data) const {
	return (sender_id == nop_data.sender_id &&
		   nop_data.channel_status_neighbor[0] == channel_status_neighbor[0]
		&& nop_data.channel_status_neighbor[1] == channel_status_neighbor[1]
		&& nop_data.channel_status_neighbor[2] == channel_status_neighbor[2]
		&& nop_data.channel_status_neighbor[3] == channel_status_neighbor[3]
		);
    };
};

// NoximFlit -- Flit data structure (smallest unit of data transmitted through the network)
// Flits are fragments of packets that flow through routers and links
struct NoximFlit {
    int vc;                  // Virtual channel this flit is using
    int south;               // Pre-computed routing hint for south direction
    int east;                // Pre-computed routing hint for east direction
	int			  test_id;    // Test/debugging identifier
    int           src_id     ;  // Source node ID where the packet originated
    int           dst_id     ;  // Final destination node ID
	int           mid_id     ;  // Intermediate node ID (for multi-path routing)
    NoximFlitType flit_type  ;  // Type of flit: HEAD, BODY, or TAIL
    int           sequence_no;  // Sequence number indicating position of this flit within the packet
    NoximPayload  payload    ;  // Payload data carried by this flit
    double        timestamp  ;  // Timestamp when the packet was generated (in SystemC time)
	double 		  timestamp_ni; // Timestamp when packet entered network interface
	double 		  timestamp_nw; // Timestamp when packet entered the network (first router)
    int           hop_no     ;  // Current number of hops this flit has traveled from source
	int           routing_f  ;  // Routing function/algorithm being used
	int           DW_layer   ;  // Data width layer selection (for 3D NoCs)
	int           waiting_cnt;  // Counter for cycles spent waiting due to contention/blocking
	bool          arr_mid    ;  // Flag: whether this flit has arrived at intermediate node
	bool          beltway    ;  // Flag: whether this flit uses beltway routing path
	int			  pre_routing; // Pre-computed routing direction for next hop
	
	inline bool operator ==(const NoximFlit & flit) const {
	return (flit.src_id == src_id && flit.dst_id == dst_id
		&& flit.flit_type == flit_type
		&& flit.sequence_no == sequence_no
		&& flit.payload == payload && flit.timestamp == timestamp
		&& flit.hop_no == hop_no
		&& flit.vc == vc);
}};

// Utility function to get the current simulation cycle number
// Converts SystemC simulation time to cycle number based on CYCLE_PERIOD
inline int getCurrentCycleNum(){
	return (int)(sc_time_stamp().to_double()/1000/CYCLE_PERIOD);
};

// Output overloading

inline ostream & operator <<(ostream & os, const NoximFlit & flit)
{

    if (NoximGlobalParams::verbose_mode == VERBOSE_HIGH) {

	os << "### FLIT ###" << endl;
	os << "Source Tile[" << flit.src_id << "]" << endl;
	os << "Destination Tile[" << flit.dst_id << "]" << endl;
	os << "Intermediate Tile[" << flit.mid_id << "]" << endl;
	switch (flit.flit_type) {
	case FLIT_TYPE_HEAD:
	    os << "Flit Type is HEAD" << endl;
	    break;
	case FLIT_TYPE_BODY:
	    os << "Flit Type is BODY" << endl;
	    break;
	case FLIT_TYPE_TAIL:
	    os << "Flit Type is TAIL" << endl;
	    break;
	}
	os << "Sequence no. " << flit.sequence_no << endl;
	os << "Payload printing not implemented (yet)." << endl;
	os << "Unix timestamp at packet generation " << flit.
	    timestamp << endl;
	os << "Total number of hops from source to destination is " <<
	    flit.hop_no << endl;
    } else {
	os << "[type: ";
	switch (flit.flit_type) {
	case FLIT_TYPE_HEAD:
	    os << "H";
	    break;
	case FLIT_TYPE_BODY:
	    os << "B";
	    break;
	case FLIT_TYPE_TAIL:
	    os << "T";
	    break;
	}

	os << ", seq: " << flit.sequence_no << ", " << flit.src_id << "-->" << flit.mid_id << "-->" << flit.dst_id << " routing: "<<flit.routing_f<<"]";
    if(flit.arr_mid)
		os <<"M-node:0";
	else
		os <<"M-node:1";
	}

    return os;
}

inline ostream & operator <<(ostream & os,
			     const NoximChannelStatus & status)
{
    char msg;
    if (status.available)
	msg = 'A';
    else
	msg = 'N';
    os << msg << "(" << status.free_slots << ")";
    return os;
}

inline ostream & operator <<(ostream & os, const NoximNoP_data & NoP_data)
{
    os << "      NoP data from [" << NoP_data.sender_id << "] [ ";

    for (int j = 0; j < DIRECTIONS; j++)
	os << NoP_data.channel_status_neighbor[j] << " ";

    cout << "]" << endl;
    return os;
}

inline ostream & operator <<(ostream & os, const NoximCoord & coord)
{
	os << "(" << coord.x << "," << coord.y << "," << coord.z <<")";
    return os;
}

// Trace overloading

inline void sc_trace(sc_trace_file * &tf, const NoximFlit & flit, string & name)
{
    sc_trace(tf, flit.src_id, name + ".src_id");
    sc_trace(tf, flit.dst_id, name + ".dst_id");
    sc_trace(tf, flit.sequence_no, name + ".sequence_no");
    sc_trace(tf, flit.timestamp, name + ".timestamp");
    sc_trace(tf, flit.hop_no, name + ".hop_no");
	sc_trace(tf, flit.vc, name + ".vc");
}

inline void sc_trace(sc_trace_file * &tf, const NoximNoP_data & NoP_data, string & name)
{
    sc_trace(tf, NoP_data.sender_id, name + ".sender_id");
}

inline void sc_trace(sc_trace_file * &tf, const NoximChannelStatus & bs, string & name)
{
    sc_trace(tf, bs.free_slots, name + ".free_slots");
    sc_trace(tf, bs.available, name + ".available");
}

// Misc common functions

// Convert node ID to 3D coordinates (x, y, z)
// Node IDs are assigned sequentially: layer 0 first (row by row), then layer 1, etc.
inline NoximCoord id2Coord(int id)
{
    NoximCoord coord;

    // Calculate Z coordinate (which layer/stack level)
    coord.z = id / (NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y);
    // Calculate Y coordinate (which row in the current layer)
    coord.y = (id-coord.z*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y) /NoximGlobalParams::mesh_dim_x;
    // Calculate X coordinate (which column in the current row)
    coord.x = (id-coord.z*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y)  % NoximGlobalParams::mesh_dim_x;

    // Validate that coordinates are within mesh boundaries
    assert(coord.x < NoximGlobalParams::mesh_dim_x);
    assert(coord.y < NoximGlobalParams::mesh_dim_y);
	assert(coord.z < NoximGlobalParams::mesh_dim_z);

    return coord;
}

// Convert 3D coordinates (x, y, z) to node ID
// Inverse operation of id2Coord: computes unique node ID from position in mesh
inline int coord2Id(const NoximCoord & coord)
{
	// Formula: id = z*(X*Y) + y*X + x
	// This gives a unique ID for each position in the 3D mesh
	int id = coord.z*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y + (coord.y * NoximGlobalParams::mesh_dim_x) + coord.x; 
    // Validate that computed ID is within valid range
    assert(id < NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y * NoximGlobalParams::mesh_dim_z); 
    return id;
}

// Alternative function to convert x, y, z coordinates to node ID
// Same as coord2Id but takes individual parameters instead of NoximCoord structure
inline int xyz2Id( int x, int y, int z){
	return z * NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y + y * NoximGlobalParams::mesh_dim_x + x;
}

inline string MarkFileName( string name ){
	char temperal [20];
	sprintf( temperal, "%d", NoximGlobalParams::routing_algorithm);
	if      ( NoximGlobalParams::routing_algorithm == ROUTING_DLADR ){
		if ( NoximGlobalParams::cascade_node ){
			if( NoximGlobalParams::beltway )
				name =name  + "_TTABR";
			else
				name =name  + "_TAAR";
		}
		else
			name =name  + "_DLADR";
	}
	else if ( NoximGlobalParams::routing_algorithm == ROUTING_DLAR  )
		name =name  + "_DLAR";
	else if ( NoximGlobalParams::routing_algorithm == ROUTING_DLDR  )
		name =name  + "_DLDR";
	else
		name =name  + "_routing-" + temperal;
	
	sprintf( temperal, "%d", NoximGlobalParams::selection_strategy);
	name  = name + "_sel-" + temperal;
	sprintf( temperal, "%d", NoximGlobalParams::dw_layer_sel);
	if      ( NoximGlobalParams::dw_layer_sel == DW_BL    )
		name =name  + "_BL";
	else if ( NoximGlobalParams::dw_layer_sel == DW_ODWL  )
		name =name  + "_ODWL";
	else if ( NoximGlobalParams::dw_layer_sel == DW_ADWL  )
		name =name  + "_ADWL";
	else if ( NoximGlobalParams::dw_layer_sel == DW_IPD   )
		name =name  + "_IPD";
	else if ( NoximGlobalParams::dw_layer_sel == DW_ODWL_IPD   )
		name =name  + "_ODWL_IPD";
	else if ( NoximGlobalParams::dw_layer_sel == DW_VBDR   )
		name =name  + "_VBDR";
	else
	name  = name + "_DW-sel-" + temperal;
	sprintf( temperal, "%f", NoximGlobalParams::packet_injection_rate);
	name = name  + "_pir-" + temperal;
	sprintf( temperal, "%d", NoximGlobalParams::traffic_distribution);
	name = name + "_traffic-" + temperal + +".txt";
	return name;
}

#endif
