/*
* Noxim - the NoC Simulator
*
* (C) 2005-2010 by the University of Catania
* For the complete list of authors refer to file ../doc/AUTHORS.txt
* For the license applied to these sources refer to file ../doc/LICENSE.txt
*
* This file contains the declaration of the tile
*/

#ifndef __NOXIMTILE_H__
#define __NOXIMTILE_H__

#include <systemc.h>
#include "NoximRouter.h"
#include "NoximProcessingElement.h"
using namespace std;

SC_MODULE(NoximTile)
{

	// I/O Ports
	sc_in_clk clock;		                // The input clock for the tile
	sc_in <bool> reset;	                    // The reset signal for the tile

	//////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////   added by Amin Norollah
    sc_in  <NoximFlit>  flit_rx[DIRECTIONS];	// The input channels
    sc_in  <bool>       req_rx [DIRECTIONS]	[DEFAULT_NUM_VC	];	        // The requests associated with the input channels
    sc_out <bool>       ack_rx [DIRECTIONS]	[DEFAULT_NUM_VC	];	        // The outgoing ack signals associated with the input channels

    sc_out <NoximFlit>  flit_tx[DIRECTIONS];	// The output channels
    sc_out <bool>       req_tx [DIRECTIONS]	[DEFAULT_NUM_VC	];	        // The requests associated with the output channels
    sc_in  <bool>       ack_tx [DIRECTIONS]	[DEFAULT_NUM_VC	];	        // The outgoing ack signals associated with the output channels
	//////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////

	sc_out <int>     free_slots[DIRECTIONS];
	sc_in  <int>     free_slots_neighbor[DIRECTIONS];
	/*******THROTTLING******/
	sc_out<bool>	  on_off[DIRECTIONS];
	sc_in<bool>		  on_off_neighbor[DIRECTIONS];

	/*******TB*******/
	sc_out<float>	  TB[DIRECTIONS];
	sc_in<float>	  TB_neighbor[DIRECTIONS];

	/*******PDT*******/
	sc_out<float>     PDT[DIRECTIONS];
	sc_in<float>      PDT_neighbor[DIRECTIONS];

	/*******buf info*******/
	sc_out<float>     buf[DIRECTIONS + 2][DIRECTIONS];
	sc_in<float>      buf_neighbor[DIRECTIONS + 2][DIRECTIONS];
	/*
	sc_out<float>     buf1[DIRECTIONS];
	sc_in<float>      buf1_neighbor[DIRECTIONS];

	sc_out<float>     buf2[DIRECTIONS];
	sc_in<float>      buf2_neighbor[DIRECTIONS];

	sc_out<float>     buf3[DIRECTIONS];
	sc_in<float>      buf3_neighbor[DIRECTIONS];

	sc_out<float>     buf4[DIRECTIONS];
	sc_in<float>      buf4_neighbor[DIRECTIONS];

	sc_out<float>     buf5[DIRECTIONS];
	sc_in<float>      buf5_neighbor[DIRECTIONS];

	sc_out<float>     buf6[DIRECTIONS];
	sc_in<float>      buf6_neighbor[DIRECTIONS];

	sc_out<float>     buf7[DIRECTIONS];
	sc_in<float>      buf7_neighbor[DIRECTIONS];
	*/

	/*******RCA******/
	// Derek----
	sc_in<int>			RCA_data_in[8];
	sc_out<int>			RCA_data_out[8];
	sc_out<double>		monitor_out[DIRECTIONS];
	sc_in<double>		monitor_in[DIRECTIONS];
	/*******RCA******/
	// NoP related I/O
	sc_out < NoximNoP_data > NoP_data_out[DIRECTIONS];
	sc_in < NoximNoP_data > NoP_data_in[DIRECTIONS];
	//
	sc_out < NoximNoP_data > vertical_free_slot_out;
	sc_in  < NoximNoP_data > vertical_free_slot_in[DEFAULT_MESH_DIM_Z];
	// Signals
	sc_signal <NoximFlit> flit_rx_local;	    // The input channels	
	sc_signal <bool>       req_rx_local[DEFAULT_NUM_VC];        // The requests associated with the input channels
	sc_signal <bool>       ack_rx_local[DEFAULT_NUM_VC];	    // The outgoing ack signals associated with the input channels

	sc_signal <NoximFlit> flit_rx_semi_local;	// The input channels
	sc_signal <bool>       req_rx_semi_local[DEFAULT_NUM_VC];   // The requests associated with the input channels
	sc_signal <bool>       ack_rx_semi_local[DEFAULT_NUM_VC];	// The outgoing ack signals associated with the input channels

	sc_signal <NoximFlit> flit_tx_local;	    // The output channels
	sc_signal <bool>       req_tx_local[DEFAULT_NUM_VC];	    // The requests associated with the output channels
	sc_signal <bool>       ack_tx_local[DEFAULT_NUM_VC];	    // The outgoing ack signals associated with the output channels

	sc_signal <NoximFlit> flit_tx_semi_local;	// The output channels
	sc_signal <bool>       req_tx_semi_local[DEFAULT_NUM_VC];	// The requests associated with the output channels
	sc_signal <bool>       ack_tx_semi_local[DEFAULT_NUM_VC];	// The outgoing ack signals associated with the output channels

	sc_signal <int> free_slots_local;
	sc_signal <int> free_slots_neighbor_local;
	//sc_signal <int> free_slots_semi_local;
	//sc_signal <int> free_slots_neighbor_semi_local;
	//Beltway
	sc_signal <int> free_slots_neighbor_router[4];
	sc_signal <int> RCA_PE_router[8];
	sc_signal <NoximNoP_data> NoP_PE_router[4];
	// Instances
	NoximRouter *r;		                // Router instance
	NoximProcessingElement *pe;	                // Processing Element instance

												// Constructor
	int i;
	SC_CTOR(NoximTile) {

		// Router pin assignments
		r = new NoximRouter("Router");
		r->clock(clock);
		r->reset(reset);
		// cout<<"Start to build Tile"<<endl;
		for (i = 0; i < DIRECTIONS; i++) {
			for (int vc = 0; vc < DEFAULT_NUM_VC; vc++) {
					r->req_rx[i][vc](req_rx[i][vc]);
					r->ack_rx[i][vc](ack_rx[i][vc]);
					r->req_tx[i][vc](req_tx[i][vc]);
					r->ack_tx[i][vc](ack_tx[i][vc]);
				}
			r->flit_rx[i](flit_rx[i]);
			r->flit_tx[i](flit_tx[i]);
			
			r->free_slots[i](free_slots[i]);
			r->free_slots_neighbor[i](free_slots_neighbor[i]);

			// NoP 
			r->NoP_data_out[i](NoP_data_out[i]);
			r->NoP_data_in[i](NoP_data_in[i]);
			/*******RCA******/
			if (i < 4) {
				r->RCA_data_out[i * 2 + 0](RCA_data_out[i * 2 + 0]);
				r->RCA_data_out[i * 2 + 1](RCA_data_out[i * 2 + 1]);
				r->RCA_data_in[i * 2 + 0](RCA_data_in[i * 2 + 0]);
				r->RCA_data_in[i * 2 + 1](RCA_data_in[i * 2 + 1]);
			}
			r->monitor_out[i](monitor_out[i]);
			r->monitor_in[i](monitor_in[i]);

			r->TB[i](TB[i]);
			r->TB_neighbor[i](TB_neighbor[i]);

			r->buf[0][i](buf[0][i]);
			r->buf_neighbor[0][i](buf_neighbor[0][i]);

			r->buf[1][i](buf[1][i]);
			r->buf_neighbor[1][i](buf_neighbor[1][i]);

			r->buf[2][i](buf[2][i]);
			r->buf_neighbor[2][i](buf_neighbor[2][i]);

			r->buf[3][i](buf[3][i]);
			r->buf_neighbor[3][i](buf_neighbor[3][i]);

			r->buf[4][i](buf[4][i]);
			r->buf_neighbor[4][i](buf_neighbor[4][i]);

			r->buf[5][i](buf[5][i]);
			r->buf_neighbor[5][i](buf_neighbor[5][i]);

			r->buf[6][i](buf[6][i]);
			r->buf_neighbor[6][i](buf_neighbor[6][i]);

			r->buf[7][i](buf[7][i]);
			r->buf_neighbor[7][i](buf_neighbor[7][i]);

			r->PDT[i](PDT[i]);
			r->PDT_neighbor[i](PDT_neighbor[i]);

			r->on_off[i](on_off[i]);
			r->on_off_neighbor[i](on_off_neighbor[i]);
		}
		/*
		for( i=0; i<4; i++){
		r->on_off[i](on_off[i]);
		r->on_off_neighbor[i](on_off_neighbor[i]);
		}*/

		for (i = 0; i<NoximGlobalParams::mesh_dim_z; i++)
			r->vertical_free_slot_in[i](vertical_free_slot_in[i]);
		r->vertical_free_slot_out(vertical_free_slot_out);

		for (int vc = 0; vc < DEFAULT_NUM_VC; vc++) {
			r->req_rx[DIRECTION_LOCAL][vc]	(req_tx_local[vc]);
			r->ack_rx[DIRECTION_LOCAL][vc]	(ack_tx_local[vc]);

			r->req_tx[DIRECTION_LOCAL][vc]	(req_rx_local[vc]);
			r->ack_tx[DIRECTION_LOCAL][vc]	(ack_rx_local[vc]);

			r->req_rx[DIRECTION_SEMI_LOCAL][vc](req_tx_semi_local[vc]);
			r->ack_rx[DIRECTION_SEMI_LOCAL][vc](ack_tx_semi_local[vc]);

			r->req_tx[DIRECTION_SEMI_LOCAL][vc](req_rx_semi_local[vc]);
			r->ack_tx[DIRECTION_SEMI_LOCAL][vc](ack_rx_semi_local[vc]);
		}
			
		r->flit_rx[DIRECTION_LOCAL]			(flit_tx_local);
		r->flit_tx[DIRECTION_LOCAL]			(flit_rx_local);
		r->flit_rx[DIRECTION_SEMI_LOCAL]	(flit_tx_semi_local);
		r->flit_tx[DIRECTION_SEMI_LOCAL]	(flit_rx_semi_local);

		r->free_slots[DIRECTION_LOCAL]			(free_slots_local);
		r->free_slots_neighbor[DIRECTION_LOCAL] (free_slots_neighbor_local);

		//r->free_slots         [DIRECTION_SEMI_LOCAL](free_slots_semi_local);
		//r->free_slots_neighbor[DIRECTION_SEMI_LOCAL](free_slots_neighbor_semi_local);
		// Processing Element pin assignments
		pe = new NoximProcessingElement("ProcessingElement");
		pe->clock(clock);
		pe->reset(reset);

		for (int vc = 0; vc < DEFAULT_NUM_VC; vc++) {
			pe->req_rx[vc](req_rx_local[vc]);
			pe->ack_rx[vc](ack_rx_local[vc]);

			pe->req_tx[vc](req_tx_local[vc]);
			pe->ack_tx[vc](ack_tx_local[vc]);

			pe->req_semi_rx[vc](req_rx_semi_local[vc]);
			pe->ack_semi_rx[vc](ack_rx_semi_local[vc]);

			pe->req_semi_tx[vc](req_tx_semi_local[vc]);
			pe->ack_semi_tx[vc](ack_tx_semi_local[vc]);
		}
		pe->flit_rx(flit_rx_local);
		pe->flit_semi_rx(flit_rx_semi_local);
		pe->flit_tx(flit_tx_local);
		pe->flit_semi_tx(flit_tx_semi_local);

		pe->free_slots(free_slots_neighbor_local);
		pe->free_slots_neighbor(free_slots_local);

		//Beltway 
		for (i = 0; i < 4; i++) {
			r->free_slots_PE[i](free_slots_neighbor_router[i]);
			pe->free_slots_router[i](free_slots_neighbor_router[i]);
		}
		for (i = 0; i < 8; i++) {
			r->RCA_PE[i](RCA_PE_router[i]);
			pe->RCA_router[i](RCA_PE_router[i]);
		}
		for (i = 0; i < 4; i++) {
			r->NoP_PE[i](NoP_PE_router[i]);
			pe->NoP_router[i](NoP_PE_router[i]);
		}
	}

};

#endif
