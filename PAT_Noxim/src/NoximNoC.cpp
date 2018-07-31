/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the Network-on-Chip
 */
#include "NoximNoC.h"
#include "NoximGlobalStats.h"
#include <sys/stat.h>
//#include <direct.h>

using namespace std;

bool throttling[20][20][10];
int beltway[20][20][10];
double temp_budget[20][20][10];	  // Derek 2012.10.16
float thermal_factor[20][20][10];  // Derek 2012.12.14
float penalty_factor[20][20][10];  // Derek 2012.12.17
float MTTT[20][20][10];
int traffic[20][20][10];
int num_pkt = 2800;
double consumption_rate[20][20][10];

extern ofstream transient_log_throughput;
extern ofstream transient_topology;
extern ofstream static_log_power;
extern ofstream temp_tei_power;
extern ofstream results_log_pwr_router;
extern ofstream results_log_pwr_mac;
extern ofstream results_log_pwr_mem;

void NoximNoC::buildMesh()
{
	cout<<"Start buildMesh..."<<endl;
    // Check for routing table availability
    if (NoximGlobalParams::routing_algorithm == ROUTING_TABLE_BASED)
	assert(grtable.load(NoximGlobalParams::routing_table_filename));
    // Check for traffic table availability
    if (NoximGlobalParams::traffic_distribution == TRAFFIC_TABLE_BASED)
	assert(gttable.load(NoximGlobalParams::traffic_table_filename));
    // Create the mesh as a matrix of tiles
	int i,j,k;
	char _name[20];
	for ( i = 0; i < NoximGlobalParams::mesh_dim_x; i++) 
	for ( j = 0; j < NoximGlobalParams::mesh_dim_y; j++){
		sprintf( _name, "VLink[%02d][%02d]", i, j);
		v[i][j] = new NoximVLink( _name );
		v[i][j]->clock(clock);
		v[i][j]->reset(reset);
		v[i][j]->setId( i + NoximGlobalParams::mesh_dim_x*j );
		for ( k = 0; k < NoximGlobalParams::mesh_dim_z; k++){
			// Create the single Tile with a proper name
			sprintf(_name, "Tile[%02d][%02d][%02d]", i, j, k);
			t[i][j][k] = new NoximTile(_name);
			// Tell to the router its coordinates
			t[i][j][k]->r->configure( xyz2Id( i , j , k ), NoximGlobalParams::stats_warm_up_time,
					NoximGlobalParams::buffer_depth, grtable);
			//cout<<" set NoC ..."<<endl;
			// Tell to the PE its coordinates
			t[i][j][k]->pe->local_id       = xyz2Id( i , j , k );
			t[i][j][k]->pe->traffic_table  = &gttable;	// Needed to choose destination
			t[i][j][k]->pe->never_transmit = (gttable.occurrencesAsSource(t[i][j][k]->pe->local_id) == 0);
			//cout<<"get set NoC ..."<<endl;
			// Map clock and reset
			t[i][j][k]->clock(clock);
			t[i][j][k]->reset(reset);
			
			////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////  Map Rx signals  
			for (int VC_num = 0; VC_num < DEFAULT_NUM_VC; VC_num++){
				t[i][j][k]->req_rx	             [DIRECTION_NORTH][VC_num](req_to_south       [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_rx	             [DIRECTION_NORTH][VC_num](ack_to_north       [i  ][j  ][k  ][VC_num]);
																							
				t[i][j][k]->req_rx 	             [DIRECTION_EAST ][VC_num](req_to_west        [i+1][j  ][k  ][VC_num]);
				t[i][j][k]->ack_rx               [DIRECTION_EAST ][VC_num](ack_to_east        [i+1][j  ][k  ][VC_num]);
																							
				t[i][j][k]->req_rx 	             [DIRECTION_SOUTH][VC_num](req_to_north       [i  ][j+1][k  ][VC_num]);
				t[i][j][k]->ack_rx               [DIRECTION_SOUTH][VC_num](ack_to_south       [i  ][j+1][k  ][VC_num]);
																				
				t[i][j][k]->req_rx               [DIRECTION_WEST ][VC_num](req_to_east        [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_rx               [DIRECTION_WEST ][VC_num](ack_to_west        [i  ][j  ][k  ][VC_num]);
			
				//To VLink																	
				t[i][j][k]->req_rx               [DIRECTION_UP   ][VC_num](req_toT_down       [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_rx               [DIRECTION_UP   ][VC_num](ack_toV_up         [i  ][j  ][k  ][VC_num]);
				//To VLink																			
				t[i][j][k]->req_rx               [DIRECTION_DOWN ][VC_num] (req_toT_up        [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_rx               [DIRECTION_DOWN ][VC_num] (ack_toV_down      [i  ][j  ][k  ][VC_num]);
			
				// Map Tx signals                                                    
				t[i][j][k]->req_tx              [DIRECTION_NORTH][VC_num](req_to_north       [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_tx              [DIRECTION_NORTH][VC_num](ack_to_south       [i  ][j  ][k  ][VC_num]);
																				
				t[i][j][k]->req_tx              [DIRECTION_EAST ][VC_num](req_to_east        [i+1][j  ][k  ][VC_num]);
				t[i][j][k]->ack_tx              [DIRECTION_EAST ][VC_num](ack_to_west        [i+1][j  ][k  ][VC_num]);
																				
				t[i][j][k]->req_tx              [DIRECTION_SOUTH][VC_num](req_to_south       [i  ][j+1][k  ][VC_num]);
				t[i][j][k]->ack_tx              [DIRECTION_SOUTH][VC_num](ack_to_north       [i  ][j+1][k  ][VC_num]);
																				
				t[i][j][k]->req_tx              [DIRECTION_WEST ][VC_num](req_to_west        [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_tx              [DIRECTION_WEST ][VC_num](ack_to_east        [i  ][j  ][k  ][VC_num]);
			
				//To VLink																	
				t[i][j][k]->req_tx              [DIRECTION_UP   ][VC_num](req_toV_up         [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_tx              [DIRECTION_UP   ][VC_num](ack_toT_down       [i  ][j  ][k  ][VC_num]);
				//To VLink														     
				t[i][j][k]->req_tx              [DIRECTION_DOWN ][VC_num](req_toV_down       [i  ][j  ][k  ][VC_num]);
				t[i][j][k]->ack_tx              [DIRECTION_DOWN ][VC_num](ack_toT_up         [i  ][j  ][k  ][VC_num]);
			}
			t[i][j][k]->flit_rx	             [DIRECTION_NORTH]	(flit_to_south      [i  ][j  ][k  ]);
			t[i][j][k]->flit_rx	             [DIRECTION_EAST ]	(flit_to_west       [i+1][j  ][k  ]);
			t[i][j][k]->flit_rx              [DIRECTION_SOUTH]	(flit_to_north      [i  ][j+1][k  ]);
			t[i][j][k]->flit_rx              [DIRECTION_WEST ]	(flit_to_east       [i  ][j  ][k  ]);
			t[i][j][k]->flit_rx              [DIRECTION_UP   ]	(flit_toT_down      [i  ][j  ][k  ]);
			t[i][j][k]->flit_rx              [DIRECTION_DOWN ]	(flit_toT_up        [i  ][j  ][k  ]);
			t[i][j][k]->flit_tx              [DIRECTION_NORTH]	(flit_to_north      [i  ][j  ][k  ]);
			t[i][j][k]->flit_tx              [DIRECTION_EAST ]	(flit_to_east       [i+1][j  ][k  ]);
			t[i][j][k]->flit_tx              [DIRECTION_SOUTH]	(flit_to_south      [i  ][j+1][k  ]);
			t[i][j][k]->flit_tx              [DIRECTION_WEST ]	(flit_to_west       [i  ][j  ][k  ]);
			t[i][j][k]->flit_tx              [DIRECTION_UP   ]	(flit_toV_up        [i  ][j  ][k  ]);
			t[i][j][k]->flit_tx              [DIRECTION_DOWN ]	(flit_toV_down      [i  ][j  ][k  ]);
			////////////////////////////////////////////////  End
			////////////////////////////////////////////////////////////////////////////////////////////////

			// Map buffer level signals (analogy with req_tx/rx port mapping)
			t[i][j][k]->free_slots         [DIRECTION_NORTH] (free_slots_to_north[i  ][j  ][k  ]);
			t[i][j][k]->free_slots         [DIRECTION_EAST ] (free_slots_to_east [i+1][j  ][k  ]);
			t[i][j][k]->free_slots         [DIRECTION_SOUTH] (free_slots_to_south[i  ][j+1][k  ]);
			t[i][j][k]->free_slots         [DIRECTION_WEST ] (free_slots_to_west [i  ][j  ][k  ]);
			t[i][j][k]->free_slots         [DIRECTION_UP   ] (free_slots_to_up   [i  ][j  ][k  ]);
			t[i][j][k]->free_slots         [DIRECTION_DOWN ] (free_slots_to_down [i  ][j  ][k+1]);
			
			t[i][j][k]->free_slots_neighbor[DIRECTION_NORTH] (free_slots_to_south[i  ][j  ][k  ]);
			t[i][j][k]->free_slots_neighbor[DIRECTION_EAST ] (free_slots_to_west [i+1][j  ][k  ]);
			t[i][j][k]->free_slots_neighbor[DIRECTION_SOUTH] (free_slots_to_north[i  ][j+1][k  ]);
			t[i][j][k]->free_slots_neighbor[DIRECTION_WEST ] (free_slots_to_east [i  ][j  ][k  ]);
			t[i][j][k]->free_slots_neighbor[DIRECTION_UP   ] (free_slots_to_down [i  ][j  ][k  ]);
			t[i][j][k]->free_slots_neighbor[DIRECTION_DOWN ] (free_slots_to_up   [i  ][j  ][k+1]);
			////added by taheri

			// NoP 
			t[i][j][k]->NoP_data_out       [DIRECTION_NORTH] (NoP_data_to_north  [i  ][j  ][k  ]);
			t[i][j][k]->NoP_data_out       [DIRECTION_EAST ] (NoP_data_to_east   [i+1][j  ][k  ]);
			t[i][j][k]->NoP_data_out       [DIRECTION_SOUTH] (NoP_data_to_south  [i  ][j+1][k  ]);
			t[i][j][k]->NoP_data_out       [DIRECTION_WEST ] (NoP_data_to_west   [i  ][j  ][k  ]);
			t[i][j][k]->NoP_data_out       [DIRECTION_UP   ] (NoP_data_to_up     [i  ][j  ][k  ]);
			t[i][j][k]->NoP_data_out       [DIRECTION_DOWN ] (NoP_data_to_down   [i  ][j  ][k+1]);
																				
			t[i][j][k]->NoP_data_in        [DIRECTION_NORTH] (NoP_data_to_south  [i  ][j  ][k  ]);
			t[i][j][k]->NoP_data_in        [DIRECTION_EAST ] (NoP_data_to_west   [i+1][j  ][k  ]);
			t[i][j][k]->NoP_data_in        [DIRECTION_SOUTH] (NoP_data_to_north  [i  ][j+1][k  ]);
			t[i][j][k]->NoP_data_in        [DIRECTION_WEST ] (NoP_data_to_east   [i  ][j  ][k  ]);
			t[i][j][k]->NoP_data_in        [DIRECTION_UP   ] (NoP_data_to_down   [i  ][j  ][k  ]);
			t[i][j][k]->NoP_data_in        [DIRECTION_DOWN ] (NoP_data_to_up     [i  ][j  ][k+1]);
			// Derek @2012.03.07
			t[i][j][k]->RCA_data_out[DIRECTION_NORTH*2+0](RCA_data_to_north0[i][j][k]);
			t[i][j][k]->RCA_data_out[DIRECTION_NORTH*2+1](RCA_data_to_north1[i][j][k]);
			t[i][j][k]->RCA_data_out[DIRECTION_EAST*2+0](RCA_data_to_east0[i+1][j][k]);
			t[i][j][k]->RCA_data_out[DIRECTION_EAST*2+1](RCA_data_to_east1[i+1][j][k]);
			t[i][j][k]->RCA_data_out[DIRECTION_SOUTH*2+0](RCA_data_to_south0[i][j+1][k]);
			t[i][j][k]->RCA_data_out[DIRECTION_SOUTH*2+1](RCA_data_to_south1[i][j+1][k]);
			t[i][j][k]->RCA_data_out[DIRECTION_WEST*2+0](RCA_data_to_west0[i][j][k]);
			t[i][j][k]->RCA_data_out[DIRECTION_WEST*2+1](RCA_data_to_west1[i][j][k]);
            
			t[i][j][k]->RCA_data_in[DIRECTION_NORTH*2+0](RCA_data_to_south1[i][j][k]);    //***0 1 inverse
			t[i][j][k]->RCA_data_in[DIRECTION_NORTH*2+1](RCA_data_to_south0[i][j][k]);    //***0 1 inverse
			t[i][j][k]->RCA_data_in[DIRECTION_EAST*2+0](RCA_data_to_west1[i+1][j][k]);    //***0 1 inverse
			t[i][j][k]->RCA_data_in[DIRECTION_EAST*2+1](RCA_data_to_west0[i+1][j][k]);    //***0 1 inverse
			t[i][j][k]->RCA_data_in[DIRECTION_SOUTH*2+0](RCA_data_to_north1[i][j+1][k]);    //***0 1 inverse
			t[i][j][k]->RCA_data_in[DIRECTION_SOUTH*2+1](RCA_data_to_north0[i][j+1][k]);    //***0 1 inverse
			t[i][j][k]->RCA_data_in[DIRECTION_WEST*2+0](RCA_data_to_east1[i][j][k]);    //***0 1 inverse
			t[i][j][k]->RCA_data_in[DIRECTION_WEST*2+1](RCA_data_to_east0[i][j][k]);    //***0 1 inverse			
			
			t[i][j][k]->monitor_out        [DIRECTION_NORTH] (RCA_to_north       [i  ][j  ][k  ]);
			t[i][j][k]->monitor_out        [DIRECTION_EAST ] (RCA_to_east        [i+1][j  ][k  ]);
			t[i][j][k]->monitor_out        [DIRECTION_SOUTH] (RCA_to_south       [i  ][j+1][k  ]);
			t[i][j][k]->monitor_out        [DIRECTION_WEST ] (RCA_to_west        [i  ][j  ][k  ]);
			t[i][j][k]->monitor_out        [DIRECTION_UP   ] (RCA_to_up          [i  ][j  ][k  ]);
			t[i][j][k]->monitor_out        [DIRECTION_DOWN ] (RCA_to_down        [i  ][j  ][k+1]);
			t[i][j][k]->monitor_in         [DIRECTION_NORTH] (RCA_to_south       [i  ][j  ][k  ]);
			t[i][j][k]->monitor_in         [DIRECTION_EAST ] (RCA_to_west        [i+1][j  ][k  ]);
			t[i][j][k]->monitor_in         [DIRECTION_SOUTH] (RCA_to_north       [i  ][j+1][k  ]);
			t[i][j][k]->monitor_in         [DIRECTION_WEST ] (RCA_to_east        [i  ][j  ][k  ]);
			t[i][j][k]->monitor_in         [DIRECTION_UP   ] (RCA_to_down        [i  ][j  ][k  ]);
			t[i][j][k]->monitor_in         [DIRECTION_DOWN ] (RCA_to_up          [i  ][j  ][k+1]);
			
			// on/off ports in emergency mode
			t[i][j][k]->on_off             [DIRECTION_NORTH] (on_off_to_north    [i  ][j  ][k  ]);
			t[i][j][k]->on_off             [DIRECTION_EAST ] (on_off_to_east     [i+1][j  ][k  ]);
			t[i][j][k]->on_off             [DIRECTION_SOUTH] (on_off_to_south    [i  ][j+1][k  ]);
			t[i][j][k]->on_off             [DIRECTION_WEST ] (on_off_to_west     [i  ][j  ][k  ]);
			t[i][j][k]->on_off             [DIRECTION_UP ]    (on_off_to_up      [i  ][j  ][k  ]);
			t[i][j][k]->on_off             [DIRECTION_DOWN ] (on_off_to_down    [i  ][j  ][k+1]);
			
			t[i][j][k]->on_off_neighbor    [DIRECTION_NORTH] (on_off_to_south    [i  ][j  ][k  ]);
			t[i][j][k]->on_off_neighbor    [DIRECTION_EAST ] (on_off_to_west     [i+1][j  ][k  ]);
			t[i][j][k]->on_off_neighbor    [DIRECTION_SOUTH] (on_off_to_north    [i  ][j+1][k  ]);
			t[i][j][k]->on_off_neighbor    [DIRECTION_WEST ] (on_off_to_east     [i  ][j  ][k  ]);
			t[i][j][k]->on_off_neighbor    [DIRECTION_UP]    (on_off_to_down     [i  ][j  ][k  ]);
			t[i][j][k]->on_off_neighbor    [DIRECTION_DOWN ] (on_off_to_up       [i  ][j  ][k+1]);
			
			
			// Thermal budget
			t[i][j][k]->TB             [DIRECTION_NORTH] (TB_to_north    [i  ][j  ][k  ]);
			t[i][j][k]->TB             [DIRECTION_EAST ] (TB_to_east     [i+1][j  ][k  ]);
			t[i][j][k]->TB             [DIRECTION_SOUTH] (TB_to_south    [i  ][j+1][k  ]);
			t[i][j][k]->TB             [DIRECTION_WEST ] (TB_to_west     [i  ][j  ][k  ]);
			t[i][j][k]->TB             [DIRECTION_UP ]    (TB_to_up      [i  ][j  ][k  ]);
			t[i][j][k]->TB             [DIRECTION_DOWN ] (TB_to_down    [i  ][j  ][k+1]);
			
			t[i][j][k]->TB_neighbor    [DIRECTION_NORTH] (TB_to_south    [i  ][j  ][k  ]);
			t[i][j][k]->TB_neighbor    [DIRECTION_EAST ] (TB_to_west     [i+1][j  ][k  ]);
			t[i][j][k]->TB_neighbor    [DIRECTION_SOUTH] (TB_to_north    [i  ][j+1][k  ]);
			t[i][j][k]->TB_neighbor    [DIRECTION_WEST ] (TB_to_east     [i  ][j  ][k  ]);
			t[i][j][k]->TB_neighbor    [DIRECTION_UP]    (TB_to_down     [i  ][j  ][k  ]);
			t[i][j][k]->TB_neighbor    [DIRECTION_DOWN ] (TB_to_up       [i  ][j  ][k+1]);
		
			// predict temp.
                        t[i][j][k]->PDT             [DIRECTION_NORTH] (PDT_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->PDT             [DIRECTION_EAST ] (PDT_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->PDT             [DIRECTION_SOUTH] (PDT_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->PDT             [DIRECTION_WEST ] (PDT_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->PDT             [DIRECTION_UP ]   (PDT_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->PDT             [DIRECTION_DOWN ] (PDT_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->PDT_neighbor    [DIRECTION_NORTH] (PDT_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->PDT_neighbor    [DIRECTION_EAST ] (PDT_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->PDT_neighbor    [DIRECTION_SOUTH] (PDT_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->PDT_neighbor    [DIRECTION_WEST ] (PDT_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->PDT_neighbor    [DIRECTION_UP]    (PDT_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->PDT_neighbor    [DIRECTION_DOWN ] (PDT_to_up       [i  ][j  ][k+1]);
	
			//buffer information
						t[i][j][k]->buf[0]             [DIRECTION_NORTH] (buf0_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[0]             [DIRECTION_EAST ] (buf0_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[0]             [DIRECTION_SOUTH] (buf0_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[0]             [DIRECTION_WEST ] (buf0_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[0]             [DIRECTION_UP ]   (buf0_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[0]             [DIRECTION_DOWN ] (buf0_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[0]    [DIRECTION_NORTH] (buf0_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[0]    [DIRECTION_EAST ] (buf0_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[0]    [DIRECTION_SOUTH] (buf0_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[0]    [DIRECTION_WEST ] (buf0_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[0]    [DIRECTION_UP]    (buf0_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[0]    [DIRECTION_DOWN ] (buf0_to_up       [i  ][j  ][k+1]);

			//buffer information
                        t[i][j][k]->buf[1]             [DIRECTION_NORTH] (buf1_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[1]             [DIRECTION_EAST ] (buf1_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[1]             [DIRECTION_SOUTH] (buf1_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[1]             [DIRECTION_WEST ] (buf1_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[1]             [DIRECTION_UP ]   (buf1_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[1]             [DIRECTION_DOWN ] (buf1_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[1]    [DIRECTION_NORTH] (buf1_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[1]    [DIRECTION_EAST ] (buf1_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[1]    [DIRECTION_SOUTH] (buf1_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[1]    [DIRECTION_WEST ] (buf1_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[1]    [DIRECTION_UP]    (buf1_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[1]    [DIRECTION_DOWN ] (buf1_to_up       [i  ][j  ][k+1]);

			//buffer information
                        t[i][j][k]->buf[2]             [DIRECTION_NORTH] (buf2_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[2]             [DIRECTION_EAST ] (buf2_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[2]             [DIRECTION_SOUTH] (buf2_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[2]             [DIRECTION_WEST ] (buf2_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[2]             [DIRECTION_UP ]   (buf2_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[2]             [DIRECTION_DOWN ] (buf2_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[2]    [DIRECTION_NORTH] (buf2_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[2]    [DIRECTION_EAST ] (buf2_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[2]    [DIRECTION_SOUTH] (buf2_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[2]    [DIRECTION_WEST ] (buf2_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[2]    [DIRECTION_UP]    (buf2_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[2]    [DIRECTION_DOWN ] (buf2_to_up       [i  ][j  ][k+1]);

			//buffer information
                        t[i][j][k]->buf[3]             [DIRECTION_NORTH] (buf3_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[3]             [DIRECTION_EAST ] (buf3_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[3]             [DIRECTION_SOUTH] (buf3_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[3]             [DIRECTION_WEST ] (buf3_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[3]             [DIRECTION_UP ]   (buf3_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[3]             [DIRECTION_DOWN ] (buf3_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[3]    [DIRECTION_NORTH] (buf3_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[3]    [DIRECTION_EAST ] (buf3_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[3]    [DIRECTION_SOUTH] (buf3_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[3]    [DIRECTION_WEST ] (buf3_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[3]    [DIRECTION_UP]    (buf3_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[3]    [DIRECTION_DOWN ] (buf3_to_up       [i  ][j  ][k+1]);

			//buffer information
                        t[i][j][k]->buf[4]             [DIRECTION_NORTH] (buf4_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[4]             [DIRECTION_EAST ] (buf4_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[4]             [DIRECTION_SOUTH] (buf4_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[4]             [DIRECTION_WEST ] (buf4_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[4]             [DIRECTION_UP ]   (buf4_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[4]             [DIRECTION_DOWN ] (buf4_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[4]    [DIRECTION_NORTH] (buf4_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[4]    [DIRECTION_EAST ] (buf4_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[4]    [DIRECTION_SOUTH] (buf4_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[4]    [DIRECTION_WEST ] (buf4_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[4]    [DIRECTION_UP]    (buf4_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[4]    [DIRECTION_DOWN ] (buf4_to_up       [i  ][j  ][k+1]);

			//buffer information
                        t[i][j][k]->buf[5]             [DIRECTION_NORTH] (buf5_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[5]             [DIRECTION_EAST ] (buf5_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[5]             [DIRECTION_SOUTH] (buf5_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[5]             [DIRECTION_WEST ] (buf5_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[5]             [DIRECTION_UP ]   (buf5_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[5]             [DIRECTION_DOWN ] (buf5_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[5]    [DIRECTION_NORTH] (buf5_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[5]    [DIRECTION_EAST ] (buf5_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[5]    [DIRECTION_SOUTH] (buf5_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[5]    [DIRECTION_WEST ] (buf5_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[5]    [DIRECTION_UP]    (buf5_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[5]    [DIRECTION_DOWN ] (buf5_to_up       [i  ][j  ][k+1]);

			//buffer information
                        t[i][j][k]->buf[6]             [DIRECTION_NORTH] (buf6_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[6]             [DIRECTION_EAST ] (buf6_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[6]             [DIRECTION_SOUTH] (buf6_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[6]             [DIRECTION_WEST ] (buf6_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[6]             [DIRECTION_UP ]   (buf6_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[6]             [DIRECTION_DOWN ] (buf6_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[6]    [DIRECTION_NORTH] (buf6_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[6]    [DIRECTION_EAST ] (buf6_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[6]    [DIRECTION_SOUTH] (buf6_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[6]    [DIRECTION_WEST ] (buf6_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[6]    [DIRECTION_UP]    (buf6_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[6]    [DIRECTION_DOWN ] (buf6_to_up       [i  ][j  ][k+1]);

			//buffer information
                        t[i][j][k]->buf[7]             [DIRECTION_NORTH] (buf7_to_north    [i  ][j  ][k  ]);
                        t[i][j][k]->buf[7]             [DIRECTION_EAST ] (buf7_to_east     [i+1][j  ][k  ]);
                        t[i][j][k]->buf[7]             [DIRECTION_SOUTH] (buf7_to_south    [i  ][j+1][k  ]);
                        t[i][j][k]->buf[7]             [DIRECTION_WEST ] (buf7_to_west     [i  ][j  ][k  ]);
                        t[i][j][k]->buf[7]             [DIRECTION_UP ]   (buf7_to_up       [i  ][j  ][k  ]);
                        t[i][j][k]->buf[7]             [DIRECTION_DOWN ] (buf7_to_down     [i  ][j  ][k+1]);

                        t[i][j][k]->buf_neighbor[7]    [DIRECTION_NORTH] (buf7_to_south    [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[7]    [DIRECTION_EAST ] (buf7_to_west     [i+1][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[7]    [DIRECTION_SOUTH] (buf7_to_north    [i  ][j+1][k  ]);
                        t[i][j][k]->buf_neighbor[7]    [DIRECTION_WEST ] (buf7_to_east     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[7]    [DIRECTION_UP]    (buf7_to_down     [i  ][j  ][k  ]);
                        t[i][j][k]->buf_neighbor[7]    [DIRECTION_DOWN ] (buf7_to_up       [i  ][j  ][k+1]);

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////  NoximVlink
						if( k < NoximGlobalParams::mesh_dim_z - 1){
							for (int VC_num = 0; VC_num < DEFAULT_NUM_VC; VC_num++) {
								v[i][j]   ->ack_rx_to_UP   [k][VC_num](ack_toV_down  [i][j][k  ][VC_num] );
								v[i][j]   ->req_rx_to_UP   [k][VC_num](req_toT_up    [i][j][k  ][VC_num] );//output
								v[i][j]   ->ack_tx_to_UP   [k][VC_num](ack_toT_up    [i][j][k  ][VC_num] );//output
								v[i][j]   ->req_tx_to_UP   [k][VC_num](req_toV_down  [i][j][k  ][VC_num] );
				
								v[i][j]   ->ack_rx_to_DOWN [k][VC_num](ack_toT_down  [i][j][k+1][VC_num] );//output
								v[i][j]   ->req_rx_to_DOWN [k][VC_num](req_toV_up    [i][j][k+1][VC_num] );
								v[i][j]   ->ack_tx_to_DOWN [k][VC_num](ack_toV_up    [i][j][k+1][VC_num] );
								v[i][j]   ->req_tx_to_DOWN [k][VC_num](req_toT_down  [i][j][k+1][VC_num] );//output
							}
							v[i][j]   ->flit_rx_to_UP  [k](flit_toT_up   [i][j][k  ] );//output
							v[i][j]   ->flit_tx_to_UP  [k](flit_toV_down [i][j][k  ] );
							v[i][j]   ->flit_rx_to_DOWN[k](flit_toV_up   [i][j][k+1] );
							v[i][j]   ->flit_tx_to_DOWN[k](flit_toT_down [i][j][k+1] );//output
						}
////////////////////////////////////////////////  End
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
			//temp_budget[i][j][k]=10;
		}
	}
    // dummy NoximNoP_data structure
	NoximNoP_data tmp_NoP;
    tmp_NoP.sender_id = NOT_VALID;

    for ( i = 0; i < DIRECTIONS; i++) {
	tmp_NoP.channel_status_neighbor[i].free_slots = NOT_VALID;
	tmp_NoP.channel_status_neighbor[i].available  = false;
    }


	for( i=0; i<=NoximGlobalParams::mesh_dim_x; i++){
	for( k=0; k<=NoximGlobalParams::mesh_dim_z; k++){
		j = NoximGlobalParams::mesh_dim_y;
		///////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////    Clear signals for borderline nodes
		for (int VC_num = 0; VC_num < DEFAULT_NUM_VC; VC_num++) {
			req_to_south       [i][0][k][VC_num] = 0;
			ack_to_north       [i][0][k][VC_num] = 0;
			req_to_north       [i][j][k][VC_num] = 0;
			ack_to_south       [i][j][k][VC_num] = 0;
		}
		///////////////////////////////////////////////////////////////////////////////////////////
		free_slots_to_south[i][0][k].write(NOT_VALID);
		free_slots_to_north[i][j][k].write(NOT_VALID);
    
		RCA_to_south       [i][0][k].write(0);
		RCA_to_north       [i][j][k].write(0);
    
		// RCA Derek
		RCA_data_to_south0[i][0][k].write(0);
		RCA_data_to_south1[i][0][k].write(0);
		RCA_data_to_north0[i][NoximGlobalParams::mesh_dim_y][k].write(0);
		RCA_data_to_north1[i][NoximGlobalParams::mesh_dim_y][k].write(0);					
					
	
		on_off_to_south    [i][0][k].write(NOT_VALID);
		on_off_to_north    [i][j][k].write(NOT_VALID);
		
		TB_to_south    [i][0][k].write(NOT_VALID);
		TB_to_north    [i][j][k].write(NOT_VALID);

		buf0_to_south    [i][0][k].write(NOT_VALID); 
                buf0_to_north    [i][j][k].write(NOT_VALID);

		buf1_to_south    [i][0][k].write(NOT_VALID);
                buf1_to_north    [i][j][k].write(NOT_VALID);

		buf2_to_south    [i][0][k].write(NOT_VALID);
                buf2_to_north    [i][j][k].write(NOT_VALID);

		buf3_to_south    [i][0][k].write(NOT_VALID);
                buf3_to_north    [i][j][k].write(NOT_VALID);

		buf4_to_south    [i][0][k].write(NOT_VALID);
                buf4_to_north    [i][j][k].write(NOT_VALID);

		buf5_to_south    [i][0][k].write(NOT_VALID);
                buf5_to_north    [i][j][k].write(NOT_VALID);

		buf6_to_south    [i][0][k].write(NOT_VALID);
                buf6_to_north    [i][j][k].write(NOT_VALID);

		buf7_to_south    [i][0][k].write(NOT_VALID);
                buf7_to_north    [i][j][k].write(NOT_VALID);

		PDT_to_south    [i][0][k].write(100);
                PDT_to_north    [i][j][k].write(100);
    
		NoP_data_to_south  [i][0][k].write(tmp_NoP);
		NoP_data_to_north  [i][j][k].write(tmp_NoP);
		}
    }
	for( j=0; j<=NoximGlobalParams::mesh_dim_y; j++)
	for( k=0; k<=NoximGlobalParams::mesh_dim_z; k++){
			i = NoximGlobalParams::mesh_dim_x;
			/////////////////////////////////////////////////////////////////////////////////////
			for (int VC_num = 0; VC_num < DEFAULT_NUM_VC; VC_num++) {
				req_to_east       [0][j][k][VC_num] = 0;
				ack_to_west       [0][j][k][VC_num] = 0;
				req_to_west       [i][j][k][VC_num] = 0;
				ack_to_east       [i][j][k][VC_num] = 0;
			}
			/////////////////////////////////////////////////////////////////////////////////////
			free_slots_to_east[0][j][k].write(NOT_VALID);
			free_slots_to_west[i][j][k].write(NOT_VALID);
	
			// RCA 
			RCA_data_to_east0[0][j][k].write(0);
			RCA_data_to_east1[0][j][k].write(0);
			RCA_data_to_west0[NoximGlobalParams::mesh_dim_x][j][k].write(0);
			RCA_data_to_west1[NoximGlobalParams::mesh_dim_x][j][k].write(0);					
					
			RCA_to_east       [0][j][k].write(0);
			RCA_to_west       [i][j][k].write(0);

			on_off_to_east    [0][j][k].write(NOT_VALID);
			on_off_to_west    [i][j][k].write(NOT_VALID);
			
			TB_to_east    [0][j][k].write(NOT_VALID);
			TB_to_west    [i][j][k].write(NOT_VALID);

			buf0_to_east    [0][j][k].write(NOT_VALID);
                        buf0_to_west    [i][j][k].write(NOT_VALID);

			buf1_to_east    [0][j][k].write(NOT_VALID);
                        buf1_to_west    [i][j][k].write(NOT_VALID);

			buf2_to_east    [0][j][k].write(NOT_VALID);
                        buf2_to_west    [i][j][k].write(NOT_VALID);

			buf3_to_east    [0][j][k].write(NOT_VALID);
                        buf3_to_west    [i][j][k].write(NOT_VALID);

			buf4_to_east    [0][j][k].write(NOT_VALID);
                        buf4_to_west    [i][j][k].write(NOT_VALID);

			buf5_to_east    [0][j][k].write(NOT_VALID);
                        buf5_to_west    [i][j][k].write(NOT_VALID);

			buf6_to_east    [0][j][k].write(NOT_VALID);
                        buf6_to_west    [i][j][k].write(NOT_VALID);

			buf7_to_east    [0][j][k].write(NOT_VALID);
                        buf7_to_west    [i][j][k].write(NOT_VALID);

			PDT_to_east    [0][j][k].write(100);
                        PDT_to_west    [i][j][k].write(100);

			NoP_data_to_east  [0][j][k].write(tmp_NoP);
			NoP_data_to_west  [i][j][k].write(tmp_NoP);
	}
	for( i=0; i<=NoximGlobalParams::mesh_dim_x; i++){
	for( j=0; j<=NoximGlobalParams::mesh_dim_y; j++){
		k = NoximGlobalParams::mesh_dim_z;
		/////////////////////////////////////////////////////////////////////////////////////
		for (int VC_num = 0; VC_num < DEFAULT_NUM_VC; VC_num++) {
			req_toT_down       [i][j][0]	[VC_num] = 0;
			ack_toT_up         [i][j][k-1]	[VC_num] = 0;
			req_toT_up         [i][j][k-1]	[VC_num] = 0;
			ack_toT_down       [i][j][0]	[VC_num] = 0;
		}
		/////////////////////////////////////////////////////////////////////////////////////

    
		free_slots_to_down[i][j][0].write(NOT_VALID);
		free_slots_to_up  [i][j][k].write(NOT_VALID);
    
		RCA_to_down       [i][j][0].write(0);
		RCA_to_up         [i][j][k].write(0);
    
		NoP_data_to_down  [i][j][0].write(tmp_NoP);
		NoP_data_to_up    [i][j][k].write(tmp_NoP);
		
		on_off_to_down    [i][j][0].write(NOT_VALID);
		on_off_to_up    [i][j][k].write(NOT_VALID);

		TB_to_down    [i][j][0].write(NOT_VALID);
		TB_to_up    [i][j][k].write(NOT_VALID);

		buf0_to_down    [i][j][0].write(NOT_VALID);
                buf0_to_up    [i][j][k].write(NOT_VALID);

		buf1_to_down    [i][j][0].write(NOT_VALID);
                buf1_to_up    [i][j][k].write(NOT_VALID);

		buf2_to_down    [i][j][0].write(NOT_VALID);
                buf2_to_up    [i][j][k].write(NOT_VALID);

		buf3_to_down    [i][j][0].write(NOT_VALID);
                buf3_to_up    [i][j][k].write(NOT_VALID);

		buf4_to_down    [i][j][0].write(NOT_VALID);
                buf4_to_up    [i][j][k].write(NOT_VALID);

		buf5_to_down    [i][j][0].write(NOT_VALID);
                buf5_to_up    [i][j][k].write(NOT_VALID);

		buf6_to_down    [i][j][0].write(NOT_VALID);
                buf6_to_up    [i][j][k].write(NOT_VALID);

		buf7_to_down    [i][j][0].write(NOT_VALID);
                buf7_to_up    [i][j][k].write(NOT_VALID);
	
		PDT_to_down    [i][j][0].write(100);
                PDT_to_up    [i][j][k].write(100);
		}
    }
    // invalidate reservation table entries for non-exhistent channels
 	for( i=0; i<NoximGlobalParams::mesh_dim_x; i++)
	for( k=0; k<NoximGlobalParams::mesh_dim_z; k++){
		j = NoximGlobalParams::mesh_dim_y;
		t[i][0  ][k]->r->reservation_table.invalidate(DIRECTION_NORTH);
		t[i][j-1][k]->r->reservation_table.invalidate(DIRECTION_SOUTH);
	}
	for( j=0; j<NoximGlobalParams::mesh_dim_y; j++)
	for( k=0; k<NoximGlobalParams::mesh_dim_z; k++){
		i = NoximGlobalParams::mesh_dim_x;
		t[0  ][j][k]->r->reservation_table.invalidate(DIRECTION_WEST);
		t[i-1][j][k]->r->reservation_table.invalidate(DIRECTION_EAST);
	}   
	
	for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++)
	for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++)
	for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++){
		t[x][y][z]->vertical_free_slot_out(vertical_free_slot[x][y][z]);
		for( int i=0; i < NoximGlobalParams::mesh_dim_z; i++ )
			t[x][y][z]->vertical_free_slot_in[i]( vertical_free_slot[x][y][i] );
	}
	
	// Initial emergency mode 
	if(NoximGlobalParams::throt_type == THROT_TEST){
			cout<<"building throt sest!"<<endl;
			_throt_case_setting(NoximGlobalParams::dynamic_throt_case);
	}
	else{
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
		
			_setNormal(i,j,k);		
		}
	}
	int non_beltway_layer,non_throt_layer;
	int col_max,col_min,row_max,row_min;
	findNonXLayer(non_throt_layer,non_beltway_layer);
	calROC(col_max,col_min,row_max,row_min,non_beltway_layer);
	for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++ )
	for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++ ){
		t[x][y][z]->pe->RTM_set_var(non_throt_layer, non_beltway_layer, NoximGlobalParams::ROC_UP, NoximGlobalParams::ROC_DOWN, NoximGlobalParams::ROC_UP, NoximGlobalParams::ROC_DOWN);
	}
	//cout<<col_max<<" "<<col_min<<" "<<row_max<<" "<<row_min;
}

void NoximNoC::entry(){  //Foster big modified - 09/11/12
	//reset power
	if (reset.read()) {
		//in reset phase, reset power value 
		for(int k=0; k < NoximGlobalParams::mesh_dim_z; k++)
		for(int j=0; j < NoximGlobalParams::mesh_dim_y; j++)	
		for(int i=0; i < NoximGlobalParams::mesh_dim_x; i++){
			t[i][j][k]->r->stats.power.resetPwr();
			t[i][j][k]->r->stats.power.resetTransientPwr();
			//t[i][j][k]->r->stats.power.resetPwr();
			//t[i][j][k]->r->stats.power.resetTransientPwr();
			//t[i][j][k]->r->stats.temperature = INIT_TEMP - 273.15;
			t[i][j][k]->r->stats.pre_temperature1 = INIT_TEMP - 273.15;
			MTTT[i][j][k] = 10;
			traffic[i][j][k] = 0;
		}
		_emergency = false;
		_clean     = true;
	/*	if(!mkdir("results/Traffic",0777)) cout<<"Making new directory results/Hist"<<endl;
                string filename;
                filename = "results/Traffic/Traffic_analysis";
                filename = MarkFileName( filename );
	*/}
	else{
		int CurrentCycle    = getCurrentCycleNum();
		int CurrentCycleMod = (getCurrentCycleNum() % (int) (TEMP_REPORT_PERIOD));
		if(  CurrentCycleMod == ((int) (TEMP_REPORT_PERIOD) - NoximGlobalParams::clean_stage_time)){
			if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
				cout<<"% Set Clean Stage %"<<endl;
            TransientLog(); //taheri
			setCleanStage(); 
			num_pkt = 7168800;
		}

		//for progressBar
		if (CurrentCycle % (int)(NoximGlobalParams::simulation_time / 60) == 0) cout << "#"<<flush;

		if( CurrentCycleMod == 0 ){
			EndCleanStage();
			
			//accumulate steady power after warm-up time
			if( CurrentCycle > (int)( NoximGlobalParams::stats_warm_up_time ) )
				steadyPwr2PtraceFile();
			//Calculate Temperature
			if( NoximGlobalParams::cal_temp ){
				transPwr2PtraceFile();
				HS_interface->Temperature_calc(instPowerTrace, TemperatureTrace);
				setTemperature();

				//temperature effect inversion (TEI)
				//added by A.Norollah -- 2017.July.21
				double total = 0;
				for (int z = 0; z < NoximGlobalParams::mesh_dim_z; z++)
					for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
						for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++) {
							total += TemperatureTrace[3 * xyz2Id(x, y, z)];
						}
				temp_tei_power << getCurrentCycleNum() << "\t\t";
				temp_tei_power << total/(NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y*NoximGlobalParams::mesh_dim_z) << "\t\t";
				total = 0;
				for (int z = 0; z < NoximGlobalParams::mesh_dim_z; z++)
					for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
						for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++) {
							t[x][y][z]->r->stats.power.staticPowerUnderTemperature(TemperatureTrace[3 * xyz2Id(x, y, z)]);
							temp_tei_power << TemperatureTrace[3 * xyz2Id(x, y, z)] << "\t\t";
							total += t[x][y][z]->r->stats.power.getStaticPowerTransient();
						}
				temp_tei_power << "\n               ";
				static_log_power << getCurrentCycleNum() << "\t\t";
				static_log_power << total << "\t\t";
				for (int z = 0; z < NoximGlobalParams::mesh_dim_z; z++)
					for (int y = 0; y < NoximGlobalParams::mesh_dim_y; y++)
						for (int x = 0; x < NoximGlobalParams::mesh_dim_x; x++)
							static_log_power << t[x][y][z]->r->stats.power.getStaticPowerTransient() << "\t\t";
				static_log_power << "\n               ";
			}
		
		/*
		// debug---------------------------------
		// Derek 2012.10.16 
		cout << "--------------Temperature Difference/ Location Factor in Entry Function--------------"<<endl;
		int m, n, o;
		for( o = 0 ; o < NoximGlobalParams::mesh_dim_z ; o++ ){			
		for( n = NoximGlobalParams::mesh_dim_y - 1 ; n > -2  ; n-- ){
		for( m = 0 ; m < NoximGlobalParams::mesh_dim_x ; m++ ){
			if( n != -1 ){
					 cout << t[m][n][o]->r->stats.pre_temperature1 <<"\t";	
					}
					else
						cout << "0\t";
				}
				cout << "0\n";
			}
			cout<<"]"<<endl;
		}
		//-----------------------------------------
			
		cout << "--------------Buffer log--------------"<<endl;
		for(int i=0 ; i<6 ; i++){
			cout <<"Direction"<<i<<endl;
                for( o = 0 ; o < NoximGlobalParams::mesh_dim_z ; o++ ){
                for( n = NoximGlobalParams::mesh_dim_y - 1 ; n > -2  ; n-- ){
                for( m = 0 ; m < NoximGlobalParams::mesh_dim_x ; m++ ){
                        if( n != -1 ){
                                         cout << t[m][n][o]->r->buffer[i].GetMaxBufferSize() <<"\t";
                                        }
                                        else
                                                cout << "0\t";
                                }
                                cout << "0\n";
                        }
                        cout<<"]"<<endl;
                }}
                //-----------------------------------------
		*/
	
			//check temperature, whether set emergency mode or not
			EmergencyDecision();
			if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
				cout<<"% End Clean Stage %"<<endl;		
			TransientLog();//taheri
		}
		if( CurrentCycle == NoximGlobalParams::simulation_time && NoximGlobalParams::cal_temp){ //Calculate steady state temp.
			cout<<"\nCalculate SteadyTemp at "<<getCurrentCycleNum()<<endl;
			HS_interface->steadyTmp(t);
		} 
	
		/*
		int CurrentCycle    = getCurrentCycleNum();
		int CurrentCycleMod = (getCurrentCycleNum() % (int) (TEMP_REPORT_PERIOD));
		cout<<"CurrentCycle:"<<CurrentCycle<<"\r"<<flush;
		//if(  CurrentCycleMod == ((int) (TEMP_REPORT_PERIOD) - NoximGlobalParams::clean_stage_time)){
		//	setCleanStage();
		//}
		if( CurrentCycleMod == 0 ){
			//EndCleanStage();
			//accumulate steady power after warm-up time
			if( CurrentCycle > (int)( NoximGlobalParams::stats_warm_up_time ) )
				steadyPwr2PtraceFile();
			//Calculate Temperature
			if( NoximGlobalParams::cal_temp ){
				transPwr2PtraceFile();
				HS_interface->Temperature_calc(instPowerTrace, TemperatureTrace);
				setTemperature();
				cout<<getCurrentCycleNum()<<":Calc. Temp."<<endl;
			}
			//check temperature, whether set emergency mode or not
			//_emergency = EmergencyDecision();
			//if ( _emergency ){ 
				setCleanStage();
				_clean = false;
			//}
			//else {
			//	_clean = true;
			//	TransientLog();
			//}
			//TransientLog();
		}
		if( !_clean &&  ( CurrentCycleMod %  NoximGlobalParams::clean_stage_time == 0) ){
			if ( _CleanDone() ){
				EndCleanStage();
				_emergency = EmergencyDecision();
				TransientLog();
				_clean = true;
			}
			else{
				cout<<getCurrentCycleNum()<<":Clean stage fail."<<endl;
				_clean = false;
			}
		}
		if( CurrentCycle == NoximGlobalParams::simulation_time && NoximGlobalParams::cal_temp){ //Calculate steady state temp.
			cout<<"Calculate SteadyTemp at "<<getCurrentCycleNum()<<endl;
			HS_interface->steadyTmp(t);
		} 
	*/
	if(CurrentCycleMod < ((int) (TEMP_REPORT_PERIOD) - NoximGlobalParams::clean_stage_time))
	   if(getCurrentCycleNum() % 10000 == 0){

		if(!mkdir("results/Traffic",0777))
		//if (!_mkdir("results/Traffic"))
			cout<<"Making new directory results/Traffic"<<endl;
                
		string filename;
               	filename = "results/Traffic/Traffic_analysis";
               	filename = MarkFileName( filename );
		

		fstream dout;
		dout.open(filename.c_str(),ios::out|ios::app);

		dout<<"Cycletime: "<<getCurrentCycleNum()<<"\n";

		int increment;

		for( int o = 0 ; o < NoximGlobalParams::mesh_dim_z ; o++ ){
			dout<<"XY"<<o<<"=[\n";
                for( int n = 0 ; n < NoximGlobalParams::mesh_dim_y ; n++ ){
                for( int m = 0 ; m < NoximGlobalParams::mesh_dim_x ; m++ ){
			increment = t[m][n][o]->r->getRoutedFlits() - traffic[m][n][o];
			dout<<increment<<"\t";
		}
			dout<<"\n";
		}
			dout<<"]\n"<<"\n";
		}
		
		dout<<"color_range = [0 300000]"<<endl;
		dout<<"figure(1)"<<endl;

		int temp = 1;
                for( int k = 0 ; k < NoximGlobalParams::mesh_dim_z ; k++){
                        dout<<"subplot("<<NoximGlobalParams::mesh_dim_z<<",1,"<<temp<<"), pcolor(XY"<<k<<"), axis off, caxis( color_range ), colormap(jet)"<<endl;
                        temp += 1;
                }
                dout<<"set(gcf, 'PaperPosition', [1 1 7 30]);"<<endl;
                dout<<"print(gcf,'-djpeg','-r0','"<<MarkFileName( string("") )<<".jpg')"<<endl;
		//dout.close();
		
		for( int o = 0 ; o < NoximGlobalParams::mesh_dim_z ; o++ ){
                for( int n = 0 ; n < NoximGlobalParams::mesh_dim_y ; n++ ){
                for( int m = 0 ; m < NoximGlobalParams::mesh_dim_x ; m++ )
			traffic[m][n][o] = t[m][n][o]->r->getRoutedFlits();
		}
		}

	   }


	if(getCurrentCycleNum() % 100000 == 0){

		//if (!_mkdir("results/Traffic"))
		if(!mkdir("results/Traffic",0777))
                        cout<<"Making new directory results/Traffic"<<endl;

		string filename;
                filename = "results/Traffic/Traffic_analysis";
                filename = MarkFileName( filename );
		fstream dout;
                dout.open(filename.c_str(),ios::out|ios::app);

		dout<<"temper sampling :"<<getCurrentCycleNum()<<"\n";
		dout<<"total traffic\n";
		for( int o = 0 ; o < NoximGlobalParams::mesh_dim_z ; o++ ){
			dout<<"XY"<<o<<" = ["<<"\n";
                for( int n = 0 ; n < NoximGlobalParams::mesh_dim_y ; n++ ){
                for( int m = 0 ; m < NoximGlobalParams::mesh_dim_x ; m++ )
                        dout<<traffic[m][n][o]<<"\t";
		dout<<"\n";
                }
		dout<<"]\n"<<"\n";
                }

		dout<<"color_range = [0 300000]"<<endl;
                dout<<"figure(1)"<<endl;

                int temp = 1;
                for( int k = 0 ; k < NoximGlobalParams::mesh_dim_z ; k++){
                        dout<<"subplot("<<NoximGlobalParams::mesh_dim_z<<",1,"<<temp<<"), pcolor(XY"<<k<<"), axis off, caxis( color_range ), colormap(jet)"<<endl;
                        temp += 1;
                }
                dout<<"set(gcf, 'PaperPosition', [1 1 7 30]);"<<endl;
                dout<<"print(gcf,'-djpeg','-r0','"<<MarkFileName( string("") )<<".jpg')"<<endl;

	}
	}      
}

NoximTile *NoximNoC::searchNode(const int id) const{
	NoximCoord node = id2Coord(id);
	return t[node.x][node.y][node.z];
}

//----------Modified by CMH
void NoximNoC::transPwr2PtraceFile()
{
    int idx = 0;	
	int m, n, o;
	/*================================Begin of collecting POWER TRACE ======================================*/
	results_log_pwr_router << getCurrentCycleNum() << "\t\t";
	results_log_pwr_mem    << getCurrentCycleNum() << "\t\t";
	results_log_pwr_mac    << getCurrentCycleNum() << "\t\t";

	for(o=0; o < NoximGlobalParams::mesh_dim_z; o++)
	for(n=0; n < NoximGlobalParams::mesh_dim_y; n++)
	for(m=0; m < NoximGlobalParams::mesh_dim_x; m++){
		idx = xyz2Id( m, n, o);
		
		double a = t[m][n][o]->r->stats.power.getTransientRouterPower();
		//router : offset = 0
		//instPowerTrace[3*idx] = t[m][n][o]->r->stats.power.getTransientRouterPower()/(TEMP_REPORT_PERIOD *1e-9);
		//overallPowerTrace[3*idx] += instPowerTrace[3*idx];
		instPowerTrace[3*idx] = t[m][n][o]->r->stats.power.getTransientRouterPower();
		results_log_pwr_router << instPowerTrace[3*idx]<<"\t\t";
				
        //uP_mem : offset = 1
		//instPowerTrace[3*idx+1] = t[m][n][o]->r->stats.power.getTransientMEM()/(TEMP_REPORT_PERIOD *1e-9);
		//overallPowerTrace[3*idx+1] += instPowerTrace[3*idx+1];
		instPowerTrace[3*idx+1] = t[m][n][o]->r->stats.power.getTransientMEMPower();
		results_log_pwr_mem << instPowerTrace[3*idx+1]<<"\t\t";

		//uP_mac : offset = 2
		//instPowerTrace[3*idx+2] = t[m][n][o]->r->stats.power.getTransientFPMACPower()/(TEMP_REPORT_PERIOD *1e-9);
		//overallPowerTrace[3*idx+2] += instPowerTrace[3*idx+2];
		instPowerTrace[3*idx+2] = t[m][n][o]->r->stats.power.getTransientFPMACPower();
		results_log_pwr_mac << instPowerTrace[3*idx+2]<<"\t\t";

    	t[m][n][o]->r->stats.power.resetTransientPwr();
	}
	/*================================End of COLLECTING Power TRACE=================================================*/
	results_log_pwr_router << "\n               ";
	results_log_pwr_mem    << "\n               ";
	results_log_pwr_mac    << "\n               ";
}

void NoximNoC::steadyPwr2PtraceFile()
{
    int idx = 0;	
	int m, n, o;
	/*================================Begin of collecting POWER TRACE ======================================*/
	for(o=0; o < NoximGlobalParams::mesh_dim_z; o++)
	for(n=0; n < NoximGlobalParams::mesh_dim_y; n++)
	for(m=0; m < NoximGlobalParams::mesh_dim_x; m++){
        idx = xyz2Id( m, n, o);
		//router : offset = 0
		overallPowerTrace[3*idx  ] += t[m][n][o]->r->stats.power.getSteadyStateRouterPower();

        //uP_mem : offset = 1
		overallPowerTrace[3*idx+1] += t[m][n][o]->r->stats.power.getSteadyStateMEMPower   ();

		//uP_mac : offset = 2
		overallPowerTrace[3*idx+2] += t[m][n][o]->r->stats.power.getSteadyStateFPMACPower ();
	}
	/*================================End of COLLECTING Power TRACE=================================================*/
}

void NoximNoC::setTemperature(){
	int m, n, o;
    int idx = 0;
	// temperature prediction-----
	double current_temp; 
	double current_delta_temp; 
	double pre_delta_temp; 
	double pre_current_temp;
	double adjustment; 
	//double consumption_rate[20][20][4];	

	for(o=0; o < NoximGlobalParams::mesh_dim_z; o++)
	for(n=0; n < NoximGlobalParams::mesh_dim_y; n++) 
	for(m=0; m < NoximGlobalParams::mesh_dim_x; m++) {
		idx = xyz2Id( m, n, o);
		//set tile temperature
		t[m][n][o]->r->TBDB(consumption_rate[m][n][o]);
		t[m][n][o]->r->stats.last_temperature = t[m][n][o]->r->stats.temperature;
		t[m][n][o]->r->stats.temperature      = TemperatureTrace[3*idx];
		////taheri
        //thermal budget
		temp_budget[m][n][o]            	  = TEMP_THRESHOLD - t[m][n][o]->r->stats.temperature; // Derek 2012.10.16 	
		if (temp_budget[m][n][o]<0)
			temp_budget[m][n][o] = 0;	
		//thermal prediction
		if(t[m][n][o]->r->stats.temperature > 85)
		{
			current_temp = t[m][n][o]->r->stats.temperature;
			current_delta_temp = t[m][n][o]->r->stats.temperature - t[m][n][o]->r->stats.last_temperature;

			if(current_delta_temp < 0){
				pre_delta_temp = t[m][n][o]->r->stats.last_pre_temperature1 - t[m][n][o]->r->stats.last_temperature;
				pre_current_temp = t[m][n][o]->r->stats.last_pre_temperature1;
				adjustment = t[m][n][o]->r->stats.last_pre_temperature1 - current_temp;
				
				t[m][n][o]->r->stats.pre_temperature1 =  pre_current_temp + pre_delta_temp* exp(-1.98*0.01) - adjustment;
				//if(TEMP_THRESHOLD - t[m][n][o]->r->stats.pre_temperature1>0)
				consumption_rate[m][n][o] = t[m][n][o]->r->stats.pre_temperature1 - t[m][n][o]->r->stats.temperature; // Jason
				//else
				//	temp_budget[m][n][o]=0;
				/*t[m][n][o]->r->stats.pre_temperature2 =  pre_current_temp + pre_delta_temp* exp(-1.98*0.01) + pre_delta_temp* exp(-1.98*0.02) - adjustment;
				t[m][n][o]->r->stats.pre_temperature3 =  pre_current_temp + pre_delta_temp* exp(-1.98*0.01) + pre_delta_temp* exp(-1.98*0.02) +
				                                         pre_delta_temp* exp(-1.98*0.03) - adjustment;
				t[m][n][o]->r->stats.pre_temperature4 =  pre_current_temp + pre_delta_temp* exp(-1.98*0.01) + pre_delta_temp* exp(-1.98*0.02) +
				                                         pre_delta_temp* exp(-1.98*0.03) + pre_delta_temp* exp(-1.98*0.04) - adjustment;
				t[m][n][o]->r->stats.pre_temperature5 =  pre_current_temp + pre_delta_temp* exp(-1.98*0.01) + pre_delta_temp* exp(-1.98*0.02) +
				                                         pre_delta_temp* exp(-1.98*0.03) + pre_delta_temp* exp(-1.98*0.04) + 
														 pre_delta_temp* exp(-1.98*0.05) - adjustment;
				t[m][n][o]->r->stats.pre_temperature6 =  pre_current_temp + pre_delta_temp* exp(-1.98*0.01) + pre_delta_temp* exp(-1.98*0.02) +
				                                         pre_delta_temp* exp(-1.98*0.03) + pre_delta_temp* exp(-1.98*0.04) + 
														 pre_delta_temp* exp(-1.98*0.05) + pre_delta_temp* exp(-1.98*0.06) - adjustment;														 
			*/
			}
			//else t[m][n][o]->r->stats.pre_temperature =  current_temp + current_delta_temp* exp(-2.95*0.01) + current_delta_temp* exp(-2.95*0.02) + current_delta_temp* exp(-2.95*0.03) + current_delta_temp*exp(-2.95*0.04) + current_delta_temp* exp(-2.95*0.05);
			else{
				t[m][n][o]->r->stats.pre_temperature1 =  current_temp + current_delta_temp* exp(-1.98*0.01);
				//if(TEMP_THRESHOLD - t[m][n][o]->r->stats.pre_temperature1>0)
			        //	temp_budget[m][n][o]                  = TEMP_THRESHOLD - t[m][n][o]->r->stats.pre_temperature1; // Jason
				//else
				//	temp_budget[m][n][o]=0;
				consumption_rate[m][n][o] = t[m][n][o]->r->stats.pre_temperature1 - t[m][n][o]->r->stats.temperature;	
				/*
				t[m][n][o]->r->stats.pre_temperature2 =  current_temp + current_delta_temp* exp(-1.98*0.01) + current_delta_temp* exp(-1.98*0.02);
				t[m][n][o]->r->stats.pre_temperature3 =  current_temp + current_delta_temp* exp(-1.98*0.01) + current_delta_temp* exp(-1.98*0.02) +
				                                         current_delta_temp* exp(-1.98*0.03);
				t[m][n][o]->r->stats.pre_temperature4 =  current_temp + current_delta_temp* exp(-1.98*0.01) + current_delta_temp* exp(-1.98*0.02) +
				                                         current_delta_temp* exp(-1.98*0.03) + current_delta_temp* exp(-1.98*0.04);
				t[m][n][o]->r->stats.pre_temperature5 =  current_temp + current_delta_temp* exp(-1.98*0.01) + current_delta_temp* exp(-1.98*0.02) +
				                                         current_delta_temp* exp(-1.98*0.03) + current_delta_temp* exp(-1.98*0.04) + 
														 current_delta_temp* exp(-1.98*0.05);
				t[m][n][o]->r->stats.pre_temperature6 =  current_temp + current_delta_temp* exp(-1.98*0.01) + current_delta_temp* exp(-1.98*0.02) +
				                                         current_delta_temp* exp(-1.98*0.03) + current_delta_temp* exp(-1.98*0.04) + 
														 current_delta_temp* exp(-1.98*0.05) + current_delta_temp* exp(-1.98*0.06);														 
			*/
			}
			
			t[m][n][o]->r->stats.last_pre_temperature1 = t[m][n][o]->r->stats.pre_temperature1;
			/*t[m][n][o]->r->stats.last_pre_temperature2 = t[m][n][o]->r->stats.pre_temperature2;
			t[m][n][o]->r->stats.last_pre_temperature3 = t[m][n][o]->r->stats.pre_temperature3;
			t[m][n][o]->r->stats.last_pre_temperature4 = t[m][n][o]->r->stats.pre_temperature4;
			t[m][n][o]->r->stats.last_pre_temperature5 = t[m][n][o]->r->stats.pre_temperature5;
			t[m][n][o]->r->stats.last_pre_temperature6 = t[m][n][o]->r->stats.pre_temperature6;		
	        */
		}
		else
		{
			t[m][n][o]->r->stats.pre_temperature1 = t[m][n][o]->r->stats.temperature;
			/*t[m][n][o]->r->stats.pre_temperature2 = t[m][n][o]->r->stats.temperature;
			t[m][n][o]->r->stats.pre_temperature3 = t[m][n][o]->r->stats.temperature;
			t[m][n][o]->r->stats.pre_temperature4 = t[m][n][o]->r->stats.temperature;
			t[m][n][o]->r->stats.pre_temperature5 = t[m][n][o]->r->stats.temperature;
			t[m][n][o]->r->stats.pre_temperature6 = t[m][n][o]->r->stats.temperature;	
			*/	
		}
	}
	
	
	// Derek 2012.12.10
	// Thermal factor(location prone to be hot spot)

	for(o=0; o < NoximGlobalParams::mesh_dim_z; o++){
		thermal_factor[0][0][o]=1;   thermal_factor[1][0][o]=2;   thermal_factor[2][0][o]=3;   thermal_factor[3][0][o]=3;
		thermal_factor[4][0][o]=3;   thermal_factor[5][0][o]=2;   thermal_factor[6][0][o]=2;   thermal_factor[7][0][o]=1;		

		thermal_factor[0][1][o]=2;   thermal_factor[1][1][o]=4;   thermal_factor[2][1][o]=5;   thermal_factor[3][1][o]=5;
		thermal_factor[4][1][o]=5;   thermal_factor[5][1][o]=3;   thermal_factor[6][1][o]=2;   thermal_factor[7][1][o]=1;	

		thermal_factor[0][2][o]=3;   thermal_factor[1][2][o]=5;   thermal_factor[2][2][o]=6;   thermal_factor[3][2][o]=7;
		thermal_factor[4][2][o]=6;   thermal_factor[5][2][o]=5;   thermal_factor[6][2][o]=3;   thermal_factor[7][2][o]=2;		
		
		thermal_factor[0][3][o]=4;   thermal_factor[1][3][o]=6;   thermal_factor[2][3][o]=7;   thermal_factor[3][3][o]=8;
		thermal_factor[4][3][o]=7;   thermal_factor[5][3][o]=6;   thermal_factor[6][3][o]=4;   thermal_factor[7][3][o]=2;		

		thermal_factor[0][4][o]=4;   thermal_factor[1][4][o]=6;   thermal_factor[2][4][o]=7;   thermal_factor[3][4][o]=8;
		thermal_factor[4][4][o]=7;   thermal_factor[5][4][o]=6;   thermal_factor[6][4][o]=4;   thermal_factor[7][4][o]=2;		
		
		thermal_factor[0][5][o]=3;   thermal_factor[1][5][o]=5;   thermal_factor[2][5][o]=6;   thermal_factor[3][5][o]=7;
		thermal_factor[4][5][o]=6;   thermal_factor[5][5][o]=5;   thermal_factor[6][5][o]=3;   thermal_factor[7][5][o]=2;		

		thermal_factor[0][6][o]=2;   thermal_factor[1][6][o]=4;   thermal_factor[2][6][o]=5;   thermal_factor[3][6][o]=5;
		thermal_factor[4][6][o]=5;   thermal_factor[5][6][o]=4;   thermal_factor[6][6][o]=2;   thermal_factor[7][6][o]=1;		

		thermal_factor[0][7][o]=1;   thermal_factor[1][7][o]=2;   thermal_factor[2][7][o]=3;   thermal_factor[3][7][o]=3;
		thermal_factor[4][7][o]=3;   thermal_factor[5][7][o]=2;   thermal_factor[6][7][o]=2;   thermal_factor[7][7][o]=1;			
	}

	// Derek 2012.12.17
	// Penalty Factor(decline largely when temperature close to limit)

	for(o=0; o < NoximGlobalParams::mesh_dim_z; o++)
	for(n=0; n < NoximGlobalParams::mesh_dim_y; n++) 
	for(m=0; m < NoximGlobalParams::mesh_dim_x; m++) {	
		if( (t[m][n][o]->r->stats.pre_temperature1)<90 )
			penalty_factor[m][n][o]= 1- 0.1*(t[m][n][o]->r->stats.pre_temperature1-85);
		else if((t[m][n][o]->r->stats.pre_temperature1)>=90 && (t[m][n][o]->r->stats.pre_temperature1)<95)
			penalty_factor[m][n][o]= 0.5- 0.04*(t[m][n][o]->r->stats.pre_temperature1-90);
		else if((t[m][n][o]->r->stats.pre_temperature1)>=95 && (t[m][n][o]->r->stats.pre_temperature1)<100)
			penalty_factor[m][n][o]= 0.3- 0.06*(t[m][n][o]->r->stats.pre_temperature1-95);
		else
			penalty_factor[m][n][o]= 0;
	}

	
	// Total Thermal Budget
	for(o=0; o < NoximGlobalParams::mesh_dim_z; o++)
	for(n=0; n < NoximGlobalParams::mesh_dim_y; n++) 
	for(m=0; m < NoximGlobalParams::mesh_dim_x; m++) {	
		if(consumption_rate[m][n][o]>0){
		//	t[m][n][o]->r->TBDB(consumption_rate[m][n][o]);
			MTTT[m][n][o] = (temp_budget[m][n][o]/**penalty_factor[m][n][o]*/)/consumption_rate[m][n][o];	
		}
	}


}

bool NoximNoC::EmergencyDecision()
{	
	bool isEmergency = false;
	if     (NoximGlobalParams::throt_type == THROT_GLOBAL)
		GlobalThrottle(isEmergency);
	else if(NoximGlobalParams::throt_type == THROT_DISTRIBUTED)
		DistributedThrottle(isEmergency);
	else if(NoximGlobalParams::throt_type == THROT_TAVT)
		TAVT(isEmergency);
	else if(NoximGlobalParams::throt_type == THROT_TAVT_MAX)
		TAVT_MAX(isEmergency);
	else if(NoximGlobalParams::throt_type == THROT_VERTICAL)
		Vertical(isEmergency);
	else if(NoximGlobalParams::throt_type == THROT_VERTICAL_MAX)
		Vertical_MAX(isEmergency);
	else return isEmergency;//THROT_NORMAL,THROT_TEST do nothing, because the topology won't change
	return isEmergency;
}
void NoximNoC::GlobalThrottle(bool& isEmergency){
	for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++) 
	for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++)
		if(t[x][y][z]->r->stats.temperature > TEMP_THRESHOLD){ // each temperature of routers exceed temperature threshould
			isEmergency = true;
			break;
	}
	if(isEmergency){
	    for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++) 
	    for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++) 
	    for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++){
	    	t[x][y][z]->pe->IntoEmergency();
	    	t[x][y][z]->r ->IntoEmergency();
	    	throttling[x][y][z] = isEmergency; 
	    }
	}
	else{
		for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++) 
	    for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++) 
	    for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++){
	    	t[x][y][z]->pe->OutOfEmergency();
	    	t[x][y][z]->r ->OutOfEmergency();
	    	throttling[x][y][z] = isEmergency; 
	    }
	}
}
void NoximNoC::DistributedThrottle(bool& isEmergency){
	for(int z=0; z < NoximGlobalParams::mesh_dim_z - 1 ; z++)
	for(int y=0; y < NoximGlobalParams::mesh_dim_y     ; y++) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x     ; x++){
		if(t[x][y][z]->r->stats.temperature > TEMP_THRESHOLD){	
			isEmergency = true;
			t[x][y][z]->pe->IntoEmergency();
			t[x][y][z]->r ->IntoEmergency();
			throttling[x][y][z] = true;/*
			for(int zz=0; zz<z+1; zz++){
				t[x][y][zz]->pe->IntoEmergency();
	                        t[x][y][zz]->r ->IntoEmergency();
        	                throttling[x][y][zz] = true;
 			}*/
		}
		else{	
			t[x][y][z]->pe->OutOfEmergency();	
			t[x][y][z]->r ->OutOfEmergency();
			throttling[x][y][z] = false; 
		}
	}
	//taheri
	for(int z=0; z < NoximGlobalParams::mesh_dim_z - 1 ; z++)
	for(int y=0; y < NoximGlobalParams::mesh_dim_y     ; y++)
	for(int x=0; x < NoximGlobalParams::mesh_dim_x     ; x++){
		if(t[x][y][z]->r->stats.temperature > TEMP_THRESHOLD){
			for(int zz=0; zz < z ; zz++){
			t[x][y][zz]->pe->IntoEmergency();
			t[x][y][zz]->r ->IntoEmergency();
			throttling[x][y][zz] = true;
			}
		}
	}
}

void NoximNoC::TAVT(bool& isEmergency){
	for(int y=0; y < NoximGlobalParams::mesh_dim_y     ; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x     ; x++ )
	for(int z=0; z < NoximGlobalParams::mesh_dim_z - 1 ; z++ )
	{
		if (t[x][y][z]->r->stats.temperature < TEMP_THRESHOLD ){	
			t[x][y][z]->pe->OutOfEmergency();		
			t[x][y][z]->r ->OutOfEmergency();
			throttling[x][y][z] = 0; 
			if( t[x][y][z]->r->stats.temperature > 0/*NoximGlobalParams::beltway_trigger*/ && NoximGlobalParams::beltway )beltway[x][y][z]       = true;
			else	beltway[x][y][z]       = false;
		}
		else{ // >TEMP_THRESHOLD
			isEmergency = true;
			for( int zz = 0 ; zz < z + 1 ; zz++){
			// for( int zz = 0 ; zz < NoximGlobalParams::mesh_dim_z ; zz++){
				if(zz < NoximGlobalParams::mesh_dim_z-1){	//Bottom chip layer won't be throttle
					t[x][y][zz]->pe->IntoEmergency();
					t[x][y][zz]->r ->IntoEmergency();
					throttling[x][y][zz]       = 1;
				}
			}
			break;
		}
	}
	Reconfiguration();
}

void NoximNoC::TAVT_MAX(bool& isEmergency){
	for(int y=0; y < NoximGlobalParams::mesh_dim_y     ; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x     ; x++ )
	for(int z=0; z < NoximGlobalParams::mesh_dim_z     ; z++ )
	{
		if (t[x][y][z]->r->stats.temperature < TEMP_THRESHOLD ){	
			t[x][y][z]->pe->OutOfEmergency();		
			t[x][y][z]->r ->OutOfEmergency();
			throttling[x][y][z] = 0; 
			if( t[x][y][z]->r->stats.temperature > NoximGlobalParams::beltway_trigger && NoximGlobalParams::beltway )beltway[x][y][z]       = true;
			else                                                                					beltway[x][y][z]       = false;
		}
		else{ // >TEMP_THRESHOLD
			isEmergency = true;
			for( int zz = 0 ; zz < z + 1 ; zz++){
					t[x][y][zz]->pe->IntoEmergency();
					t[x][y][zz]->r ->IntoEmergency();
					throttling[x][y][zz]       = 1;
			}
			break;
		}
	}
	Reconfiguration();	
}

void NoximNoC::Vertical(bool& isEmergency){
	for(int y=0; y < NoximGlobalParams::mesh_dim_y     ; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x     ; x++ )
	for(int z=0; z < NoximGlobalParams::mesh_dim_z - 1 ; z++ )
	{
		if (t[x][y][z]->r->stats.temperature < TEMP_THRESHOLD ){	
			t[x][y][z]->pe->OutOfEmergency();		
			t[x][y][z]->r ->OutOfEmergency();
			throttling[x][y][z] = 0; 
			if( t[x][y][z]->r->stats.temperature > NoximGlobalParams::beltway_trigger && NoximGlobalParams::beltway )
				beltway[x][y][z] = true;
			else                                                                					
				beltway[x][y][z] = false;
		}
		else{ // >TEMP_THRESHOLD
			isEmergency = true;
			for( int zz = 0 ; zz < z + 1 ; zz++){
				if(zz < NoximGlobalParams::mesh_dim_z-1){	//Bottom chip layer won't be throttle
					t[x][y][zz]->pe->IntoEmergency();
					t[x][y][zz]->r ->IntoEmergency();
					throttling[x][y][zz]       = 1;
				}
			}
			break;
		}
	}
	Reconfiguration();
}

void NoximNoC::Vertical_MAX(bool& isEmergency){
	for(int y=0; y < NoximGlobalParams::mesh_dim_y     ; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x     ; x++ )
	for(int z=0; z < NoximGlobalParams::mesh_dim_z     ; z++ ){
		if (t[x][y][z]->r->stats.temperature < TEMP_THRESHOLD ){	
			t[x][y][z]->pe->OutOfEmergency();		
			t[x][y][z]->r ->OutOfEmergency();
			throttling[x][y][z] = 0; 
			if( t[x][y][z]->r->stats.temperature > NoximGlobalParams::beltway_trigger && NoximGlobalParams::beltway )beltway[x][y][z]       = true;
			else                                                                					beltway[x][y][z]       = false;
		}
		else{ // >TEMP_THRESHOLD
			isEmergency = true;
			for( int zz = 0 ; zz < NoximGlobalParams::mesh_dim_z ; zz++){
					t[x][y][zz]->pe->IntoEmergency();
					t[x][y][zz]->r ->IntoEmergency();
					throttling[x][y][zz]       = 1;
			}
			break;
		}
	}
	Reconfiguration();
}

void NoximNoC::calROC(int &col_max, int &col_min, int &row_max, int &row_min,int non_beltway_layer){
	int X_min = 0;
	int X_max = NoximGlobalParams::mesh_dim_x - 1;
	int Y_min = 0;
	int Y_max = NoximGlobalParams::mesh_dim_x - 1;
	int Z     = non_beltway_layer;
	
	int m, n, layer;
	bool index = false;
	for( n = 0 ; n < NoximGlobalParams::mesh_dim_y ; n++ ){
		for(layer=0; layer < Z; layer++) 
		for(m=0; m < NoximGlobalParams::mesh_dim_x; m++)
			index |= beltway[m][n][layer];
		if (index){
			Y_min = n ;
			break;
		}
	}
	index = 0;
	for( n = NoximGlobalParams::mesh_dim_y - 1 ; n > -1  ; n-- ){
		for(layer=0; layer < Z; layer++) 
		for(m=0; m < NoximGlobalParams::mesh_dim_x; m++)
			index |= beltway[m][n][layer];
		if (index){
			Y_max = n ;
			break;
		}
	}
	index = 0;
	for( m = 0 ; m < NoximGlobalParams::mesh_dim_x ; m++ ){
		for(layer=0; layer < Z; layer++) 
		for(n = Y_min ; n < Y_max + 1 ; n++)
			index |= beltway[m][n][layer];
		if (index){
			X_min = m ;
			break;
		}
	}
	index = 0;
	for( m = NoximGlobalParams::mesh_dim_x - 1 ; m > -1  ; m-- ){
		for(layer = 0; layer < Z; layer++) 
		for(n=Y_min ; n < Y_max + 1; n++){
			index |= beltway[m][n][layer];
			}
		if (index){
			X_max = m ;
			break;
		}
	}
	col_min = X_min;
	col_max = X_max;
	row_min = Y_min;
	row_max = Y_max;
}
void NoximNoC::setCleanStage(){
	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
	    cout<<getCurrentCycleNum()<<":Into Clean Stage"<<endl;
	for(int z=0; z < NoximGlobalParams::mesh_dim_z ; z++ )//from top to down
	for(int y=0; y < NoximGlobalParams::mesh_dim_y ; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x ; x++ ){
		t[x][y][z]->pe->IntoCleanStage();
		t[x][y][z]->pe->OutOfEmergency();
		t[x][y][z]->r ->OutOfEmergency();
		throttling[x][y][z]             = false;
	}
}

void NoximNoC::EndCleanStage(){
	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
	    cout<<getCurrentCycleNum()<<":Out of Clean Stage"<<endl;
	for(int z=0; z < NoximGlobalParams::mesh_dim_z ; z++ )//from top to down
	for(int y=0; y < NoximGlobalParams::mesh_dim_y ; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x ; x++ )
		t[x][y][z]->pe->OutOfCleanStage();
}

void NoximNoC::findNonXLayer(int &non_throt_layer, int &non_beltway_layer){
	int m, n, layer;
	bool index_throt = false,index_beltway = false;
	non_throt_layer = 0,non_beltway_layer = 0;
	for(layer=NoximGlobalParams::mesh_dim_z - 1 ; layer > -1 ; layer-- ){
		for(n=0; n < NoximGlobalParams::mesh_dim_y; n++) 
		for(m=0; m < NoximGlobalParams::mesh_dim_x; m++)
			index_beltway |= beltway[m][n][layer];
		if (index_beltway){
			non_beltway_layer = layer + 1;
			break;
		}
	}
	for(layer = NoximGlobalParams::mesh_dim_z - 1 ; layer > -1 ; layer-- ){
		for(n=0; n < NoximGlobalParams::mesh_dim_y; n++) 
		for(m=0; m < NoximGlobalParams::mesh_dim_x; m++)
			index_throt   |= throttling[m][n][layer];
		if (index_throt){
			non_throt_layer = layer + 1;
			break;
		}
	}
	assert( non_throt_layer > -1 && non_beltway_layer > -1 );
}
void NoximNoC::Reconfiguration(){
	int non_beltway_layer,non_throt_layer;
	int col_max,col_min,row_max,row_min;
	findNonXLayer(non_throt_layer,non_beltway_layer);
	calROC(col_max,col_min,row_max,row_min,non_beltway_layer);
	for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++ )
	for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++ ) 
	for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++ )
		t[x][y][z]->pe->RTM_set_var(non_throt_layer, non_beltway_layer, col_max, col_min, row_max, row_min);
	transient_topology<<"non_beltway_layer,non_throt_layer = "<<non_beltway_layer<<","<<non_throt_layer<<endl;
	transient_topology<<"col_max,col_min,row_max,row_min = "<<col_max<<","<<col_min<<","<<row_max<<","<<row_min<<endl;
}
bool NoximNoC::_equal(int x, int y, int z, int m, int n, int o){
	return ( x == m )&&( y == n )&&( z == o );
}

bool NoximNoC::_CleanDone(){
	bool clean = true;
	for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++)
	for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++)
	for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++){
	
		if( !t[x][y][z]->pe->flit_queue.empty() ){
			cout<<"t["<<x<<"]["<<y<<"]["<<z<<"] pe flit_queue is not empty "<<t[x][y][z]->pe->flit_queue.front()<<endl;
		}
		if( !t[x][y][z]->pe->packet_queue.empty() ){
			cout<<"t["<<x<<"]["<<y<<"]["<<z<<"] pe packet_queue is not empty "<<t[x][y][z]->pe->packet_queue.front().dst_id<<endl;
		}
		for(int d = 0; d < DIRECTIONS+1; d++)
			for (int vc = 0; vc < DEFAULT_NUM_VC; vc++){
			if ( !(t[x][y][z]->r->buffer[d][vc].IsEmpty()) ){
				clean = false;
				
				if( ((int)(t[x][y][z]->r->buffer[d][vc].Front().timestamp) % (int) (TEMP_REPORT_PERIOD) ) >  NoximGlobalParams::clean_stage_time){
					int output_channel = t[x][y][z]->r->getFlitRoute(d);
					cout<<"In node t["<<x<<"]["<<y<<"]["<<z<<"] direction "<<d<<" waiting time "<<getCurrentCycleNum() - t[x][y][z]->r->buffer[d][vc].Front().waiting_cnt<<" ";
					cout<<t[x][y][z]->r->buffer[d][vc].Front()<<" ";
					cout<<"This flit is route to "<<output_channel<<" ack("<</*t[x][y][z]->r->ack_tx[output_channel]<<*/") avalible("<<t[x][y][z]->r->getDirAvailable(output_channel, vc)<<")"<<endl;
				}
			}
		}
	}
	return clean;
}

void NoximNoC::TransientLog(){
	//calculate the period throughtput

	int packet_in_buffer           =0;
	int throttle_num               =0;
	float max_temp                 =0;
	int total_transmit             =0;
	int total_adaptive_transmit    =0;
	int total_dor_transmit         =0;
	int total_dw_transmit          =0;
	int total_mid_adaptive_transmit=0;
	int total_mid_dor_transmit     =0;
	int total_mid_dw_transmit      =0;
	int total_beltway              =0;
	int max_delay                  =0;
	int max_delay_id               =0;
	int max_delay_id_d             =0;
	
		
		for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++)
		for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++)
		for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++){
			//taheri
			/*if( !t[x][y][z]->pe->flit_queue.empty() ){
				cout<<"t["<<x<<"]["<<y<<"]["<<z<<"] pe flit_queue is not empty "<<t[x][y][z]->pe->flit_queue.front()<<", flit_queue size "<<t[x][y][z]->pe->flit_queue.size()<<endl;
			}
			if( !t[x][y][z]->pe->packet_queue.empty() ){
				cout<<"t["<<x<<"]["<<y<<"]["<<z<<"] pe packet_queue is not empty "<<t[x][y][z]->pe->packet_queue.front().dst_id<<", packet_queue size"<<t[x][y][z]->pe->packet_queue.size()<<endl;
			}*/
		for(int d = 0; d < DIRECTIONS+1; d++){
			if ( !(t[x][y][z]->r->buffer[d][0].IsEmpty()) ){
				packet_in_buffer++;
				//taheri
				//int output_channel = t[x][y][z]->r->getFlitRoute(d);

				//if( _emergency ){
					/*cout<<"In node t["<<x<<"]["<<y<<"]["<<z<<"] direction "<<d<<" waiting time "<<getCurrentCycleNum() - t[x][y][z]->r->buffer[d].Front().waiting_cnt<<" ";
					cout<<t[x][y][z]->r->buffer[d].Front()<<" ";
					cout<<"This flit is route to "<<output_channel<<" ack("<<t[x][y][z]->r->ack_tx[output_channel]<<") avalible("<<t[x][y][z]->r->getDirAvailable(output_channel)<<")"<<endl;
					cout<<"Reservation table:"<<d<<" route to "<<t[x][y][z]->r->reservation_table.getOutputPort(d)<<" ";
					cout<<"beltway: "<<t[x][y][z]->r->buffer[d].Front().beltway<<" ";
					cout<<"hop no.: "<<t[x][y][z]->r->buffer[d].Front().hop_no<<" ";
					cout<<"arr mid: "<<t[x][y][z]->r->buffer[d].Front().arr_mid<<" ";
					cout<<"arr mid: "<<t[x][y][z]->r->buffer[d].Front().arr_mid<<" ";
					cout<<"DW_layer: "<<t[x][y][z]->r->buffer[d].Front().DW_layer<<" ";
					cout<<endl;
					if( ((int)(t[x][y][z]->r->buffer[d].Front().timestamp) % (int) (TEMP_REPORT_PERIOD) ) > (int) (TEMP_REPORT_PERIOD) - NoximGlobalParams::clean_stage_time){
						cout<<"flit timestamp = "<<t[x][y][z]->r->buffer[d].Front().timestamp<<endl;
						//assert(false);
					//}
					//assert(false);*/
				//}
			}
		
			if( d == 0){
				if( throttling[x][y][z] )throttle_num++;
				max_temp = ( t[x][y][z]->r->stats.temperature > max_temp)?t[x][y][z]->r->stats.temperature:max_temp;
				// total_transmit             += t[x][y][z]->pe->getTransient_Total_Transmit();
				total_adaptive_transmit    += t[x][y][z]->pe->getTransient_Adaptive_Transmit();
				total_dor_transmit         += t[x][y][z]->pe->getTransient_DOR_Transmit();
				total_dw_transmit          += t[x][y][z]->pe->getTransient_DW_Transmit();
				total_mid_adaptive_transmit+= t[x][y][z]->pe->getTransient_Mid_Adaptive_Transmit();
				total_mid_dor_transmit     += t[x][y][z]->pe->getTransient_Mid_DOR_Transmit();
				total_mid_dw_transmit      += t[x][y][z]->pe->getTransient_Mid_DW_Transmit();
				total_beltway              += t[x][y][z]->pe->getTransient_Beltway_Transmit();
			}
		}
		t[x][y][z]->pe->ResetTransient_Transmit();
		}
		total_transmit = total_adaptive_transmit    + 
						 total_dor_transmit         +       
						 total_dw_transmit          +       
						 total_mid_adaptive_transmit+
						 total_mid_dor_transmit     +
						 total_mid_dw_transmit      +
						 total_beltway              ;	
		
	transient_log_throughput<<getCurrentCycleNum()<<"\t"
	                        <<total_transmit<<"\t"
							<<total_transmit/TEMP_REPORT_PERIOD<<"\t"
							<<throttle_num<<"\t"
							<<max_temp<<"\t"
							<<total_adaptive_transmit    <<"\t"
							<<total_dor_transmit         <<"\t"
							<<total_dw_transmit          <<"\t"
							<<total_mid_adaptive_transmit<<"\t"
							<<total_mid_dor_transmit     <<"\t"
							<<total_mid_dw_transmit      <<"\t"
							<<total_beltway              <<"\t"
							<<endl;
	transient_topology<<"Throttling Table @"<<getCurrentCycleNum()<<" "<<throttle_num<<" nodes are throttled, "<<packet_in_buffer<<" Non-Empty Buffers, Throughput "
				    <<total_transmit/TEMP_REPORT_PERIOD<<endl;
				for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++)
					transient_topology<<"Layer "<<z<<"\t";
				transient_topology<<"\n";
				for(int y=0; y < NoximGlobalParams::mesh_dim_y; y++){
					for(int z=0; z < NoximGlobalParams::mesh_dim_z; z++){
						for(int x=0; x < NoximGlobalParams::mesh_dim_x; x++){
							if(throttling[x][y][z])transient_topology<<"X";
							else if( beltway[x][y][z] )transient_topology<<"*";
							else transient_topology<<".";
						}
						transient_topology<<"\t";	
					}
					transient_topology<<endl;			
				}
}

void NoximNoC::_setThrot(int i, int j, int k){
	throttling[i][j][k] = 1;
	t[i][j][k]->pe->IntoEmergency();
	t[i][j][k]->r ->IntoEmergency();
}

void NoximNoC::_setNormal(int i, int j, int k){
	throttling[i][j][k] = 0;
	t[i][j][k]->pe->OutOfEmergency();
	t[i][j][k]->r ->OutOfEmergency();
}

void NoximNoC::_throt_case_setting( int throt_case ){
	switch( throt_case ){
	case 1 ://normal
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++)
			_setNormal(i,j,k);
	break;
	case 2 :
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ..X.....        ..X.....        ........        ........
		// ..X.....        ..X.....        ........        ........
		// ....X...        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(2,2,0,i,j,k) || _equal(2,3,0,i,j,k) || _equal(4,4,0,i,j,k) ||
				_equal(2,2,1,i,j,k) || _equal(2,3,1,i,j,k) )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 3 :
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ..X.....        ..X.....        ........        ........
		// ..X.....        ..X.....        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(2,2,0,i,j,k) || _equal(2,3,0,i,j,k) || 
				_equal(2,2,1,i,j,k) || _equal(2,3,1,i,j,k) ||
				_equal(2,2,2,i,j,k) || _equal(2,3,2,i,j,k) )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 4 :
		// ........        ........        ........        ........
		// ........        ........        ........        ........        
		// ..X.....        ..X.....        ........        ........        
		// ..X.....        ..X.....        ........        ........        
		// ........        ........        ........        ........        
		// ........        ........        ........        ........        
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(2,2,0,i,j,k) || _equal(2,3,0,i,j,k) ||
				_equal(2,2,1,i,j,k) || _equal(2,3,1,i,j,k) )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 5:
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
		if(k < (NoximGlobalParams::mesh_dim_z - 1)){
				if( ((i == 1)&&(j == 1)) || ((i == 1)&&(j == 2)) || ((i == 2)&&(j == 2)) || ((i == 2)&&(j == 1))
				|| ((i == 6)&&(j == 5)) || ((i == 5)&&(j == 6)) || ((i == 6)&&(j == 6)) || ((i == 5)&&(j == 5)) ){
					throttling[i][j][k] = 1;
					t[i][j][k]->pe->IntoEmergency();
					t[i][j][k]->r ->IntoEmergency();
				}
				else{
					throttling[i][j][k] = 0;
					t[i][j][k]->pe->OutOfEmergency();
					t[i][j][k]->r ->OutOfEmergency();
				}				
			}
			else {
				throttling[i][j][k] = 0;
				beltway[i][j][k]    = false;
				t[i][j][k]->pe->OutOfEmergency();
				t[i][j][k]->r ->OutOfEmergency();
			}
		}
	break;
	case 6 :
		// ........        ........        ........        ........
		// ....x...        ........        ........        ........
		// ....x...        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ..xx....        ..xx....        ........        ........
		// ..xx....        ..xx....        ........        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(2,5,0,i,j,k) || _equal(2,6,0,i,j,k) ||
				_equal(3,5,0,i,j,k) || _equal(3,6,0,i,j,k) || 
				_equal(4,1,0,i,j,k) || _equal(4,2,0,i,j,k) || 
				_equal(2,5,1,i,j,k) || _equal(2,6,1,i,j,k) ||  
				_equal(3,5,1,i,j,k) || _equal(3,6,1,i,j,k)   )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 7 :
		// ........        ........        ........        ........
		// ....x...        ....x...        ........        ........
		// ....x...        ....x...        ........        ........
		// ........        ........        ........        ........
		// .xxxx...        ..x.....        ........        ........
		// ..xx....        ..xx....        ..x.....        ........
		// ..xx....        ..xx....        ..x.....        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(1,4,0,i,j,k) ||  
				_equal(2,4,0,i,j,k) || _equal(2,5,0,i,j,k) || _equal(2,6,0,i,j,k) || 
				_equal(3,4,0,i,j,k) || _equal(3,5,0,i,j,k) || _equal(3,6,0,i,j,k) || 
				_equal(4,1,0,i,j,k) || _equal(4,2,0,i,j,k) || _equal(4,4,0,i,j,k) ||
				_equal(2,4,1,i,j,k) || _equal(2,5,1,i,j,k) || _equal(2,6,1,i,j,k) || 
				_equal(3,5,1,i,j,k) || _equal(3,6,1,i,j,k) || 
				_equal(4,1,1,i,j,k) || _equal(4,2,1,i,j,k) || 
				_equal(2,5,2,i,j,k) || _equal(2,6,2,i,j,k) )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 8 :
		// ........        ........        ........        ........
		// ....x...        ........        ........        ........
		// ....x...        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ..xx....        ..xx....        ........        ........
		// ..xx....        ..xx....        ........        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(2,5,0,i,j,k) || _equal(2,6,0,i,j,k) || _equal(3,5,0,i,j,k) || 
				_equal(3,6,0,i,j,k) || _equal(4,1,0,i,j,k) || _equal(4,2,0,i,j,k) || 
				_equal(2,5,1,i,j,k) || _equal(2,6,1,i,j,k) || _equal(3,5,1,i,j,k) || 
				_equal(3,6,1,i,j,k) )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 9 :
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ....x...        ........        ........        ........
		// ........        ........        ........        ........
		// ..xx....        ........        ........        ........
		// ..xx....        ..xx....        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(2,4,0,i,j,k) || _equal(2,5,0,i,j,k) || _equal(3,4,0,i,j,k) || 
				_equal(3,5,0,i,j,k) || _equal(4,2,0,i,j,k) || _equal(2,5,1,i,j,k) || 
				_equal(3,5,1,i,j,k) )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 10:
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ..xx....        ........        ........        ........
		// ..xx....        ..xx....        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		// ........        ........        ........        ........
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if( _equal(2,4,0,i,j,k) || _equal(2,5,0,i,j,k) || _equal(3,4,0,i,j,k) || 
				_equal(3,5,0,i,j,k) || _equal(2,5,1,i,j,k) || _equal(3,5,1,i,j,k) )
				_setThrot(i,j,k);
			else
				_setNormal(i,j,k);
		}
	break;
	case 11:
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
		if(k < (NoximGlobalParams::mesh_dim_z - 2)){
				if( ((i == 1)&&(j == 1)) || ((i == 1)&&(j == 2)) || ((i == 2)&&(j == 2)) || ((i == 2)&&(j == 1))
				|| ((i == 6)&&(j == 5)) || ((i == 5)&&(j == 6)) || ((i == 6)&&(j == 6)) || ((i == 5)&&(j == 5)) ){
					throttling[i][j][k] = 1;
					t[i][j][k]->pe->IntoEmergency();
					t[i][j][k]->r ->IntoEmergency();
				}
				else{
					throttling[i][j][k] = 0;
					t[i][j][k]->pe->OutOfEmergency();
					t[i][j][k]->r ->OutOfEmergency();
				}				
			}
			else {
				throttling[i][j][k] = 0;
				beltway[i][j][k]    = false;
				t[i][j][k]->pe->OutOfEmergency();
				t[i][j][k]->r ->OutOfEmergency();
			}
		}
	break;
	case 12:
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if(k < (NoximGlobalParams::mesh_dim_z - 2)){
				if(((i == 3))&&((j == 3))){
					throttling[i][j][k] = 1;
					t[i][j][k]->pe->IntoEmergency();
					t[i][j][k]->r ->IntoEmergency();
				}
				else{
					throttling[i][j][k] = 0;
					t[i][j][k]->pe->OutOfEmergency();
					t[i][j][k]->r ->OutOfEmergency();
				}				
			}
			else {
				throttling[i][j][k] = 0;
				beltway[i][j][k]    = false;
				t[i][j][k]->pe->OutOfEmergency();
				t[i][j][k]->r ->OutOfEmergency();
			}
		}
	break;
	case 13:
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
		if(k < (NoximGlobalParams::mesh_dim_z - 1)){
				if( ((i == 1)&&(j == 5)) || ((i == 1)&&(j == 6)) || ((i == 2)&&(j == 5)) || ((i == 2)&&(j == 6))
				|| ((i == 5)&&(j == 1)) || ((i == 6)&&(j == 1)) || ((i == 5)&&(j == 2)) || ((i == 6)&&(j == 2)) ){
					throttling[i][j][k] = 1;
					t[i][j][k]->pe->IntoEmergency();
					t[i][j][k]->r ->IntoEmergency();
				}
				else{
					throttling[i][j][k] = 0;
					t[i][j][k]->pe->OutOfEmergency();
					t[i][j][k]->r ->OutOfEmergency();
				}				
			}
			else {
				throttling[i][j][k] = 0;
				beltway[i][j][k]    = false;
				t[i][j][k]->pe->OutOfEmergency();
				t[i][j][k]->r ->OutOfEmergency();
			}
		}
	break;
	case 14:
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
			if(k < (NoximGlobalParams::mesh_dim_z - 1)){
				if(((i == 3))&&((j == 3))){
					throttling[i][j][k] = 1;
					t[i][j][k]->pe->IntoEmergency();
					t[i][j][k]->r ->IntoEmergency();
				}
				else{
					throttling[i][j][k] = 0;
					t[i][j][k]->pe->OutOfEmergency();
					t[i][j][k]->r ->OutOfEmergency();
				}				
			}
			else {
				throttling[i][j][k] = 0;
				beltway[i][j][k]    = false;
				t[i][j][k]->pe->OutOfEmergency();
				t[i][j][k]->r ->OutOfEmergency();
			}
		}
	break;
	case 15:
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
		if(k < (NoximGlobalParams::mesh_dim_z - 1)){
				if( ((i == 4)&&(j == 4)) || ((i == 4)&&(j == 3)) || ((i == 3)&&(j == 4)) || ((i == 3)&&(j == 3)) ){
				//if( ((i == 1)&&(j == 1)) || ((i == 6)&&(j == 6)) ){
				//if ( false ){
					throttling[i][j][k] = 1;
					t[i][j][k]->pe->IntoEmergency();
					t[i][j][k]->r ->IntoEmergency();
				}
				else{
					throttling[i][j][k] = 0;
					t[i][j][k]->pe->OutOfEmergency();
					t[i][j][k]->r ->OutOfEmergency();
				}				
			}
			else {
				throttling[i][j][k] = 0;
				beltway[i][j][k]    = false;
				t[i][j][k]->pe->OutOfEmergency();
				t[i][j][k]->r ->OutOfEmergency();
			}
		}
	break;
	case 16:
		for (int k=0; k<NoximGlobalParams::mesh_dim_z; k++)
		for (int j=0; j<NoximGlobalParams::mesh_dim_y; j++)
		for (int i=0; i<NoximGlobalParams::mesh_dim_x; i++){
		if(k < (NoximGlobalParams::mesh_dim_z - 1)){
				if( ((i == 1)&&(j == 1)) || ((i == 6)&&(j == 6)) ){
					throttling[i][j][k] = 1;
					t[i][j][k]->pe->IntoEmergency();
					t[i][j][k]->r ->IntoEmergency();
				}
				else{
					throttling[i][j][k] = 0;
					t[i][j][k]->pe->OutOfEmergency();
					t[i][j][k]->r ->OutOfEmergency();
				}				
			}
			else {
				throttling[i][j][k] = 0;
				beltway[i][j][k]    = false;
				t[i][j][k]->pe->OutOfEmergency();
				t[i][j][k]->r ->OutOfEmergency();
			}
		}
	break;
		
	} 
}
