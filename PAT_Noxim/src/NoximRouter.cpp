/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the router
 *
 *
 * Edited by Amin Norollah and Danesh Derafshi (2017 May 25)
 *
 */
#include <math.h>
#include "NoximMain.h"
#include "NoximRouter.h"
#include "NoximStats.h"

/////////////////////////////
//////    rxProcess - Receive Process //////
// This process handles incoming flits from all input ports and virtual channels
// It validates incoming requests, checks buffer availability, and stores flits
//...........................
void NoximRouter::rxProcess() {
	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
		cout << "Router[" << local_id << "]-Rx" << endl;
	if (reset.read()) { // RESET PHASE
		// Reset phase: Initialize all buffers, counters, and state variables
		for (int i = 0; i < DIRECTIONS + 2; i++)
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
				routed_flits[i][vc] = 0;         // Reset flit counter for each input/VC
				buffer[i][vc].Clean();           // Clear all buffers
				id_recieve[i][vc] = -1;          // Reset test ID tracking
			}
		reservation_table.clear();                // Clear switch reservation table
		local_drained = 0;                       // Reset local drain counter
		routed_DWflits = 0;                      // Reset Data Width (DW) flit counter
		routed_packets = 0;                      // Reset packet counter
	}
	else {
		// NORMAL OPERATION: Process incoming flits from all input ports and VCs
		// This process implements the receive side of the handshake protocol
		// All arbitration and routing decisions happen in txProcess()
		for (int i = 0; i < DIRECTIONS + 2; i++)
			// Flit acceptance conditions:
			// 1) Neighbor is requesting to send (req_rx == 1)
			// 2) Router is not in emergency/throttled mode
			// 3) Input buffer has free space for this VC
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
			if (req_rx[i][vc].read() == 1) {
				if (!_emergency && !buffer[i][vc].IsFull()) {  // Check throttling and buffer space
					NoximFlit received_flit = flit_rx[i].read();  // Read the incoming flit
					if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
						cout << getCurrentCycleNum() << ": Router[" << local_id << "], Input[" << i
							<< "], Received flit: " << received_flit << endl;
					}
					// Record when flit entered this router (for delay calculation)
					received_flit.waiting_cnt = getCurrentCycleNum();
					// Store the incoming flit in the circular buffer for this input port and VC
					buffer[i][vc].Push(received_flit);
					routed_flits[i][vc]++;  // Increment flit counter for statistics

					// Track Data Width (DW) flits separately (flits using adaptive/downward routing)
					if (received_flit.routing_f != ROUTING_WEST_FIRST)
						routed_DWflits++;
				
					// Count packets (only HEAD flits represent new packets)
					if (received_flit.flit_type == FLIT_TYPE_HEAD)
						routed_packets++;
					//received_flit.vc = vc;
					//id_recieve[i][vc] = received_flit.test_id;
					//}
					// Update power consumption: lateral (horizontal) links consume power
					// Vertical links (UP/DOWN) have different power characteristics
					if (i != DIRECTION_UP && i != DIRECTION_DOWN)
						stats.power.RouterRxLateral();  // Account for lateral link receive power
				}
			}
		}
	}

	//if (!_emergency)//if router not throttle
	//	stats.power.TileLeakage();//Router + FPMAC + MEM
	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
		cout << "Router[" << local_id << "]-Rx ends" << endl;
}


/////////////////////////////
//////    txProcess - Transmit Process //////
// Main router pipeline logic: RC -> VA -> SA -> ST (Routing -> VC Allocation -> Switch Allocation -> Switch Traversal)
// This process handles all routing decisions, arbitration, and flit forwarding
//...........................
void NoximRouter::txProcess() {
	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
		cout << "Router[" << local_id << "]-Tx" << endl;
	if (reset.read()) { // RESET PHASE
		// Reset phase: Initialize all output signals, VC states, and pipeline registers
		for (int i = 0; i < DIRECTIONS + 2; i++) {// 0~6 DIRECTIONS + 2 = 8
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
				ack_rx[i][vc].write(0);                           // Clear acknowledge signals
				req_tx[i][vc].write(0);                           // Clear transmit request signals
				waiting[i][vc] = 0;                               // Reset waiting time counter
				vc_state.global_state(i, vc, G_IDLE);             // Set VC state to IDLE
				vc_state.routing(i, vc, NOT_VALID);               // Clear routing output
				vc_state.o(i, vc, NOT_VALID);                     // Clear output VC assignment
				baseline_wait[i][vc] = 0;                         // Reset baseline pipeline wait counter
				pre_routing[i][vc] = -1;                          // Clear pre-routing computation
				speculation_sa_enale[i][vc] = true;               // Enable speculative switch allocation
				// Initialize credit counters based on credit-based flow control mode
				if(NoximGlobalParams::arch_with_credit == 1)
					vc_state.credit(i, vc, NoximGlobalParams::buffer_depth, SET_MODE);  // Full credit = buffer depth
				else if (NoximGlobalParams::arch_with_credit == 0)
					vc_state.credit(i, vc, 1, SET_MODE);                                // Single credit mode

			}
			change_vc_baseline[i] = true;
			choose_vc_in_baseline[i] = NOT_RESERVED;
			win_port[i] = NOT_RESERVED;
			win_vc[i] = NOT_RESERVED;
			per_step1[i] = NOT_RESERVED;
			can_step_2_speculation[i] = false;
		}
		reservation_table.clear();
		_total_waiting = 0;
		for (int i = 0; i < 200; i++) {
			wait_cnt[i] = 0;
		}
		start_from_port_baseline = 0;
		start_from_port = 0;
		start_from_port_SA = 0;
		start_from_vc = 0;
		stats.resetChist();
	}
	else {

		//////////////////////////////
		//////    Check Global VC State - State Machine Transitions
		// VC states: IDLE -> ROUTING -> VCALLOCATING -> ACTIVE -> (back to IDLE on TAIL)
		//............................
		for (int input = 0; input < DIRECTIONS + 2; input++)
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
				// Transition: VCALLOCATING -> ACTIVE (VC allocated, ready to transmit)
				if (vc_state.getOut(input, vc, G_VCS) == G_VCALLOCATING && vc_state.getOut(input, vc, O_VCS) != NOT_VALID) 
					vc_state.global_state(input, vc, G_ACTIVE);
				// Transition: ROUTING -> VCALLOCATING (routing done, proceed to VC allocation)
				if (vc_state.getOut(input, vc, G_VCS) == G_ROUTING) 
					vc_state.global_state(input, vc, G_VCALLOCATING);
			}
		//////////////////////////////
		//////    Credit Management and Baseline Router Mode
		//............................
		// Baseline router mode: simpler VC selection without full pipeline support
		if (NoximGlobalParams::arch_router == 0) { // Baseline router mode
			for (int i = 0; i < DIRECTIONS + 2; i++) {
				if (change_vc_baseline[i]) {
					// Round-robin VC selection: find next non-empty VC for this input
					int count = 0, vc = (start_from_port_baseline) % NoximGlobalParams::num_vcs;
					while (buffer[i][vc].IsEmpty() && count < NoximGlobalParams::num_vcs) {
						start_from_port_baseline++;
						vc = (start_from_port_baseline) % NoximGlobalParams::num_vcs;
						count++;
					}
					if (!buffer[i][vc].IsEmpty()) {
						choose_vc_in_baseline[i] = vc;  // Select this VC
						change_vc_baseline[i] = false;  // VC selected, don't change until needed
					}
					else choose_vc_in_baseline[i] = NOT_RESERVED;  // No VC available
				}
				// Decrement baseline pipeline wait counter
				if (baseline_wait[i][choose_vc_in_baseline[i]] > 0)
					delay_baseline(i, choose_vc_in_baseline[i], MINUS_MODE);
			}
			start_from_port_baseline++;  // Advance round-robin pointer
		}
		// Credit-based flow control: update credits when downstream router acknowledges receipt
		for (int o = 0; o < DIRECTIONS + 2; o++) {
			for (int vc_out = 0; vc_out < NoximGlobalParams::num_vcs; vc_out++) {
				req_tx[o][vc_out].write(0);      // Clear transmit requests (will be set if flit transmitted)
				ack_rx[o][vc_out].write(0);      // Clear receive acknowledgments
				// Credit update: when downstream router acknowledges, increment available credits
				if (ack_tx[o][vc_out].read() == 1)
					vc_state.credit(o, vc_out, NOT_VALID, PLUS_MODE);  // Increase credit count
			}
			can_step_2_speculation[o] = false;  // Reset speculative SA flag
		}
		//////////////////////////////
		//////     Switch Traversal (ST) Stage - Flit Transmission
		//............................
		// This stage transmits flits that won in previous SA stage through the crossbar
		for (int output = 0; output < DIRECTIONS + 2; output++) {
			int input = win_port[output];	// Get winning input port from Switch Allocation
			int vc_in = win_vc  [output];	// Get winning VC from Switch Allocation
			// Check if this output port has a valid winner from SA stage
			if (input != NOT_RESERVED && vc_in != NOT_RESERVED) {
				// Validate flit can be transmitted: buffer not empty, not in emergency, VC in ACTIVE state
				if (!buffer[input][vc_in].IsEmpty() && !_emergency && vc_state.getOut(input, vc_in, G_VCS) == G_ACTIVE) {
					NoximFlit flit = buffer[input][vc_in].Front();  // Get flit from front of buffer
					int vc_out = vc_state.getOut(input, vc_in, O_VCS);  // Get allocated output VC
					//verbose
					if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
						cout << getCurrentCycleNum() << ": Router[" << local_id << "], Input[" << input <<
							"] forward to Output[" << output << "], flit: " << flit << endl;
					}
					// Check if router speed throttling allows transmission (DFS: Data Forwarding Speed)
					if ((getCurrentCycleNum() % DFS) < (8 - RST)) {
						if (flit.flit_type == FLIT_TYPE_HEAD) { // Update routing hints for next router (look-ahead routing)
							flit.east = east[input][vc_in];        // Pre-computed east direction hint
							flit.south = south[input][vc_in];      // Pre-computed south direction hint
							flit.vc = vc_out;                      // Assign output VC to flit
							flit.pre_routing = pre_routing[input][vc_in];  // Pre-computed routing for next router
						}
						// Transmit flit through crossbar to output port
						flit_tx[output].write(flit);
						// Assert request signal to downstream router
						req_tx[output][vc_out].write(!buffer[input][vc_in].IsEmpty());
						buffer[input][vc_in].Pop();                // Remove flit from input buffer
						waiting[input][vc_in] = 0;                 // Reset waiting counter
						vc_state.credit(output, vc_out, NOT_VALID, MINUS_MODE);  // Decrement credit (flit sent)
						ack_rx[input][vc_in].write(!_emergency);  // Acknowledge upstream router (unless in emergency)

						// Baseline architecture: set pipeline wait cycles based on architecture (3/4/5 stage)
						if (flit.flit_type != FLIT_TYPE_TAIL && NoximGlobalParams::arch_router == 0) {
							baseline_wait[input][vc_in] = 4;		// Default: 5-stage pipeline (wait 4 cycles)
							if (NoximGlobalParams::arch_rc == 1 || NoximGlobalParams::arch_sa == 1)
								baseline_wait[input][vc_in]--;		// 4-stage: RC or SA pipelined (wait 3 cycles)
							if (NoximGlobalParams::arch_rc == 1 && NoximGlobalParams::arch_sa == 1)
								baseline_wait[input][vc_in]--;		// 3-stage: both RC and SA pipelined (wait 2 cycles)
							change_vc_baseline[input] = true;      // Need to reselect VC after wait period
						}

						// Update power consumption: lateral (horizontal) crossbar and link power
						if (output != DIRECTION_UP && output != DIRECTION_DOWN)
							stats.power.Router2Lateral(); // Account for crossbar traversal + lateral link transmission

						_total_waiting += getCurrentCycleNum() - flit.waiting_cnt;

						if (flit.flit_type == FLIT_TYPE_HEAD)
							if (((getCurrentCycleNum() - flit.waiting_cnt) < 200) && (getCurrentCycleNum() > 400000))
								wait_cnt[(getCurrentCycleNum() - flit.waiting_cnt)]++;

						// Update statistics and handle flit reception
						if (output == DIRECTION_LOCAL) {
							// Flit reached local processing element - update receive statistics
							stats.receivedFlit(getCurrentCycleNum(), flit);
							stats.power.Router2Local();  // Account for local port power
							// Volume-based simulation stopping: stop when specified number of flits drained
							if (NoximGlobalParams::max_volume_to_be_drained) {
								if (drained_volume >= NoximGlobalParams::max_volume_to_be_drained)
									sc_stop();  // Stop simulation when target volume reached
								else {
									drained_volume++;      // Increment global counter
									local_drained++;       // Increment local counter
								}
							}
						}
						// Tail flit processing: release reserved resources
						if (flit.flit_type == FLIT_TYPE_TAIL) {
							reservation_table.release(output, vc_out);  // Release switch reservation
							vc_state.global_state(input, vc_in, G_IDLE);  // Return VC to IDLE state
							vc_state.routing(input, vc_in, NOT_VALID);    // Clear routing output
							vc_state.o(input, vc_in, NOT_VALID);          // Clear output VC assignment
							speculation_sa_enale[input][vc_in] = true;    // Re-enable speculative SA for next packet
						}
					}
				}//output port reserve,
			}
			else {//wait for other input to use that output port
				  //count waiting time
				waiting[input][vc_in]++;
			}
		}
		// Switch Allocating initialization
		// --Permission & Candidator per VCs for outputs
		temp_SA_stage = 0;
		wait_SA_stage = false;
		vc_SA_stage = NOT_RESERVED;
		o_SA_stage = NOT_RESERVED;
		for (int i = 0; i < DIRECTIONS + 2; i++) {
			win_port[i] = NOT_RESERVED;
			win_vc[i] = NOT_RESERVED;
			per_step1[i] = NOT_RESERVED;
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
				can_step1[i][vc] = false;
			for (int j = 0; j < DIRECTIONS + 2; j++)
				can_step2[i][j] = false;
		}
		//////////////////////////////
		//////    Routing Computation (RC) Stage
		//............................
		// RC stage: compute output port direction for HEAD flits
		// Each input port has one RC unit, so only one VC per input can use RC per cycle
		for (int i = 0; i < DIRECTIONS + 2; i++)
			RC_ACTION[i] = true;	// Mark RC unit as available for each input port
		// Round-robin scheduling across input ports
		for (int j = 0; j < DIRECTIONS + 2; j++) {
			int i = (start_from_port + j) % (DIRECTIONS + 2);  // Round-robin port selection
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
				// RC conditions: VC in IDLE state, and either baseline VC selected or pipelined router
				if (vc_state.getOut(i, vc, G_VCS) == G_IDLE && 
				    (choose_vc_in_baseline[i] == vc && baseline_wait[i][vc]<=0 || NoximGlobalParams::arch_router == 1)) {
					if (!buffer[i][vc].IsEmpty() && RC_ACTION[i] == true) {
						NoximFlit flit = buffer[i][vc].Front();  // Get HEAD flit from buffer
						/////////////// buffer power
						stats.power.input_buffer_read();  // Account for buffer read power
						/////////////// end power
						//verbose
						// Debugging: BODY flits should not need routing (should follow HEAD flit path)
						if (flit.flit_type == FLIT_TYPE_BODY && NoximGlobalParams::verbose_mode > VERBOSE_LOW) { 
							cout << "** MISS ROUTE **" << endl;
							cout << "		* Router[" << local_id << "], Input[" << i << "], VC[" << vc << "]" << endl;
							cout << "		* Flit: "  << flit << endl << endl;
						}
						if (flit.flit_type == FLIT_TYPE_HEAD) {
							// Perform routing computation (unless RC is pipelined and routing already done)
							if (!NoximGlobalParams::arch_rc || i >= DIRECTION_LOCAL) {
								vc_state.global_state(i, vc, G_ROUTING);  // Set VC state to ROUTING
								//start_from_port++;
								// Prepare routing data structure with all necessary information
								NoximRouteData route_data;
								route_data.current_id = local_id;                    // Current router ID
								// Handle multi-path routing: if arrived at intermediate node, route to final destination
								route_data.src_id = (flit.arr_mid) ? flit.mid_id : flit.src_id;
								route_data.dst_id = (flit.arr_mid) ? flit.dst_id : flit.mid_id;
								route_data.dir_in = i;                              // Input direction
								route_data.routing = flit.routing_f;                // Routing algorithm to use
								route_data.DW_layer = flit.DW_layer;                // Data width layer for 3D routing
								route_data.arr_mid = flit.arr_mid;                  // Flag: has packet reached intermediate node
								if (flit.routing_f < 0) {
									cout << getCurrentCycleNum() << ":" << flit << endl;
									cout << "flit.current_id" << "=" << local_id << id2Coord(route_data.current_id) << endl;
									cout << "flit.src_id    " << "=" << flit.src_id << id2Coord(flit.src_id) << endl;
									cout << "flit.mid_id    " << "=" << flit.mid_id << id2Coord(flit.mid_id) << endl;
									cout << "flit.dst_id    " << "=" << flit.dst_id << id2Coord(flit.dst_id) << endl;
									cout << "flit.dir_in    " << "=" << i << endl;
									cout << "flit.routing   " << "=" << flit.routing_f << endl;
									cout << "flit.DW_layer  " << "=" << flit.DW_layer << endl;
									cout << "flit.arr_mid   " << "=" << flit.arr_mid << endl;
									assert(false);
								}
								if (NoximGlobalParams::verbose_mode > VERBOSE_LOW)
									cout << "Before route:" << flit;
								// Perform routing computation: returns output port direction
								int output_routing = route(route_data, &flit.south, &flit.east, &flit.vc);
								vc_state.routing(i, vc, output_routing);  // Store routing result in VC state
								RC_ACTION[i] = false;                     // RC unit busy - only one VC per input per cycle
								vc_state.id(i, vc, flit.test_id);         // Store test ID for debugging
							}
							else {
								// Pipelined RC: routing already computed, skip to VC allocation
								vc_state.global_state(i, vc, G_VCALLOCATING);
								vc_state.routing(i, vc, flit.pre_routing);  // Use pre-computed routing result
								if (NoximGlobalParams::verbose_mode > VERBOSE_MEDIUM)
									cout << " Pre_route:" << flit;
							}
							/********** need power **********/
						}
					}
				}
			}
		}
		////////////////////////////////////////////////
		//////    Virtual Channel Allocation (VA) Stage
		//..............................................
		// VA stage: allocate output VC for each HEAD flit that completed routing
		// Uses round-robin arbitration to ensure fairness
		for (int j = 0; j < DIRECTIONS + 2; j++) {
			int i = (start_from_port + j) % (DIRECTIONS + 2); // RoundRobin Arbiter across input ports
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
				// VA conditions: VC in VCALLOCATING state, and eligible for VA
				if (vc_state.getOut(i, vc, G_VCS) == G_VCALLOCATING && 
				    (choose_vc_in_baseline[i] == vc && baseline_wait[i][vc] <= 0 || NoximGlobalParams::arch_router == 1)) {
					if (!buffer[i][vc].IsEmpty()) {
						NoximFlit flit = buffer[i][vc].Front();
						/////////////// buffer power
						stats.power.input_buffer_read();  // Account for buffer read power
						/////////////// end power
						if (flit.flit_type == FLIT_TYPE_HEAD) {
							int output_port = vc_state.getOut(i, vc, R_VCS);	// Get routing output port from RC stage
							/////////////////////////////
							//////// Pre Routing - Look-ahead Routing Computation
							// Compute routing for the next router while current router does VA/SA
							// This enables pipelined routing and reduces latency
							if (NoximGlobalParams::arch_rc && output_port < DIRECTION_LOCAL) {
								int next_router = NOT_VALID;  // ID of next router in path
								int dir_in;                   // Input direction at next router
								NoximRouteData route_data;    // Routing data for next router
								switch (output_port) {
								case 0: next_router = local_id - NoximGlobalParams::mesh_dim_x; dir_in = 2; break;
								case 1: next_router = local_id + 1; dir_in = 3; break;
								case 2: next_router = local_id + NoximGlobalParams::mesh_dim_x; dir_in = 0; break;
								case 3: next_router = local_id - 1; dir_in = 1; break;
								case 4: next_router = local_id - (NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y); dir_in = 5; break;
								case 5: next_router = local_id + (NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y); dir_in = 4; break;
								default: assert(false);
								}
								route_data.current_id = next_router;
								route_data.src_id = (flit.arr_mid) ? flit.mid_id : flit.src_id;
								route_data.dst_id = (flit.arr_mid) ? flit.dst_id : flit.mid_id;
								route_data.dir_in = dir_in;
								route_data.routing = flit.routing_f;
								route_data.DW_layer = flit.DW_layer;
								route_data.arr_mid = flit.arr_mid;
								if (flit.routing_f < 0) {
									cout << getCurrentCycleNum() << ":" << flit << endl;
									cout << "flit.current_id" << "=" << local_id << id2Coord(route_data.current_id) << endl;
									cout << "flit.src_id    " << "=" << flit.src_id << id2Coord(flit.src_id) << endl;
									cout << "flit.mid_id    " << "=" << flit.mid_id << id2Coord(flit.mid_id) << endl;
									cout << "flit.dst_id    " << "=" << flit.dst_id << id2Coord(flit.dst_id) << endl;
									cout << "flit.dir_in    " << "=" << i << endl;
									cout << "flit.routing   " << "=" << flit.routing_f << endl;
									cout << "flit.DW_layer  " << "=" << flit.DW_layer << endl;
									cout << "flit.arr_mid   " << "=" << flit.arr_mid << endl;
									assert(false);
								}
								pre_routing[i][vc] = route(route_data, &flit.south, &flit.east, &flit.vc);
								/********** need power **********/
							}
							//////// End Pre Routing	
							/////////////////////////////
							// Check if output port and VC are available for reservation
							if (reservation_table.isAvailable(output_port, flit.vc) && (getCurrentCycleNum() % DFS) < (8 - RST))
							{
								// Reserve output port and VC in reservation table (prevents conflicts)
								reservation_table.reserve(i, vc, output_port, flit.vc); 
								south[i][vc] = flit.south;  // Store routing hints for later use
								east[i][vc] = flit.east;
								vc_state.o(i, vc, flit.vc);  // Store allocated output VC in VC state
								if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
									cout << getCurrentCycleNum() << ": Router[" << local_id << "], Input[" << i << "], VC[" << vc << "] (" << buffer[i][vc].Size() << " flits)" << ", reserved Output[" << output_port << "], flit: " << flit << endl;
								}
								//stats.power.ArbiterNControl();
								stats.power.vcAllocation();
							}
							 else {
								 baseline_wait[i][vc]++;
								 change_vc_baseline[i] = true;
							 }
							 /////////////////////////////
							 //////// Speculation SA
							if (NoximGlobalParams::arch_sa)
								if(win_port[output_port] == NOT_RESERVED && speculation_sa_enale[i][vc]) {
									speculation_sa_enale[i][vc] = false;
									can_step_2_speculation[i] = true;	//in SA Stage
									win_port[output_port] = i;			//step 1 for speculation

									int vc_out = vc_state.getOut(i, vc, O_VCS);
									if (vc_out!= NOT_RESERVED) {
										if (vc_state.getOut(output_port, vc_out, C_VCS) > 0)
											win_vc	[output_port] = vc;	//step 2 for speculation							
									}
								}
							//////// End Speculation SA
							/////////////////////////////
						}
					}
				}
		}
		start_from_port++;
		////////////////////////////////////////////////
		//////    Switch Allocating
		//..............................................
		// tow stage SA Arbitration
		////////////////////////////////////////////////
		////		SA  can & per   -- step 1		////
		////////////////////////////////////////////////
		for (int input = 0; input < DIRECTIONS + 2; input++)
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
				o_SA_stage  = vc_state.getOut(input, vc, R_VCS);
				vc_SA_stage = vc_state.getOut(input, vc, O_VCS);
				if (choose_vc_in_baseline[input] == vc  && baseline_wait[input][vc] <= 0 || NoximGlobalParams::arch_router == 1)
					if (!_emergency && vc_state.getOut(input, vc, G_VCS) == G_ACTIVE && vc_state.getOut(o_SA_stage, vc_SA_stage, C_VCS) > 0)
						can_step1[input][vc] = true;
					else {
						delay_baseline(input, vc, SET_MODE);
						change_vc_baseline[input] = true;
					}
			}
		for (int i = 0; i < DIRECTIONS + 2; i++) {
			if (!can_step_2_speculation[i]) {
				wait_SA_stage = false;
				for (int vc_1 = 0; vc_1 < NoximGlobalParams::num_vcs; vc_1++) {
					int vc_in = (start_from_port_SA + vc_1) % NoximGlobalParams::num_vcs;
					if (can_step1[i][vc_in] && !wait_SA_stage) {
						per_step1[i] = vc_in;  // vc for step 2
						temp_SA_stage = vc_state.getOut(i, vc_in, R_VCS);  //SA step 2
						can_step2[temp_SA_stage][i] = true;
						wait_SA_stage = true;
					}
					else if (can_step1[i][vc_in] && wait_SA_stage)
						waiting[i][vc_in]++;
				}
			}
		}
		////////////////////////////////////////////////
		////		SA  can & per   -- step 2		////
		////////////////////////////////////////////////
		for (int o = 0; o < DIRECTIONS + 2; o++) {
			wait_SA_stage = false;
			if (win_port[o] == NOT_RESERVED)
				for (int i = 0; i < DIRECTIONS + 2; i++) {
					int input = (start_from_port_SA + i) % (DIRECTIONS + 2);
					if (can_step2[o][input] && !wait_SA_stage) {
						win_port[o] = input;			//input
						win_vc[o] = per_step1[input];   //per_step1[input] is vc input
						wait_SA_stage = true;
						stats.power.switchAllocation(); // SA power
					}
				}
		}
		start_from_port_SA++;
	}
	stats.power.clock();
} /////////////    END TxProcess
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

// delay_baseline - Manages pipeline wait cycles for baseline router architectures
// Baseline routers have different pipeline depths (3, 4, or 5 stages)
// This function sets or decrements the wait counter accordingly
void NoximRouter::delay_baseline(int input, int vc_in, int mode) {
	if (SET_MODE) {
		// Initialize wait counter based on pipeline depth
		baseline_wait[input][vc_in] = 4;		// Default: 5-stage pipeline (wait 4 cycles)
		if (NoximGlobalParams::arch_rc == 1 || NoximGlobalParams::arch_sa == 1)
			baseline_wait[input][vc_in]--;		// 4-stage: RC or SA pipelined (wait 3 cycles)
		if (NoximGlobalParams::arch_rc == 1 && NoximGlobalParams::arch_sa == 1)
			baseline_wait[input][vc_in]--;		// 3-stage: both RC and SA pipelined (wait 2 cycles)
	}
	else if (MINUS_MODE)
		baseline_wait[input][vc_in]--;  // Decrement wait counter (called each cycle)
}


// Detour - Alternative routing function when primary path is congested or throttled
// Uses fully adaptive routing but filters out throttled directions
int NoximRouter::Detour(const NoximRouteData & route_data, int input, int waiting)
{//	assert(false);

	// Handle special cases: destination reached
	if (route_data.dst_id == local_id && route_data.arr_mid)
		return DIRECTION_LOCAL;  // Final destination reached, deliver to local PE
	else if ((route_data.dst_id == local_id && !route_data.arr_mid))
		return DIRECTION_SEMI_LOCAL;  // Intermediate destination reached, continue routing

	// Get coordinates for routing computation
	NoximCoord position = id2Coord(route_data.current_id);
	NoximCoord src_coord = id2Coord(route_data.src_id);
	NoximCoord dst_coord = id2Coord(route_data.dst_id);

	// Use fully adaptive routing to get all candidate directions
	vector < int > candidate_channels = routingFullyAdaptive(position, dst_coord);
	//cout<<candidate_channels.size()<<endl;
	// Filter out throttled lateral directions (cannot route through throttled neighbors)
	for (int i = candidate_channels.size() - 1; i >= 0; i--) {
		if (candidate_channels[i] < 4) // Only check lateral directions (not UP/DOWN)
			if (on_off_neighbor[candidate_channels[i]].read() == 1) {  // If direction is throttled
				candidate_channels.erase(candidate_channels.begin() + i);  // Remove throttled direction
			}
	}
	//cout<<candidate_channels.size()<<endl;
	// Select randomly from remaining candidate directions
	return selectionRandom(candidate_channels);
}


// getCurrentNoPData - Builds Neighbor-on-Path (NoP) data structure
// NoP data contains congestion information about neighboring routers
// Used by adaptive routing algorithms to make informed routing decisions
NoximNoP_data NoximRouter::getCurrentNoPData() const
{
	NoximNoP_data NoP_data;
	bool x = false;
	// Collect buffer status and availability for each neighboring direction
	for (int j = 0; j < DIRECTIONS; j++) {
		// Get free slots from neighbor (received via free_slots_neighbor signal)
		NoP_data.channel_status_neighbor[j].free_slots = free_slots_neighbor[j].read();

		// Check if any VC is available on this output port
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
			if (reservation_table.isAvailable(j, vc) != 0) {
				x = true;			// Port has available VCs
				break;
			}

		NoP_data.channel_status_neighbor[j].available = x;  // Set availability flag
		x = false;  // Reset for next direction
	}

	NoP_data.sender_id = local_id;  // Identify this router as sender

	return NoP_data;
}

// bufferMonitor - Monitors buffer status and sends information to neighbors
// This function runs every cycle to update free slot information for adaptive routing
void NoximRouter::bufferMonitor()
{
	int i;
	if (reset.read()) {
		// Reset phase: Initialize free slots to maximum buffer size
		for (i = 0; i < DIRECTIONS + 1; i++)
			free_slots[i].write(buffer[i][0].GetMaxBufferSize());  // All buffers empty initially
		for (int i = 0; i<4; i++)
			free_slots_PE[i].write(free_slots_neighbor[i]);  // Initialize PE signals

		NoximCoord position = id2Coord(local_id);  // Get router position for buffer budget calculation

		if ((position.x == 0 && position.y == 0 && position.z == 0) || (position.x == 7 && position.y == 7 && position.z == 7) ||
			(position.x == 7 && position.y == 0 && position.z == 0) ||
			(position.x == 0 && position.y == 7 && position.z == 0) ||
			(position.x == 0 && position.y == 0 && position.z == 7) ||
			(position.x == 7 && position.y == 7 && position.z == 0) ||
			(position.x == 7 && position.y == 0 && position.z == 7) ||
			(position.x == 0 && position.y == 7 && position.z == 7)) {
			buf_budget = 12;
		}
		else if ((position.x > 0 && position.x < 7 && (position.y == 0 && position.z == 0)) ||
			(position.y > 0 && position.y < 7 && (position.x == 0 && position.z == 0)) ||
			(position.z > 0 && position.z < 7 && (position.x == 0 && position.y == 0)) ||
			(position.x > 0 && position.x < 7 && (position.y == 0 && position.z == 7)) ||
			(position.y > 0 && position.y < 7 && (position.x == 0 && position.z == 7)) ||
			(position.z > 0 && position.z < 7 && (position.x == 0 && position.y == 7)) ||
			(position.x > 0 && position.x < 7 && (position.y == 7 && position.z == 0)) ||
			(position.y > 0 && position.y < 7 && (position.x == 7 && position.z == 0)) ||
			(position.z > 0 && position.z < 7 && (position.x == 7 && position.y == 0)) ||
			(position.x > 0 && position.x < 7 && (position.y == 7 && position.z == 7)) ||
			(position.y > 0 && position.y < 7 && (position.x == 7 && position.z == 7)) ||
			(position.z > 0 && position.z < 7 && (position.x == 7 && position.y == 7))) {
			buf_budget = 8;
		}
		else if ((position.x > 0 && position.x < 7 && position.y > 0 && position.y < 7 && position.z == 0)) {
			buf_budget = 4;
		}
		else
			buf_budget = 0;
	}
	else {
		// Normal operation: Update buffer status and send to neighbors
		// Update current input buffer free slots (sent to neighbors for adaptive routing)
		for (i = 0; i < DIRECTIONS + 1; i++)
			free_slots[i].write(buffer[i][0].getCurrentFreeSlots());  // Send current free slots
		// NoP (Neighbor-on-Path) selection: send congestion info to each neighbor direction
		NoximNoP_data current_NoP_data = getCurrentNoPData();  // Build NoP data structure

		// Broadcast NoP data to all lateral neighbors
		for (i = 0; i < DIRECTIONS; i++)
			NoP_data_out[i].write(current_NoP_data);
		// Broadcast NoP data vertically (for 3D NoC layer-to-layer communication)
		vertical_free_slot_out.write(current_NoP_data);

		for (int i = 0; i<4; i++)
			free_slots_PE[i].write(free_slots_neighbor[i]);
		for (int i = 0; i<8; i++)
			RCA_PE[i].write(RCA_data_in[i]);
		for (int i = 0; i<4; i++)
			NoP_PE[i].write(NoP_data_in[i]);
	}
}

// routingFunction - Main routing function dispatcher
// Selects and calls the appropriate routing algorithm based on global configuration
// Returns a vector of candidate output port directions (can be 1 or more for adaptive routing)
vector < int >NoximRouter::routingFunction(const NoximRouteData & route_data, int *south, int *east, int *vc)
{
	// Extract coordinates from route data for routing computation
	NoximCoord position = id2Coord(route_data.current_id);  // Current router position
	NoximCoord src_coord = id2Coord(route_data.src_id);     // Source router position
	NoximCoord dst_coord = id2Coord(route_data.dst_id);     // Destination router position
	//bool downn=route_data.down;
	int dir_in = route_data.dir_in;      // Input direction
	int routing = route_data.routing;    // Routing algorithm ID
	int DW_layer = route_data.DW_layer;  // Data width layer for 3D routing
	int arr_mid = route_data.arr_mid;    // Flag: reached intermediate node
	//int south         = route_data.south ;
	//int east          = route_data.east ;

	// Dispatch to appropriate routing algorithm based on global configuration
	switch (NoximGlobalParams::routing_algorithm) {
		/***ACCESS IC LAB's Routing Algorithm***/
		//taheri
	case ROUTING_XYZ:
		return routingXYZ(position, dst_coord);
	case ROUTING_DTBR:
		return routing_DTBR(position, dst_coord, south, east, vc);
	case ROUTING_off_on_xy2:
		return routing_off_on_xy2(position, dst_coord, south, east, vc);
	case ROUTING_ZXY:
		return routingZXY(position, dst_coord);

	case ROUTING_DOWNWARD:
		return routingDownward(position, src_coord, dst_coord);

	case ROUTING_WEST_FIRST_DOWNWARD:
		if (route_data.routing == ROUTING_WEST_FIRST)
			return routingWestFirst(position, dst_coord);
		else
			return routingWF_Downward(position, src_coord, dst_coord);

	case ROUTING_ODD_EVEN_DOWNWARD:
		return routingOddEven_Downward(position, src_coord, dst_coord, route_data);

	case ROUTING_ODD_EVEN_3D:
		return routingOddEven_3D(position, src_coord, dst_coord);

	case ROUTING_ODD_EVEN_Z:
		return routingOddEven_Z(position, src_coord, dst_coord);

	case ROUTING_WEST_FIRST:
		return routingWestFirst(position, dst_coord);

	case ROUTING_NORTH_LAST:
		return routingNorthLast(position, dst_coord);

	case ROUTING_NEGATIVE_FIRST:
		return routingNegativeFirst(position, dst_coord);

	case ROUTING_ODD_EVEN:
		return routingOddEven(position, src_coord, dst_coord);

	case ROUTING_DYAD:
		return routingDyAD(position, src_coord, dst_coord);

	case ROUTING_FULLY_ADAPTIVE:
		return routingFullyAdaptive(position, dst_coord);

	case ROUTING_TABLE_BASED:
		return routingTableBased(position, dir_in, dst_coord);

	case ROUTING_DLADR:
		return routingDLADR(position, src_coord, dst_coord, routing, DW_layer);

	case ROUTING_DLAR:
		return routingDLAR(position, src_coord, dst_coord, routing, DW_layer);

	case ROUTING_DLDR:
		return routingDLDR(position, src_coord, dst_coord, routing, DW_layer);
	default:
		assert(false);
	}

	// something weird happened, you shouldn't be here
	return (vector < int >) (0);
}

/* FAULT-TOLERANT ROUTING IMPLEMENTATION SUGGESTION:
 * To add fault-tolerant routing in Noxim, follow this approach: (1) Add fault status signals similar to thermal throttling:
 * declare sc_out<bool> fault_status[DIRECTIONS] and sc_in<bool> fault_neighbor[DIRECTIONS] in NoximRouter.h to propagate
 * fault information (router/link failures) between neighbors, (2) Implement fault detection mechanism: add a fault detection
 * process that monitors link health (timeouts, CRC errors, handshake failures) and sets fault_status signals accordingly,
 * (3) Modify the route() function: after thermal filtering (line 760-767), add fault filtering that removes candidate channels
 * where fault_neighbor[direction].read() == true, similar to how throttled directions are filtered, (4) Use adaptive routing
 * algorithms: prefer ROUTING_FULLY_ADAPTIVE or ROUTING_ODD_EVEN which provide multiple alternative paths, allowing packets to
 * detour around faults, (5) Implement fallback mechanism: if all candidate directions are faulty, use a backup routing algorithm
 * (e.g., switch from fully adaptive to XY routing with fault avoidance) or implement a fault-aware routing table that pre-computes
 * alternative paths, (6) Add fault recovery: periodically re-check fault status to allow routing through previously faulty links
 * if they recover, and (7) Consider deadlock prevention: ensure fault-tolerant routing maintains deadlock freedom by using turn
 * models or virtual channel partitioning even when avoiding faults. The key is leveraging the existing thermal-aware filtering
 * infrastructure and extending it to handle permanent/temporary faults while maintaining connectivity through alternative paths.
 */

// route - Main routing function that calls routing algorithm and applies thermal-aware filtering
// Filters out throttled directions from candidate channels for thermal-aware routing
int NoximRouter::route(const NoximRouteData & route_data, int *south, int *east, int *vc)
{
	// Check if destination reached
	if (route_data.dst_id == route_data.current_id && route_data.arr_mid)
		return DIRECTION_LOCAL;  // Final destination, deliver to local PE
	else if ((route_data.dst_id == route_data.current_id && !route_data.arr_mid))
		return DIRECTION_SEMI_LOCAL;  // Intermediate destination, continue routing
	
	// Get candidate directions from routing algorithm
	vector < int >candidate_channels = routingFunction(route_data, south, east, vc);
	
	// Thermal-aware filtering: remove throttled directions from candidates
	for (int i = candidate_channels.size() - 1; i >= 0; i--) {
		if (candidate_channels[i] < 4) // Only check lateral directions (0-3: N,E,S,W)
			if (on_off_neighbor[candidate_channels[i]].read() == 1) {  // If neighbor is throttled
																	 //				if(candidate_channels.size() == 1)
																	 //				break;
				candidate_channels.erase(candidate_channels.begin() + i);  // Remove throttled direction
			}
	}

	NoximCoord position = id2Coord(local_id);  // Get current router position

	// Fallback: if all lateral directions are throttled, try vertical direction
	if (candidate_channels.size() == 0) {
		/*cout<<"no any candidate channels"<<endl;
		if(!on_off_neighbor[DIRECTION_NORTH] && position.y > 0)//if the direction is not throttled
		candidate_channels.push_back(DIRECTION_NORTH);
		if(!on_off_neighbor[DIRECTION_SOUTH] && position.y <  NoximGlobalParams::mesh_dim_y - 1)//if the direction is not throttled
		candidate_channels.push_back(DIRECTION_SOUTH);
		if(!on_off_neighbor[DIRECTION_EAST] && position.x <  NoximGlobalParams::mesh_dim_x - 1)//if the direction is not throttled
		candidate_channels.push_back(DIRECTION_EAST);
		if(!on_off_neighbor[DIRECTION_WEST] && position.x > 0)//if the direction is not throttled
		candidate_channels.push_back(DIRECTION_WEST);*/
		// Escape route: if all lateral directions throttled, try going down (toward heat sink)
		if (!on_off_neighbor[DIRECTION_DOWN] && position.z < NoximGlobalParams::mesh_dim_z-1)
			candidate_channels.push_back(DIRECTION_DOWN);  // Downward direction not throttled
	}
	// For beltway packet, push more directions into candidate_channels to achieve DBA (Dynamic Beltway Adaptation)
	// Apply selection function to choose best direction from candidates (random, buffer level, NoP, etc.)
	return selectionFunction(candidate_channels, route_data);
}

void NoximRouter::NoP_report() const
{
	NoximNoP_data NoP_tmp;
	cout << getCurrentCycleNum() << ": Router[" << local_id << "] NoP report: " << endl;

	for (int i = 0; i < DIRECTIONS; i++) {
		NoP_tmp = NoP_data_in[i].read();
		if (NoP_tmp.sender_id != NOT_VALID)
			cout << NoP_tmp;
	}
}

void NoximRouter::RCA_Aggregation()
{
	int i, j;
	if (reset.read()) {
		for (i = 0; i < DIRECTIONS; i++)
			monitor_out[i].write(0);

		buffer_util = 0;

	}
	else {
		int RCA_data_tmp;
		if (!_emergency) {
			if (NoximGlobalParams::routing_algorithm == ROUTING_ODD_EVEN_Z) {            //Restriction of OE routing is considered

				NoximCoord position = id2Coord(local_id);

				//N0
				RCA_data_tmp = (buffer[0][0].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[5].read() + RCA_data_in[6].read()) / 4;
				//cout << "\tN0: " << RCA_data_tmp;
				RCA_data_out[0].write(RCA_data_tmp);
				//N1
				RCA_data_tmp = (buffer[0][0].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[4].read() + RCA_data_in[3].read()) / 4;
				//cout << "\tN1: " << RCA_data_tmp;
				RCA_data_out[1].write(RCA_data_tmp);

				//E0
				if (position.x % 2 == 0)
					RCA_data_tmp = (buffer[1][1].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[7].read() + RCA_data_in[0].read()) / 4;
				else
					RCA_data_tmp = (buffer[1][1].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[7].read() + 0) / 4;
				//cout << "\tE0: " << RCA_data_tmp;
				RCA_data_out[2].write(RCA_data_tmp);
				//E1
				if (position.x % 2 == 0)
					RCA_data_tmp = (buffer[1][1].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[6].read() + RCA_data_in[5].read()) / 4;
				else
					RCA_data_tmp = (buffer[1][1].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[6].read() + 0) / 4;
				//cout << "\tE1: " << RCA_data_tmp;
				RCA_data_out[3].write(RCA_data_tmp);

				//S0
				RCA_data_tmp = (buffer[2][2].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[1].read() + RCA_data_in[2].read()) / 4;
				//cout << "\tS0: " << RCA_data_tmp;
				RCA_data_out[4].write(RCA_data_tmp);
				//S1
				RCA_data_tmp = (buffer[2][2].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[0].read() + RCA_data_in[7].read()) / 4;
				//cout << "\tS1: " << RCA_data_tmp;
				RCA_data_out[5].write(RCA_data_tmp);

				//W0
				if (position.x % 2 == 1)
					RCA_data_tmp = (buffer[3][3].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[3].read() + RCA_data_in[4].read()) / 4;
				else
					RCA_data_tmp = (buffer[3][3].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[3].read() + 0) / 4;
				//cout << "\tW0: " << RCA_data_tmp;
				RCA_data_out[6].write(RCA_data_tmp);
				//W1
				if (position.x % 2 == 1)
					RCA_data_tmp = (buffer[3][3].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[2].read() + RCA_data_in[1].read()) / 4;
				else
					RCA_data_tmp = (buffer[3][3].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[2].read() + 0) / 4;
				//cout << "\tW1: " << RCA_data_tmp;
				RCA_data_out[7].write(RCA_data_tmp);
			}
			else {                              //Restriction of OE routing is NOT considered
												//N0
				RCA_data_tmp = (buffer[0][0].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[5].read() + RCA_data_in[6].read()) / 4;
				//cout << "\tN0: " << RCA_data_tmp;
				RCA_data_out[0].write(RCA_data_tmp);
				//N1
				RCA_data_tmp = (buffer[0][0].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[4].read() + RCA_data_in[3].read()) / 4;
				//cout << "\tN1: " << RCA_data_tmp;
				RCA_data_out[1].write(RCA_data_tmp);
				//E0
				RCA_data_tmp = (buffer[1][1].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[7].read() + RCA_data_in[0].read()) / 4;
				//cout << "\tE0: " << RCA_data_tmp;
				RCA_data_out[2].write(RCA_data_tmp);
				//E1
				RCA_data_tmp = (buffer[1][1].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[6].read() + RCA_data_in[5].read()) / 4;
				//cout << "\tE1: " << RCA_data_tmp;
				RCA_data_out[3].write(RCA_data_tmp);
				//S0
				RCA_data_tmp = (buffer[2][2].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[1].read() + RCA_data_in[2].read()) / 4;
				//cout << "\tS0: " << RCA_data_tmp;
				RCA_data_out[4].write(RCA_data_tmp);
				//S1
				RCA_data_tmp = (buffer[2][2].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[0].read() + RCA_data_in[7].read()) / 4;
				//cout << "\tS1: " << RCA_data_tmp;
				RCA_data_out[5].write(RCA_data_tmp);
				//W0
				RCA_data_tmp = (buffer[3][3].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[3].read() + RCA_data_in[4].read()) / 4;
				//cout << "\tW0: " << RCA_data_tmp;
				RCA_data_out[6].write(RCA_data_tmp);
				//W1
				RCA_data_tmp = (buffer[3][3].getCurrentFreeSlots() * 2 * 32 + RCA_data_in[2].read() + RCA_data_in[1].read()) / 4;
				//cout << "\tW1: " << RCA_data_tmp;
				RCA_data_out[7].write(RCA_data_tmp);
			}
		}
		else {//in emergency mode
			for (int i = 0; i < 8; i++)
				RCA_data_out[i].write(0);
		}
		NoximCoord position = id2Coord(local_id);
		double throt_in[DIRECTIONS];
		for (int j = 0; j < DIRECTIONS; j++)
			throt_in[j] = monitor_in[j].read();

		int self_throt;
		if (_emergency)
			self_throt = 1;
		else
			self_throt = 0;

		double throt_out[DIRECTIONS];
		for (int j = 0; j < DIRECTIONS; j++) {
			if ((throt_in[j] != 0) || (self_throt != 0)) //OR gate for throttling awareness
				throt_out[j] = 1;
			else
				throt_out[j] = 0;
		}

		monitor_out[DIRECTION_NORTH].write(throt_out[DIRECTION_SOUTH]);
		monitor_out[DIRECTION_SOUTH].write(throt_out[DIRECTION_NORTH]);
		monitor_out[DIRECTION_EAST].write(throt_out[DIRECTION_WEST]);
		monitor_out[DIRECTION_WEST].write(throt_out[DIRECTION_EAST]);

		//cout<<getCurrentCycleNum()<<endl;
		double total_size = 0;
		buffer_used = 0;
		for (int j = 0; j < 4; j++) {
			buffer_used += buffer[j][0].GetMaxBufferSize() - buffer[j][0].getCurrentFreeSlots();
			total_size += buffer[j][0].GetMaxBufferSize();
		}
		buffer_util += buffer_used / total_size;
	}
}
//---------------------------------------------------------------------------

int NoximRouter::NoPScore(const NoximNoP_data & nop_data,
	const vector < int >&nop_channels) const
{
	int score = 0;

	for (unsigned int i = 0; i < nop_channels.size(); i++) {
		int available, not_throttle;
		if (nop_data.channel_status_neighbor[nop_channels[i]].available)available = 1;
		else			available = 0;
		int free_slots = nop_data.channel_status_neighbor[nop_channels[i]].free_slots;
		if ((nop_channels[i] == DIRECTION_DOWN) || (nop_channels[i] == DIRECTION_UP)) {
			not_throttle = 1;
		}
		else {
			int b;
			b = (int)monitor_in[i].read();
			if (b == 1) not_throttle = 0;
			else         not_throttle = 1;
		}
		score += (int)not_throttle*available*free_slots; //traffic-&throttling-aware
	}

	return score;
}

int NoximRouter::selectionNoP(const vector < int >&directions, const NoximRouteData & route_data)
{
	vector < int >neighbors_on_path;
	vector < int >score;
	int direction_selected = NOT_VALID;
	int current_id = route_data.current_id;

	for (unsigned int i = 0; i < directions.size(); i++) {
		// get id of adjacent candidate
		int candidate_id = getNeighborId(current_id, directions[i]);

		// apply routing function to the adjacent candidate node
		NoximRouteData tmp_route_data;
		tmp_route_data.current_id = candidate_id;
		tmp_route_data.src_id = route_data.src_id;
		tmp_route_data.dst_id = route_data.dst_id;
		tmp_route_data.dir_in = reflexDirection(directions[i]);
		tmp_route_data.DW_layer = route_data.DW_layer;
		tmp_route_data.routing = route_data.routing;
		//taheri
		int vc = 0;		//nead to review
		vector < int > next_candidate_channels = routingFunction(tmp_route_data, &current_id, &current_id, &vc);

		// select useful data from Neighbor-on-Path input 
		NoximNoP_data nop_tmp = NoP_data_in[directions[i]].read();

		// store the score of node in the direction[i]
		score.push_back(NoPScore(nop_tmp, next_candidate_channels));
	}

	// check for direction with higher score
	int  max_direction = directions[0];
	int  max = score[0];
	int  down_score = 0;

	for (unsigned int i = 0; i < directions.size(); i++) {
		if (score[i] > max) {
			max_direction = directions[i];
			max = score[i];
		}
	}
	// if multiple direction have the same score = max, choose randomly.

	vector < int >equivalent_directions;

	for (unsigned int i = 0; i < directions.size(); i++)
		if (score[i] == max)
			equivalent_directions.push_back(directions[i]);

	direction_selected =
		equivalent_directions[rand() % equivalent_directions.size()];

	return direction_selected;
}


int NoximRouter::selectionBufferLevel(const vector < int >&directions) //only vc0  --Need to review
{
	vector < int >best_dirs;
	double max_free_slots = 0;
	for (unsigned int i = 0; i < directions.size(); i++) {
		int free_slots = free_slots_neighbor[directions[i]].read();
		bool available = reservation_table.isAvailable(directions[i], 0);
		//double mttt = TB_neighbor[directions[i]].read();
		//double score = TB_neighbor[directions[i]].read() * free_slots;
		if (available) {
			if (free_slots > max_free_slots) {
				max_free_slots = free_slots;
				best_dirs.clear();
				best_dirs.push_back(directions[i]);
			}
			else if (free_slots == max_free_slots)
				best_dirs.push_back(directions[i]);
		}

	}
	if (best_dirs.size())
		return (best_dirs[rand() % best_dirs.size()]);
	else
		return (directions[rand() % directions.size()]);
}


int NoximRouter::selectionRCA2D(const vector<int>& directions, const NoximRouteData& route_data) {	//only vc0  --Need to review
	vector<int>  best_dirs;
	int best_RCA_value = 0;

	if (directions.size() == 1) {
		return directions[0];
	}

	for (unsigned int i = 0; i < directions.size(); ++i) {

		bool available = reservation_table.isAvailable(directions[i], 0);
		if (available) {
			int temp_RCA_value;
			if (directions[i] == DIRECTION_NORTH)
				if (id2Coord(route_data.dst_id).x > id2Coord(route_data.current_id).x)       //NE
					temp_RCA_value = RCA_data_in[DIRECTION_NORTH * 2 + 1].read();
				else if (id2Coord(route_data.dst_id).x < id2Coord(route_data.current_id).x)  //NW
					temp_RCA_value = RCA_data_in[DIRECTION_NORTH * 2 + 0].read();
				else                                                                        //N
					temp_RCA_value = (RCA_data_in[DIRECTION_NORTH * 2 + 1] + RCA_data_in[DIRECTION_NORTH * 2 + 0]) / 2;
			else if (directions[i] == DIRECTION_EAST)
				if (id2Coord(route_data.dst_id).y > id2Coord(route_data.current_id).y)       //SE
					temp_RCA_value = RCA_data_in[DIRECTION_EAST * 2 + 1].read();
				else if (id2Coord(route_data.dst_id).y < id2Coord(route_data.current_id).y)  //NE
					temp_RCA_value = RCA_data_in[DIRECTION_EAST * 2 + 0].read();
				else                                                                        //E
					temp_RCA_value = (RCA_data_in[DIRECTION_EAST * 2 + 1] + RCA_data_in[DIRECTION_EAST * 2 + 0]) / 2;
			else if (directions[i] == DIRECTION_SOUTH)
				if (id2Coord(route_data.dst_id).x > id2Coord(route_data.current_id).x)       //SE
					temp_RCA_value = RCA_data_in[DIRECTION_SOUTH * 2 + 0].read();
				else if (id2Coord(route_data.dst_id).x < id2Coord(route_data.current_id).x)  //SW
					temp_RCA_value = RCA_data_in[DIRECTION_SOUTH * 2 + 1].read();
				else                                                                        //S
					temp_RCA_value = (RCA_data_in[DIRECTION_SOUTH * 2 + 1] + RCA_data_in[DIRECTION_SOUTH * 2 + 0]) / 2;
			else if (directions[i] == DIRECTION_WEST)
				if (id2Coord(route_data.dst_id).y > id2Coord(route_data.current_id).y)       //SW
					temp_RCA_value = RCA_data_in[DIRECTION_WEST * 2 + 0].read();
				else if (id2Coord(route_data.dst_id).y < id2Coord(route_data.current_id).y)  //NW
					temp_RCA_value = RCA_data_in[DIRECTION_WEST * 2 + 1].read();
				else                                                                        //W
					temp_RCA_value = (RCA_data_in[DIRECTION_WEST * 2 + 1] + RCA_data_in[DIRECTION_WEST * 2 + 0]) / 2;
			else if (directions[i] == DIRECTION_UP)
				temp_RCA_value = free_slots_neighbor[DIRECTION_UP] * 32;
			else if (directions[i] == DIRECTION_DOWN)
				temp_RCA_value = free_slots_neighbor[DIRECTION_DOWN] * 32;
			else
				assert(false);

			if (temp_RCA_value > best_RCA_value) {
				best_RCA_value = temp_RCA_value;
				best_dirs.clear();
				best_dirs.push_back(directions[i]);
			}
			else if (temp_RCA_value == best_RCA_value) {
				best_dirs.push_back(directions[i]);
			}
		}
	}

	if (best_dirs.size()) {
		return best_dirs[rand() % best_dirs.size()];
	}
	else {
		return directions[rand() % directions.size()];
	}

	assert(false);
}

int NoximRouter::selectionProposed(const vector <int> & directions, const NoximRouteData & route_data)
{
	vector < int >neighbors_on_path;
	vector < int >score;
	int direction_selected = NOT_VALID;
	int current_id = route_data.current_id;

	for (unsigned int i = 0; i < directions.size(); i++) {
		// get id of adjacent candidate
		int candidate_id = getNeighborId(current_id, directions[i]);

		// apply routing function to the adjacent candidate node
		NoximRouteData tmp_route_data;
		tmp_route_data.current_id = candidate_id;
		tmp_route_data.src_id = route_data.src_id;
		tmp_route_data.dst_id = route_data.dst_id;
		tmp_route_data.dir_in = reflexDirection(directions[i]);
		tmp_route_data.DW_layer = route_data.DW_layer;
		tmp_route_data.routing = route_data.routing;
		int vc = 0;
		vector < int > next_candidate_channels = routingFunction(tmp_route_data, &current_id, &current_id, &vc);

		// select useful data from Neighbor-on-Path input 
		NoximNoP_data nop_tmp = NoP_data_in[directions[i]].read();

		// store the score of node in the direction[i]
		if (current_id < 128 && i < 4)
			score.push_back(NoPScore(nop_tmp, next_candidate_channels) * 2);
		else
			score.push_back(NoPScore(nop_tmp, next_candidate_channels));
	}

	// check for direction with higher score
	int  max_direction = directions[0];
	int  max = score[0];
	int  down_score = 0;

	for (unsigned int i = 0; i < directions.size(); i++) {
		if (score[i] > max) {
			max_direction = directions[i];
			max = score[i];
		}
	}
	// if multiple direction have the same score = max, choose randomly.

	vector < int >equivalent_directions;

	for (unsigned int i = 0; i < directions.size(); i++)
		if (score[i] == max)
			equivalent_directions.push_back(directions[i]);

	direction_selected =
		equivalent_directions[rand() % equivalent_directions.size()];

	return direction_selected;
}

int NoximRouter::selectionThermal(const vector<int>& directions, const NoximRouteData& route_data) { //revised by 20130803  //Thermal delay based

	NoximCoord current = id2Coord(route_data.current_id);
	NoximCoord     dst = id2Coord(route_data.dst_id);

	if (directions.size() == 1) {
		return directions[0];
	}
	else if (directions.size() == 2) {

		//throttling[packet_dst.x][packet_dst.y][packet_dst.z]

		float thermal_delay0 = 0;
		float thermal_delay1 = 0;
		float BCT0 = free_slots_neighbor[directions[0]].read();
		float BCT1 = free_slots_neighbor[directions[1]].read();

		thermal_delay0 = buf_neighbor[directions[0]][directions[0]].read() + BCT0;
		thermal_delay1 = buf_neighbor[directions[1]][directions[1]].read() + BCT1;
		/*
		if(thermal_delay1 < thermal_delay0)
		return directions[1];
		else
		return directions[0];
		*/
		// because of throttle , three path desrease to two path
		if (directions[0] == DIRECTION_NORTH && directions[1] == DIRECTION_EAST) {
			thermal_delay0 += 0.4*(buf_neighbor[DIRECTION_DOWN][directions[0]].read() + buf_neighbor[DIRECTION_EAST][directions[0]].read() + buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read()) + 0.2*buf_neighbor[DIRECTION_SOUTH][directions[0]].read() + 0.2*buf_neighbor[DIRECTION_WEST][directions[0]].read();
			thermal_delay1 += 0.4*(buf_neighbor[DIRECTION_DOWN][directions[1]].read() + buf_neighbor[DIRECTION_NORTH][directions[1]].read() + buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read()) + 0.2*buf_neighbor[DIRECTION_SOUTH][directions[1]].read() + 0.2*buf_neighbor[DIRECTION_WEST][directions[1]].read();
		}
		else if (directions[0] == DIRECTION_NORTH && directions[1] == DIRECTION_DOWN) {
			//buf_neighbor[directions[1]][directions[0]]
			thermal_delay0 += buf_neighbor[DIRECTION_EAST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_WEST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 5 + buf_neighbor[DIRECTION_EAST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_SOUTH][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 6 + buf_neighbor[DIRECTION_EAST][directions[0]].read() / 6 + buf_neighbor[DIRECTION_WEST][directions[0]].read() / 6 + buf_neighbor[DIRECTION_SOUTH][directions[0]].read() / 6 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read()) * 17 / 30;

			thermal_delay1 += (buf_neighbor[DIRECTION_EAST][directions[1]].read() + buf_neighbor[DIRECTION_NORTH][directions[1]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_WEST][directions[1]].read() + buf_neighbor[DIRECTION_SOUTH][directions[1]].read()) * 11 / 30 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read()) * 17 / 30;
		}
		else if (directions[0] == DIRECTION_SOUTH && directions[1] == DIRECTION_EAST) {
			thermal_delay0 += 0.4*(buf_neighbor[DIRECTION_DOWN][directions[0]].read() + buf_neighbor[DIRECTION_EAST][directions[0]].read() + buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read()) + 0.2*buf_neighbor[DIRECTION_NORTH][directions[0]].read() + 0.2*buf_neighbor[DIRECTION_WEST][directions[0]].read();
			thermal_delay1 += 0.4*(buf_neighbor[DIRECTION_DOWN][directions[1]].read() + buf_neighbor[DIRECTION_SOUTH][directions[1]].read() + buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read()) + 0.2*buf_neighbor[DIRECTION_NORTH][directions[1]].read() + 0.2*buf_neighbor[DIRECTION_WEST][directions[1]].read();
		}
		else if (directions[0] == DIRECTION_SOUTH && directions[1] == DIRECTION_DOWN) {
			thermal_delay0 += (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_EAST][directions[0]].read() + buf_neighbor[DIRECTION_DOWN][directions[0]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_NORTH][directions[0]].read() + buf_neighbor[DIRECTION_WEST][directions[0]].read()) * 11 / 30;

			thermal_delay1 += (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_EAST][directions[1]].read() + buf_neighbor[DIRECTION_SOUTH][directions[1]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_NORTH][directions[1]].read() + buf_neighbor[DIRECTION_WEST][directions[1]].read()) * 11 / 30;
		}
		else if (directions[0] == DIRECTION_EAST && directions[1] == DIRECTION_DOWN) {
			thermal_delay0 += (buf_neighbor[DIRECTION_EAST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_WEST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 5 + buf_neighbor[DIRECTION_EAST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_SOUTH][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 6 + buf_neighbor[DIRECTION_EAST][directions[0]].read() / 6 + buf_neighbor[DIRECTION_WEST][directions[0]].read() / 6 + buf_neighbor[DIRECTION_SOUTH][directions[0]].read() / 6 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_EAST][directions[0]].read() + buf_neighbor[DIRECTION_DOWN][directions[0]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_NORTH][directions[0]].read() + buf_neighbor[DIRECTION_WEST][directions[0]].read()) * 11 / 30) / 2;

			thermal_delay1 += ((buf_neighbor[DIRECTION_EAST][directions[1]].read() + buf_neighbor[DIRECTION_NORTH][directions[1]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_WEST][directions[1]].read() + buf_neighbor[DIRECTION_SOUTH][directions[1]].read()) * 11 / 30 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_EAST][directions[1]].read() + buf_neighbor[DIRECTION_SOUTH][directions[1]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_NORTH][directions[1]].read() + buf_neighbor[DIRECTION_WEST][directions[1]].read()) * 11 / 30) / 2;
		}
		//PTMAR

		if (thermal_delay1 < thermal_delay0)
			return directions[1];
		else
			return directions[0];


	}
	else if (directions.size() == 3) {

		float thermal_delay0 = 0;
		float thermal_delay1 = 0;
		float thermal_delay2 = 0;

		float BCT0 = free_slots_neighbor[directions[0]].read();
		float BCT1 = free_slots_neighbor[directions[1]].read();
		float BCT2 = free_slots_neighbor[directions[2]].read();

		thermal_delay0 = buf_neighbor[directions[0]][directions[0]].read() + BCT0;
		thermal_delay1 = buf_neighbor[directions[1]][directions[1]].read() + BCT1;
		thermal_delay2 = buf_neighbor[directions[2]][directions[2]].read() + BCT2;
		/*
		if(thermal_delay2 < thermal_delay1 && thermal_delay2 < thermal_delay0)
		return directions[2];
		else if(thermal_delay1 < thermal_delay0 && thermal_delay1 < thermal_delay2)
		return directions[1];
		else
		return directions[0];
		*/
		//WF3D offer zero or three path
		if (directions[0] == DIRECTION_NORTH && directions[1] == DIRECTION_EAST && directions[2] == DIRECTION_DOWN) {
			thermal_delay0 += buf_neighbor[DIRECTION_EAST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_WEST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 5 + buf_neighbor[DIRECTION_EAST][directions[0]].read() / 5 + buf_neighbor[DIRECTION_SOUTH][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 5 + buf_neighbor[DIRECTION_DOWN][directions[0]].read() / 6 + buf_neighbor[DIRECTION_EAST][directions[0]].read() / 6 + buf_neighbor[DIRECTION_WEST][directions[0]].read() / 6 + buf_neighbor[DIRECTION_SOUTH][directions[0]].read() / 6 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read()) * 17 / 30;
			thermal_delay1 += buf_neighbor[DIRECTION_DOWN][directions[1]].read() / 3 + buf_neighbor[DIRECTION_WEST][directions[1]].read() / 3 + buf_neighbor[DIRECTION_NORTH][directions[1]].read() / 3 + buf_neighbor[DIRECTION_DOWN][directions[1]].read() / 3 + buf_neighbor[DIRECTION_SOUTH][directions[1]].read() / 3 + buf_neighbor[DIRECTION_NORTH][directions[1]].read() / 3 + buf_neighbor[DIRECTION_DOWN][directions[1]].read() / 4 + buf_neighbor[DIRECTION_NORTH][directions[1]].read() / 4 + buf_neighbor[DIRECTION_SOUTH][directions[1]].read() / 4 + buf_neighbor[DIRECTION_WEST][directions[1]].read() / 4 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read()) * 17 / 30;
			thermal_delay2 += (buf_neighbor[DIRECTION_EAST][directions[2]].read() + buf_neighbor[DIRECTION_NORTH][directions[2]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_WEST][directions[2]].read() + buf_neighbor[DIRECTION_SOUTH][directions[2]].read()) * 11 / 30 + (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[2]].read() + buf_neighbor[DIRECTION_LOCAL][directions[2]].read()) * 17 / 30;
		}
		else if (directions[0] == DIRECTION_SOUTH && directions[1] == DIRECTION_EAST && directions[2] == DIRECTION_DOWN) {
			thermal_delay0 += (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_LOCAL][directions[0]].read() + buf_neighbor[DIRECTION_EAST][directions[0]].read() + buf_neighbor[DIRECTION_DOWN][directions[0]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_NORTH][directions[0]].read() + buf_neighbor[DIRECTION_WEST][directions[0]].read()) * 11 / 30;
			thermal_delay1 += (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_LOCAL][directions[1]].read() + buf_neighbor[DIRECTION_DOWN][directions[1]].read() + buf_neighbor[DIRECTION_SOUTH][directions[1]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_NORTH][directions[1]].read() + buf_neighbor[DIRECTION_WEST][directions[1]].read()) * 11 / 30;
			thermal_delay2 += (buf_neighbor[DIRECTION_SEMI_LOCAL][directions[2]].read() + buf_neighbor[DIRECTION_LOCAL][directions[2]].read() + buf_neighbor[DIRECTION_EAST][directions[2]].read() + buf_neighbor[DIRECTION_SOUTH][directions[2]].read()) * 17 / 30 + (buf_neighbor[DIRECTION_NORTH][directions[2]].read() + buf_neighbor[DIRECTION_WEST][directions[2]].read()) * 11 / 30;
		}
		//PTMAR

		if (thermal_delay2 < thermal_delay1 && thermal_delay2 < thermal_delay0)
			return directions[2];
		else if (thermal_delay1 < thermal_delay0 && thermal_delay1 < thermal_delay2)
			return directions[1];
		else
			return directions[0];

	}
	else
		return directions[rand() % directions.size()];



}

int NoximRouter::selectionRandom(const vector < int >&directions)
{
	return directions[rand() % directions.size()];
}
vector<int> NoximRouter::DW_layerSelFunction(const int select_routing, const NoximCoord& current, const NoximCoord& destination, const NoximCoord& source, int dw_layer) {
	switch (NoximGlobalParams::dw_layer_sel) {
	case DW_BL:return routingTLAR_DW(current, source, destination); break;
	case DW_ODWL:return routingTLAR_DW_ODWL(current, source, destination, dw_layer); break;
	case DW_ADWL:return routingTLAR_DW_ADWL(current, source, destination); break;
	case DW_IPD:return routingTLAR_DW_IPD(current, source, destination); break;
	case DW_VBDR:return routingTLAR_DW_VBDR(current, source, destination); break;
	case DW_ODWL_IPD:return routingTLAR_DW_ODWL_IPD(current, source, destination, dw_layer); break;
	default:return routingTLAR_DW(current, source, destination); break;
	}
}
int NoximRouter::selectionFunction(const vector < int >&directions,
	const NoximRouteData & route_data)
{
	// not so elegant but fast escape ;)
	if (directions.size() == 1)
		return directions[0];

	switch (NoximGlobalParams::selection_strategy) {
	case SEL_RANDOM:return selectionRandom(directions);
	case SEL_BUFFER_LEVEL:return selectionBufferLevel(directions);
	case SEL_NOP:return selectionNoP(directions, route_data);
	case SEL_RCA:return selectionRCA2D(directions, route_data);
	case SEL_PROPOSED:return selectionProposed(directions, route_data);
	case SEL_THERMAL:return selectionThermal(directions, route_data);
	default:assert(false);
	}
	return 0;
}

vector < int >NoximRouter::routingXYZ(const NoximCoord & current, const NoximCoord & destination) {
	vector < int >directions;
	if (destination.x > current.x)	    directions.push_back(DIRECTION_EAST) ;
	else if (destination.x < current.x)	directions.push_back(DIRECTION_WEST) ;
	else if (destination.y > current.y)	directions.push_back(DIRECTION_SOUTH);
	else if (destination.y < current.y)	directions.push_back(DIRECTION_NORTH);
	else if (destination.z > current.z)	directions.push_back(DIRECTION_DOWN) ;
	else if (destination.z < current.z)	directions.push_back(DIRECTION_UP)   ;
	if (directions.size() == 0)
		cout << "ff" << endl;
	return directions;
}
vector<int> NoximRouter::routingZXY(const NoximCoord& current, const NoximCoord& destination) {
	vector<int> directions;
	if (destination.z > current.z) directions.push_back(DIRECTION_DOWN)      ;
	else if (destination.z < current.z) directions.push_back(DIRECTION_UP)   ;
	else if (destination.x > current.x) directions.push_back(DIRECTION_EAST) ;
	else if (destination.x < current.x) directions.push_back(DIRECTION_WEST) ;
	else if (destination.y > current.y) directions.push_back(DIRECTION_SOUTH);
	else if (destination.y < current.y) directions.push_back(DIRECTION_NORTH);
	return directions;
}

vector < int >NoximRouter::routing_DTBR(const NoximCoord & current, const NoximCoord &destination, int *south, int *east, int *vc) {
	vector < int >directions;

	return directions;
}

vector<int> NoximRouter::routing_off_on_xy2(const NoximCoord & current, const NoximCoord &destination, int *south, int *east, int *vc) {
	vector<int> directions;

	return directions;
}

vector<int> NoximRouter::routingDownward(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination) {

	int down_level = NoximGlobalParams::down_level;
	vector<int> directions;
	int layer;	//means which layer that packet should be transmitted

	if ((source.z + down_level)> NoximGlobalParams::mesh_dim_z - 1)
		layer = NoximGlobalParams::mesh_dim_z - 1;
	else
		layer = source.z + down_level;

	if (current.z < layer && (current.x != destination.x || current.y != destination.y)) {
		if (current.z == destination.z && ((current.x - destination.x == 1 && current.y == destination.y)	//while the dest. is one hop to source, don't downward and transmit directly
			|| (current.x - destination.x == -1 && current.y == destination.y)
			|| (current.y - destination.y == 1 && current.x == destination.x)
			|| (current.y - destination.y == -1 && current.x == destination.x)))
		{
			directions = routingXYZ(current, destination);
		}
		else {
			directions.push_back(DIRECTION_DOWN);
		}
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y))	//forward by xyz routing
	{
		directions = routingXYZ(current, destination);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z)	//same (x,y), forward to up
	{
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z)  //same (x,y), forward to down
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else
	{
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		exit(1);
	}

	return directions;
}

vector<int> NoximRouter::routingWF_Downward(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination) {

	int down_level = NoximGlobalParams::down_level;
	vector<int> directions;
	int layer;	//means which layer that packet should be transmitted

	if ((source.z + down_level)> NoximGlobalParams::mesh_dim_z - 1)
		layer = NoximGlobalParams::mesh_dim_z - 1;
	else
		layer = source.z + down_level;

	if (current.z < layer && (current.x != destination.x || current.y != destination.y)) {
		if (current.z == destination.z && ((current.x - destination.x == 1 && current.y == destination.y)	//while the dest. is one hop to source, don't downward and transmit directly
			|| (current.x - destination.x == -1 && current.y == destination.y)
			|| (current.y - destination.y == 1 && current.x == destination.x)
			|| (current.y - destination.y == -1 && current.x == destination.x)))
		{
			directions = routingWestFirst(current, destination);
		}
		else {
			directions.push_back(DIRECTION_DOWN);
		}
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y))	//forward by xyz routing
	{
		directions = routingWestFirst(current, destination);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z)	//same (x,y), forward to up
	{
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z)  //same (x,y), forward to down
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else
	{
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		exit(1);
	}

	return directions;
}

vector < int >NoximRouter::routingWestFirst(const NoximCoord & current,
	const NoximCoord & destination)
{

	vector < int >directions;
	if (destination.x <= current.x || destination.z < current.z || (destination.y == current.y && destination.z == current.z))
		return routingXYZ(current, destination);

	if (destination.y < current.y && destination.z == current.z) {
		directions.push_back(DIRECTION_NORTH);
		directions.push_back(DIRECTION_EAST);
	}
	else if (destination.y > current.y && destination.z == current.z) {
		directions.push_back(DIRECTION_SOUTH);
		directions.push_back(DIRECTION_EAST);
	}
	else
		if (destination.y < current.y && destination.z > current.z) {
			directions.push_back(DIRECTION_NORTH);
			directions.push_back(DIRECTION_EAST);
			directions.push_back(DIRECTION_DOWN);
		}
		else if (destination.y > current.y && destination.z > current.z) {
			directions.push_back(DIRECTION_SOUTH);
			directions.push_back(DIRECTION_EAST);
			directions.push_back(DIRECTION_DOWN);
		}
		else {
			//directions.clear();
			return routingXYZ(current, destination);
		}
		//cout<<"dir"<<directions.size()<<endl;
		return directions;

		/*
		vector < int >directions;

		if (destination.x <= current.x ||  destination.y == current.y)
		return routingXYZ(current, destination);

		if (destination.y < current.y) {
		directions.push_back(DIRECTION_NORTH);
		directions.push_back(DIRECTION_EAST);
		} else {
		directions.push_back(DIRECTION_SOUTH);
		directions.push_back(DIRECTION_EAST);
		}

		return directions;*/
}

vector < int >NoximRouter::routingNorthLast(const NoximCoord & current,
	const NoximCoord & destination)
{
	vector < int >directions;

	if (destination.x == current.x || destination.y <= current.y)
		return routingXYZ(current, destination);

	if (destination.x < current.x) {
		directions.push_back(DIRECTION_SOUTH);
		directions.push_back(DIRECTION_WEST);
	}
	else {
		directions.push_back(DIRECTION_SOUTH);
		directions.push_back(DIRECTION_EAST);
	}

	return directions;
}

vector < int >NoximRouter::routingNegativeFirst(const NoximCoord & current,
	const NoximCoord &
	destination)
{
	vector < int >directions;

	if ((destination.x <= current.x && destination.y <= current.y) ||
		(destination.x >= current.x && destination.y >= current.y))
		return routingXYZ(current, destination);

	if (destination.x > current.x && destination.y < current.y) {
		directions.push_back(DIRECTION_NORTH);
		directions.push_back(DIRECTION_EAST);
	}
	else {
		directions.push_back(DIRECTION_SOUTH);
		directions.push_back(DIRECTION_WEST);
	}

	return directions;
}

vector < int >NoximRouter::routingOddEven(const NoximCoord & current,
	const NoximCoord & source,
	const NoximCoord & destination)
{
	vector < int >directions;

	int c0 = current.x;
	int c1 = current.y;
	int s0 = source.x;
	//  int s1 = source.y;
	int d0 = destination.x;
	int d1 = destination.y;
	int e0, e1;

	e0 = d0 - c0;
	e1 = -(d1 - c1);

	if (e0 == 0) {
		if (e1 > 0)
			directions.push_back(DIRECTION_NORTH);
		else
			directions.push_back(DIRECTION_SOUTH);
	}
	else {
		if (e0 > 0) {
			if (e1 == 0)
				directions.push_back(DIRECTION_EAST);
			else {
				if ((c0 % 2 == 1) || (c0 == s0)) {
					if (e1 > 0)
						directions.push_back(DIRECTION_NORTH);
					else
						directions.push_back(DIRECTION_SOUTH);
				}
				if ((d0 % 2 == 1) || (e0 != 1))
					directions.push_back(DIRECTION_EAST);
			}
		}
		else {
			directions.push_back(DIRECTION_WEST);
			if (c0 % 2 == 0) {
				if (e1 > 0)
					directions.push_back(DIRECTION_NORTH);
				if (e1 < 0)
					directions.push_back(DIRECTION_SOUTH);
			}
		}
	}

	if (!(directions.size() > 0 && directions.size() <= 2)) {
		cout << "\n PICCININI, CECCONI & ... :";	// STAMPACCHIA
		cout << source << endl;
		cout << destination << endl;
		cout << current << endl;
	}
	assert(directions.size() > 0 && directions.size() <= 2);
	return directions;
}
/*=============================Odd Even-3D==========================*/
vector<int> NoximRouter::routingOddEven_for_3D(const NoximCoord& current,
	const NoximCoord& source, const NoximCoord& destination)
{
	vector<int> directions;

	int c0 = current.y;
	int c1 = current.z;
	int s0 = source.y;
	//  int s1 = source.y;
	int d0 = destination.y;
	int d1 = destination.z;
	int e0, e1;

	e0 = d0 - c0;
	e1 = d1 - c1;

	if (e0 == 0) {
		if (e1 > 0)
			directions.push_back(DIRECTION_DOWN);
		else if (e1 < 0)
			directions.push_back(DIRECTION_UP);
	}
	else {
		if (e0 > 0) {
			if (e1 == 0)
				directions.push_back(DIRECTION_SOUTH);
			else {
				if ((c0 % 2 == 1) || (c0 == s0)) { //Odd
					if (e1 > 0)
						directions.push_back(DIRECTION_DOWN);
					else if (e1 < 0)
						directions.push_back(DIRECTION_UP);
				}
				if ((d0 % 2 == 1) || (e0 != 1))
					directions.push_back(DIRECTION_SOUTH);
			}
		}
		else {
			directions.push_back(DIRECTION_NORTH);
			if (c0 % 2 == 0) {
				if (e1 > 0)
					directions.push_back(DIRECTION_DOWN);
				if (e1 < 0)
					directions.push_back(DIRECTION_UP);
			}
		}
	}
	return directions;
}
vector<int> NoximRouter::routingOddEven_3D(const NoximCoord& current,
	const NoximCoord& source, const NoximCoord& destination)
{
	vector<int> directions;

	int c0 = current.x;
	int c1 = current.y;
	int c2 = current.z;
	int s0 = source.x;
	int s1 = source.y;
	int s2 = source.z;
	int d0 = destination.x;
	int d1 = destination.y;
	int d2 = destination.z; //z = 0, which is far from heat sink
	int e0, e1, e2;

	e0 = d0 - c0;
	e1 = -(d1 - c1); //positive: North, negative: South
	e2 = d2 - c2; //positive: Down, negative: Up

	if (e0 == 0) {
		directions = routingOddEven_for_3D(current, source, destination);
	}
	else {
		if (e0 < 0) { //x-
			if ((c0 % 2 == 0)) {
				directions = routingOddEven_for_3D(current, source, destination);
			}
			directions.push_back(DIRECTION_WEST);
		}
		else { //e0 > 0, x+
			if ((e1 == 0) && (e2 == 0))
				directions.push_back(DIRECTION_EAST);
			else {
				if ((d0 % 2 == 1) || (e0 != 1)) {
					directions.push_back(DIRECTION_EAST);
				}
				if ((c0 % 2 == 1) || (c0 == s0)) {
					directions = routingOddEven_for_3D(current, source, destination);
				}
			}
		}
	}
	if (!(directions.size() > 0 && directions.size() <= 3)) {
		cout << "\n STAMPACCHIO :";
		cout << source << endl;
		cout << destination << endl;
		cout << current << endl;
	}
	//assert(directions.size() > 0 && directions.size() <= 3);

	return directions;
}
//=============================Odd Even + Downward==========================
//آآھ©،A¦Y¥‏°ىھ؛Downward tag

vector<int> NoximRouter::routingOddEven_Downward(const NoximCoord& current,
	const NoximCoord& source, const NoximCoord& destination, const NoximRouteData& route_data)
{
	vector<int> directions;
	int layer;	//ھي¥ـ¦¹packetہ³¸س¦b­‏­سlayer°µXY¶ا»¼
				/*int down_level = NoximGlobalParams::down_level;
				if( (source.z + down_level)> NoximGlobalParams::mesh_dim_z-1)
				layer = NoximGlobalParams::mesh_dim_z-1;
				else
				layer = source.z + down_level;*/
	layer = NoximGlobalParams::mesh_dim_z - 1;
	if (current.z < layer && (current.x != destination.x || current.y != destination.y))
	{

		if (current.z == destination.z && ((current.x - destination.x == 1 && current.y == destination.y)	//X or Y¤è¦V¤W¥u¬غ¶Z¤@®و®ة´Nھ½±µ¹L¥h,¤£DW
			|| (current.x - destination.x == -1 && current.y == destination.y)
			|| (current.y - destination.y == 1 && current.x == destination.x)
			|| (current.y - destination.y == -1 && current.x == destination.x)))
		{
			directions = routingOddEven(current, source, destination);
		}
		else
		{
			directions.push_back(DIRECTION_DOWN);
		}
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y))	//¦b©³³،¥Hxy routing¶ا
	{
		directions = routingOddEven(current, source, destination);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z)	//xy¬غ¦P, z¤è¦V©¹¤W¶ا
	{
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z)  //xy¬غ¦P, z¤è¦V©¹¤U¶ا
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else
	{
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		exit(1);
	}
	assert(directions.size() > 0 && directions.size() <= 3);
	return directions;
}
vector<int> NoximRouter::routingOddEven_Z(const NoximCoord& current,
	const NoximCoord& source, const NoximCoord& destination)
{
	vector<int> directions;

	if (current.x == destination.x && current.y == destination.y) {
		if (current.z < destination.z)
			directions.push_back(DIRECTION_DOWN);
		else
			directions.push_back(DIRECTION_UP);
	}
	else
		directions = routingOddEven(current, source, destination);

	if (!(directions.size() > 0 && directions.size() <= 3)) {
		cout << "\n STAMPACCHIO :";
		cout << source << endl;
		cout << destination << endl;
		cout << current << endl;
	}
	assert(directions.size() > 0 && directions.size() <= 3);
	return directions;
}

vector < int >NoximRouter::routingDyAD(const NoximCoord & current,
	const NoximCoord & source,
	const NoximCoord & destination)
{
	vector < int >directions;

	directions = routingOddEven(current, source, destination);

	if (!inCongestion())
		directions.resize(1);

	return directions;
}

vector<int> NoximRouter::routingFullyAdaptive(const NoximCoord& current, const NoximCoord& destination) {
	vector < int >directions;
	/*
	if        (destination.x == current.x || destination.y == current.y)
	return routingXYZ(current, destination);
	*/
	if (destination.z > current.z) {
		if (destination.x == current.x || destination.y == current.y)
			return routingXYZ(current, destination);

		if (destination.x > current.x && destination.y < current.y) {
			directions.push_back(DIRECTION_NORTH);
			directions.push_back(DIRECTION_EAST);
			directions.push_back(DIRECTION_DOWN);
		}
		else if (destination.x > current.x && destination.y > current.y) {
			directions.push_back(DIRECTION_SOUTH);
			directions.push_back(DIRECTION_EAST);
			directions.push_back(DIRECTION_DOWN);
		}
		else if (destination.x < current.x && destination.y > current.y) {
			directions.push_back(DIRECTION_SOUTH);
			directions.push_back(DIRECTION_WEST);
			directions.push_back(DIRECTION_DOWN);
		}
		else {
			directions.push_back(DIRECTION_NORTH);
			directions.push_back(DIRECTION_WEST);
			directions.push_back(DIRECTION_DOWN);
		}
	}
	else if (destination.z < current.z) {
		if (destination.x == current.x || destination.y == current.y)
			return routingXYZ(current, destination);

		if (destination.x > current.x && destination.y < current.y) {
			directions.push_back(DIRECTION_NORTH);
			directions.push_back(DIRECTION_EAST);
			directions.push_back(DIRECTION_UP);
		}
		else if (destination.x > current.x && destination.y > current.y) {
			directions.push_back(DIRECTION_SOUTH);
			directions.push_back(DIRECTION_EAST);
			directions.push_back(DIRECTION_UP);
		}
		else if (destination.x < current.x && destination.y > current.y) {
			directions.push_back(DIRECTION_SOUTH);
			directions.push_back(DIRECTION_WEST);
			directions.push_back(DIRECTION_UP);
		}
		else {
			directions.push_back(DIRECTION_NORTH);
			directions.push_back(DIRECTION_WEST);
			directions.push_back(DIRECTION_UP);
		}
	}
	else {
		if (destination.x == current.x || destination.y == current.y)
			return routingXYZ(current, destination);

		if (destination.x > current.x && destination.y < current.y) {
			directions.push_back(DIRECTION_NORTH);
			directions.push_back(DIRECTION_EAST);
			//                directions.push_back(DIRECTION_UP);
		}
		else if (destination.x > current.x && destination.y > current.y) {
			directions.push_back(DIRECTION_SOUTH);
			directions.push_back(DIRECTION_EAST);
			//              directions.push_back(DIRECTION_UP);
		}
		else if (destination.x < current.x && destination.y > current.y) {
			directions.push_back(DIRECTION_SOUTH);
			directions.push_back(DIRECTION_WEST);
			//            directions.push_back(DIRECTION_UP);
		}
		else {
			directions.push_back(DIRECTION_NORTH);
			directions.push_back(DIRECTION_WEST);
			//          directions.push_back(DIRECTION_UP);
		}


	}


	return directions;
}
vector<int> NoximRouter::routingTableBased(const NoximCoord& current, const int dir_in, const NoximCoord& destination) {
	NoximAdmissibleOutputs ao =
		routing_table.getAdmissibleOutputs(dir_in, coord2Id(destination));

	if (ao.size() == 0) {
		cout << "dir: " << dir_in << ", (" << current.x << "," << current.
			y << ") --> " << "(" << destination.x << "," << destination.
			y << ")" << endl << coord2Id(current) << "->" <<
			coord2Id(destination) << endl;
	}
	assert(ao.size() > 0);
	//-----
	/*
	vector<int> aov = admissibleOutputsSet2Vector(ao);
	cout << "dir: " << dir_in << ", (" << current.x << "," << current.y << ") --> "
	<< "(" << destination.x << "," << destination.y << "), outputs: ";
	for (int i=0; i<aov.size(); i++)
	cout << aov[i] << ", ";
	cout << endl;
	*/
	//-----
	return admissibleOutputsSet2Vector(ao);
}
vector<int> NoximRouter::routingDLADR(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination, const int select_routing, int dw_layer) {
	switch (select_routing) {
	case ROUTING_XYZ:return routingXYZ(current, destination); break;
	case ROUTING_WEST_FIRST:return routingWestFirst(current, destination); break;
		//case ROUTING_FULLY_ADAPTIVE      :return routingFullyAdaptive(                 current, destination                   );break;
	case ROUTING_ODD_EVEN_3D:return routingOddEven_3D(current, source, destination); break;
	case ROUTING_DOWNWARD_CROSS_LAYER:return DW_layerSelFunction(select_routing, current, destination, source, dw_layer); break;
	default:cout << getCurrentCycleNum() << ":Wrong with Cross-Layer!" << endl;
		cout << "Current    :" << current << endl;
		cout << "Source     :" << source << endl;
		cout << "Destination:" << destination << endl;
		cout << "Select routing:" << select_routing << endl; assert(false); break;
	}
}
vector<int> NoximRouter::routingDLAR(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination, const int select_routing, int dw_layer) {
	switch (select_routing) {
	case ROUTING_WEST_FIRST:return routingOddEven_Z(current, source, destination); break;
	case ROUTING_XYZ:return DW_layerSelFunction(select_routing, current, destination, source, dw_layer); break;
	case ROUTING_DOWNWARD_CROSS_LAYER:return DW_layerSelFunction(select_routing, current, destination, source, dw_layer); break;
	default:cout << "Wrong with Cross-Layer!" << select_routing << endl; assert(false); break;
	}
}
vector<int> NoximRouter::routingDLDR(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination, const int select_routing, int dw_layer) {
	switch (select_routing) {
	case ROUTING_XYZ:return routingXYZ(current, destination); break;
	case ROUTING_WEST_FIRST:return routingXYZ(current, destination); break;
	case ROUTING_DOWNWARD_CROSS_LAYER:return DW_layerSelFunction(select_routing, current, destination, source, dw_layer); break;
	default:cout << "Wrong with Cross-Layer!" << endl; assert(false); break;
	}
}
vector<int> NoximRouter::routingTLAR_DW(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination) {
	vector<int> directions;
	int layer = NoximGlobalParams::mesh_dim_z - 1;	//ھي¥ـ¦¹packetہ³¸س¦b­‏­سlayer°µXY¶ا»¼
	if (current.z < layer && (current.x != destination.x || current.y != destination.y))
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y))	//¦b©³³،¥Hxy routing¶ا
	{
		switch (NoximGlobalParams::routing_algorithm) {
		case ROUTING_DLAR: directions = routingOddEven_Z(current, source, destination); break;
		case ROUTING_DLADR: directions = routingWestFirst(current, destination); break;
		default:	directions = routingXYZ(current, destination); break;
		}
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z)
	{
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z)
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else
	{
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		exit(1);
	}
	return directions;
}
vector<int> NoximRouter::routingTLAR_DW_VBDR(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination) {
	vector<int> directions;
	int max = 0;
	int layer = 3;
	int free_slot[4];
	NoximNoP_data nop_tmp[4];
	for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++) {
		nop_tmp[i] = vertical_free_slot_in[i].read();
		free_slot[i] = 0;
	}
	if (destination.x > current.x) {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_EAST].free_slots;
	}
	else {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_WEST].free_slots;
	}
	if (destination.y > current.y) {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_SOUTH].free_slots;
	}
	else {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_NORTH].free_slots;
	}
	for (int i = 2; i < NoximGlobalParams::mesh_dim_z; i++) {
		if (free_slot[i] > max) {
			layer = i;
			max = free_slot[i];
		}
	}

	if (current.z < layer && (current.x != destination.x || current.y != destination.y))
	{
		//Increasing Path Diversity
		if (!on_off_neighbor[0] && !on_off_neighbor[1] && !on_off_neighbor[2] && !on_off_neighbor[3])
			directions = routingXYZ(current, destination);
		directions.push_back(DIRECTION_DOWN);
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y))	//¦b©³³،¥Hxy routing¶ا
	{
		switch (NoximGlobalParams::routing_algorithm) {
		case ROUTING_DLAR: directions = routingOddEven_Z(current, source, destination); break;
		case ROUTING_DLADR: directions = routingWestFirst(current, destination); break;
		default:	directions = routingXYZ(current, destination); break;
		}
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z)	//xy¬غ¦P, z¤è¦V©¹¤W¶ا
	{
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z)  //xy¬غ¦P, z¤è¦V©¹¤U¶ا
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else
	{
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		exit(1);
	}
	return directions;
}
vector<int> NoximRouter::routingTLAR_DW_ODWL_IPD(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination, int dw_layer) {
	vector<int> directions;
	int layer = dw_layer;
	if (current.z < layer && (current.x != destination.x || current.y != destination.y)) {
		if (NoximGlobalParams::routing_algorithm == ROUTING_DLAR)
			directions = routingOddEven(current, source, destination);
		else if (NoximGlobalParams::routing_algorithm == ROUTING_DLADR)
			directions = routingWestFirst(current, destination);
		else
			directions = routingXYZ(current, destination);

		for (int i = directions.size() - 1; i >= 0; i--) {
			if (directions[i] < 4) //for lateral direction
				if (on_off_neighbor[directions[i]].read() == 1) {//if the direction is throttled
					directions.erase(directions.begin() + i);//then erase that way	
				}
		}

		if (current.z == destination.z && ((current.x - destination.x == 1 && current.y == destination.y)	//while the dest. is one hop to source, don't downward and transmit directly
			|| (current.x - destination.x == -1 && current.y == destination.y)
			|| (current.y - destination.y == 1 && current.x == destination.x)
			|| (current.y - destination.y == -1 && current.x == destination.x)))
			directions = routingOddEven(current, source, destination);
		else
			directions.push_back(DIRECTION_DOWN);
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y)) {	//¦b©³³،¥Hxy routing¶ا
		switch (NoximGlobalParams::routing_algorithm) {
		case ROUTING_DLAR: directions = routingOddEven_Z(current, source, destination); break;
		case ROUTING_DLADR: directions = routingWestFirst(current, destination); break;
		default:	directions = routingXYZ(current, destination); break;
		}
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z) {	//xy¬غ¦P, z¤è¦V©¹¤W¶ا
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z) {  //xy¬غ¦P, z¤è¦V©¹¤U¶ا
		directions.push_back(DIRECTION_DOWN);
	}
	else {
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current:     " << current << endl;
		cout << "destination: " << destination << endl;
		cout << "layer:       " << layer << endl;
		exit(1);
	}
	if (directions.size() == 0) {
		cout << "current:     " << current << endl;
		cout << "source:      " << source << endl;
		cout << "destination: " << destination << endl;
		assert(false);
	}
	return directions;
}
vector<int> NoximRouter::routingTLAR_DW_ADWL(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination) {
	vector<int> directions;
	int max = 0;
	int layer = 3;              //ھي¥ـ¦¹packetہ³¸س¦b­‏­سlayer°µXY¶ا»¼
	int free_slot[4];
	NoximNoP_data nop_tmp[4];
	for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++) {
		nop_tmp[i] = vertical_free_slot_in[i].read();
		free_slot[i] = 0;
	}
	if (destination.x > current.x) {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_EAST].free_slots;
	}
	else {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_WEST].free_slots;
	}
	if (destination.y > current.y) {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_SOUTH].free_slots;
	}
	else {
		for (int i = 0; i < NoximGlobalParams::mesh_dim_z; i++)
			free_slot[i] = nop_tmp[i].channel_status_neighbor[DIRECTION_NORTH].free_slots;
	}
	for (int i = 3; i < NoximGlobalParams::mesh_dim_z; i++) {
		if (free_slot[i] > max) {
			layer = i;
			max = free_slot[i];
		}
	}

	if (current.z < layer && (current.x != destination.x || current.y != destination.y))
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y))	//¦b©³³،¥Hxy routing¶ا
	{
		switch (NoximGlobalParams::routing_algorithm) {
		case ROUTING_DLAR: directions = routingOddEven_Z(current, source, destination); break;
		case ROUTING_DLADR: directions = routingWestFirst(current, destination); break;
		default:	directions = routingXYZ(current, destination); break;
		}
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z)	//xy¬غ¦P, z¤è¦V©¹¤W¶ا
	{
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z)  //xy¬غ¦P, z¤è¦V©¹¤U¶ا
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else
	{
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		exit(1);
	}
	return directions;
}
vector<int> NoximRouter::routingTLAR_DW_ODWL(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination, int dw_layer) {
	vector<int> directions;
	int layer = dw_layer;

	if (current.z < layer && (current.x != destination.x || current.y != destination.y))
	{
		//Increasing Path Diversity
		if (!on_off_neighbor[0] && !on_off_neighbor[1] && !on_off_neighbor[2] && !on_off_neighbor[3] && (current.z == source.z)) {
			if (NoximGlobalParams::routing_algorithm == ROUTING_DLAR) {
				directions = routingOddEven_Z(current, source, destination);
				bool adaptive = true;
				for (int i = 0; i < directions.size(); i++) {
					NoximCoord sour = source;
					NoximCoord fake = id2Coord(getNeighborId(local_id, directions[i]));
					if (!Adaptive_ok(sour, fake))
						adaptive = false;
				}
				if (!adaptive)directions.clear();
			}
			else
				directions = routingXYZ(current, destination);
		}
		if (current.z == destination.z && ((current.x - destination.x == 1 && current.y == destination.y)	//while the dest. is one hop to source, don't downward and transmit directly
			|| (current.x - destination.x == -1 && current.y == destination.y)
			|| (current.y - destination.y == 1 && current.x == destination.x)
			|| (current.y - destination.y == -1 && current.x == destination.x)))
			directions = routingOddEven_Z(current, source, destination);
		else
			directions.push_back(DIRECTION_DOWN);
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y))	//¦b©³³،¥Hxy routing¶ا
	{
		switch (NoximGlobalParams::routing_algorithm) {
		case ROUTING_DLAR: directions = routingOddEven_Z(current, source, destination); break;
		case ROUTING_DLADR: directions = routingWestFirst(current, destination); break;
		default:	directions = routingXYZ(current, destination); break;
		}
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z)	//xy¬غ¦P, z¤è¦V©¹¤W¶ا
	{
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z)  //xy¬غ¦P, z¤è¦V©¹¤U¶ا
	{
		directions.push_back(DIRECTION_DOWN);
	}
	else
	{
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "Router ID:" << local_id << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		assert(0);
		exit(1);
	}
	assert(directions.size() > 0);
	assert(directions.size() <= 3);
	return directions;
}
vector<int> NoximRouter::routingTLAR_DW_IPD(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination) {
	vector<int> directions;
	int layer = NoximGlobalParams::mesh_dim_z - 1;

	if (current.z < layer && (current.x != destination.x || current.y != destination.y)) {
		//Increasing Path Diversity
		if (!on_off_neighbor[0] && !on_off_neighbor[1] && !on_off_neighbor[2] && !on_off_neighbor[3] && (current.z == source.z)) {
			if (NoximGlobalParams::routing_algorithm == ROUTING_DLAR) {
				directions = routingOddEven_Z(current, source, destination);
				bool adaptive = true;
				for (int i = 0; i < directions.size(); i++) {
					NoximCoord sour = source;
					NoximCoord fake = id2Coord(getNeighborId(local_id, directions[i]));
					if (!Adaptive_ok(sour, fake))
						adaptive = false;
				}
				if (!adaptive)directions.clear();
			}
			else
				directions = routingXYZ(current, destination);
		}
		directions.push_back(DIRECTION_DOWN);
	}
	else if (current.z >= layer && (current.x != destination.x || current.y != destination.y)) {
		switch (NoximGlobalParams::routing_algorithm) {
		case ROUTING_DLAR: directions = routingOddEven_Z(current, source, destination); break;
		case ROUTING_DLADR: directions = routingWestFirst(current, destination); break;
		default:	directions = routingXYZ(current, destination); break;
		}
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z > destination.z) {
		directions.push_back(DIRECTION_UP);
	}
	else if ((current.x == destination.x && current.y == destination.y) && current.z < destination.z) {
		directions.push_back(DIRECTION_DOWN);
	}
	else {
		cout << "ERROR! Out of condition!?!?" << endl;
		cout << "current: (" << current.x << "," << current.y << "," << current.z << ")" << endl;
		cout << "destination: (" << destination.x << "," << destination.y << "," << destination.z << ")" << endl;
		cout << "layer: " << layer << endl;
		exit(1);
	}
	return directions;
}

void NoximRouter::configure(const int _id,
	const double _warm_up_time,
	const unsigned int _max_buffer_size,
	NoximGlobalRoutingTable & grt)
{
	local_id = _id;
	stats.configure(_id, _warm_up_time);

	start_from_port = DIRECTION_LOCAL;

	if (grt.isValid())
		routing_table.configure(grt, _id);

	//if( !NoximGlobalParams::buffer_alloc )
	for (int i = 0; i < DIRECTIONS + 1; i++)
		//for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
			buffer[i][0].SetMaxBufferSize(_max_buffer_size);
	/*else{
	int z = id2Coord(_id).z;
	switch(z){
	case 0:
	for (int i = 0; i < 4; i++)
	buffer[i].SetMaxBufferSize(BUFFER_L_LAYER_0);
	buffer[DIRECTION_DOWN].SetMaxBufferSize(BUFFER_D_LAYER_0);
	buffer[DIRECTION_UP  ].SetMaxBufferSize(BUFFER_U_LAYER_0);
	break;
	case 1:
	for (int i = 0; i < 4; i++)
	buffer[i].SetMaxBufferSize(BUFFER_L_LAYER_1);
	buffer[DIRECTION_DOWN].SetMaxBufferSize(BUFFER_D_LAYER_1);
	buffer[DIRECTION_UP  ].SetMaxBufferSize(BUFFER_U_LAYER_1);
	break;
	case 2:
	for (int i = 0; i < 4; i++)
	buffer[i].SetMaxBufferSize(BUFFER_L_LAYER_2);
	buffer[DIRECTION_DOWN].SetMaxBufferSize(BUFFER_D_LAYER_2);
	buffer[DIRECTION_UP  ].SetMaxBufferSize(BUFFER_U_LAYER_2);
	break;
	case 3:
	for (int i = 0; i < 4; i++)
	buffer[i].SetMaxBufferSize(BUFFER_L_LAYER_3);
	buffer[DIRECTION_DOWN].SetMaxBufferSize(BUFFER_D_LAYER_3);
	buffer[DIRECTION_UP  ].SetMaxBufferSize(BUFFER_U_LAYER_3);
	break;
	}

	}*/
}

void NoximRouter::TBDB(float consumption_rate)
{
	NoximCoord local = id2Coord(local_id);

	float Temp_diff_east = (stats.pre_temperature1 - stats.temperature) - PDT_neighbor[DIRECTION_EAST].read();
	float Temp_diff_west = (stats.pre_temperature1 - stats.temperature) - PDT_neighbor[DIRECTION_WEST].read();
	float Temp_diff_north = (stats.pre_temperature1 - stats.temperature) - PDT_neighbor[DIRECTION_NORTH].read();
	float Temp_diff_south = (stats.pre_temperature1 - stats.temperature) - PDT_neighbor[DIRECTION_SOUTH].read();
	float Temp_diff_up = (stats.pre_temperature1 - stats.temperature) - PDT_neighbor[DIRECTION_UP].read();
	float Temp_diff_down = (stats.pre_temperature1 - stats.temperature) - PDT_neighbor[DIRECTION_DOWN].read();
	/*
	if(local.x == 4 && local.y == 4 ){    //traffic analysis
	buffer[DIRECTION_WEST].SetMaxBufferSize(2);
	}

	if(local.x == 3 && local.y == 3 ){    //traffic analysis
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(2);
	}

	if(local.x == 2 && local.y == 4 ){    //traffic analysis
	buffer[DIRECTION_EAST].SetMaxBufferSize(2);
	}

	if(local.x == 3 && local.y == 5 ){    //traffic analysis
	buffer[DIRECTION_NORTH].SetMaxBufferSize(2);
	}

	if(local.x == 3 && local.y == 4 ){    //traffic analysis

	buffer[DIRECTION_NORTH].SetMaxBufferSize(6);
	buffer[DIRECTION_WEST].SetMaxBufferSize(6);
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(6);
	buffer[DIRECTION_EAST].SetMaxBufferSize(6);
	}
	*/
	/*
	float Tempdiff[6] = {Temp_diff_east, Temp_diff_west, Temp_diff_north, Temp_diff_south, Temp_diff_up, Temp_diff_down};
	bubbleSort(Tempdiff, 6);
	for(int j = 0; j < 6; j++) {
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(2);
	}
	*/
	/*
	int east = 0;
	int west = 0;
	int north = 0;
	int south = 0;
	int up = 0;
	int down = 0;

	if(Temp_diff_east > Temp_diff_west){
	east++;
	west--;
	}
	else{
	east--;
	west++;
	}

	if(Temp_diff_east > Temp_diff_north){
	east++;
	north--;
	}
	else{
	east--;
	north++;
	}

	if(Temp_diff_east > Temp_diff_south){
	east++;
	south--;
	}
	else{
	east--;
	south++;
	}

	if(Temp_diff_east > Temp_diff_up){
	east++;
	up--;
	}
	else{
	east--;
	up++;
	}

	if(Temp_diff_east > Temp_diff_down){
	east++;
	down--;
	}
	else{
	east--;
	down++;
	}
	//-----------------
	if(Temp_diff_west > Temp_diff_north){
	west++;
	north--;
	}
	else{
	west--;
	north++;
	}
	//----------------
	if(Temp_diff_west > Temp_diff_south){
	west++;
	south--;
	}
	else{
	west--;
	south++;
	}
	//---------------
	if(Temp_diff_west > Temp_diff_up){
	west++;
	up--;
	}
	else{
	west--;
	up++;
	}
	//----------------
	if(Temp_diff_west > Temp_diff_down){
	west++;
	down--;
	}
	else{
	west--;
	down++;
	}

	//----------------
	//----------------

	if(Temp_diff_north > Temp_diff_south){
	north++;
	south--;
	}
	else{
	north--;
	south++;
	}

	//----------------
	//----------------

	if(Temp_diff_north > Temp_diff_up){
	north++;
	up--;
	}
	else{
	north--;
	up++;
	}

	//----------------
	//----------------

	if(Temp_diff_north > Temp_diff_down){
	north++;
	down--;
	}
	else{
	north--;
	down++;
	}

	//---------------
	//---------------
	//---------------

	if(Temp_diff_south > Temp_diff_down){
	south++;
	down--;
	}
	else{
	south--;
	down++;
	}

	//---------------
	//---------------
	//---------------

	if(Temp_diff_south > Temp_diff_up){
	south++;
	up--;
	}
	else{
	south--;
	up++;
	}

	//---------------
	//---------------
	//---------------
	//---------------

	if(Temp_diff_up > Temp_diff_down){
	up++;
	down--;
	}
	else{
	up--;
	down++;
	}

	if(stats.pre_temperature1 > 98){
	if(east==5)
	buffer[DIRECTION_EAST].SetMaxBufferSize(7);
	else if(east==3)
	buffer[DIRECTION_EAST].SetMaxBufferSize(6);
	else if(east==1)
	buffer[DIRECTION_EAST].SetMaxBufferSize(5);
	else if(east==-1)
	buffer[DIRECTION_EAST].SetMaxBufferSize(4);
	else if(east==-3)
	buffer[DIRECTION_EAST].SetMaxBufferSize(3);
	else if(east==-5)
	buffer[DIRECTION_EAST].SetMaxBufferSize(2);


	if(south==5)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(7);
	else if(south==3)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(6);
	else if(south==1)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(5);
	else if(south==-1)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(4);
	else if(south==-3)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(3);
	else if(south==-5)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(2);



	if(west==5)
	buffer[DIRECTION_WEST].SetMaxBufferSize(7);
	else if(west==3)
	buffer[DIRECTION_WEST].SetMaxBufferSize(6);
	else if(west==1)
	buffer[DIRECTION_WEST].SetMaxBufferSize(5);
	else if(west==-1)
	buffer[DIRECTION_WEST].SetMaxBufferSize(4);
	else if(west==-3)
	buffer[DIRECTION_WEST].SetMaxBufferSize(3);
	else if(west==-5)
	buffer[DIRECTION_WEST].SetMaxBufferSize(2);


	if(north==5)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(7);
	else if(north==3)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(6);
	else if(north==1)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(5);
	else if(north==-1)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(4);
	else if(north==-3)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(3);
	else if(north==-5)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(2);


	if(up==5)
	buffer[DIRECTION_UP].SetMaxBufferSize(7);
	else if(up==3)
	buffer[DIRECTION_UP].SetMaxBufferSize(6);
	else if(up==1)
	buffer[DIRECTION_UP].SetMaxBufferSize(5);
	else if(up==-1)
	buffer[DIRECTION_UP].SetMaxBufferSize(4);
	else if(up==-3)
	buffer[DIRECTION_UP].SetMaxBufferSize(3);
	else if(up==-5)
	buffer[DIRECTION_UP].SetMaxBufferSize(2);


	if(down==5)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(7);
	else if(down==3)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(6);
	else if(down==1)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(5);
	else if(down==-1)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(4);
	else if(down==-3)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(3);
	else if(down==-5)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(2);

	}*/
	for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
		if (Temp_diff_east < 0) {
			if (buffer[DIRECTION_EAST][vc].GetMaxBufferSize() > 2) {
				buffer[DIRECTION_EAST][vc].SetMaxBufferSize(buffer[DIRECTION_EAST][vc].GetMaxBufferSize() - 1);
				buf_budget++;
			}
		}
		if (Temp_diff_west < 0) {
			if (buffer[DIRECTION_WEST][vc].GetMaxBufferSize() > 2) {
				buffer[DIRECTION_WEST][vc].SetMaxBufferSize(buffer[DIRECTION_WEST][vc].GetMaxBufferSize() - 1);
				buf_budget++;
			}
		}
		if (Temp_diff_north < 0) {
			if (buffer[DIRECTION_NORTH][vc].GetMaxBufferSize() > 2) {
				buffer[DIRECTION_NORTH][vc].SetMaxBufferSize(buffer[DIRECTION_NORTH][vc].GetMaxBufferSize() - 1);
				buf_budget++;
			}
		}
		if (Temp_diff_south < 0) {
			if (buffer[DIRECTION_SOUTH][vc].GetMaxBufferSize() > 2) {
				buffer[DIRECTION_SOUTH][vc].SetMaxBufferSize(buffer[DIRECTION_SOUTH][vc].GetMaxBufferSize() - 1);
				buf_budget++;
			}
		}
		if (Temp_diff_up < 0) {
			if (buffer[DIRECTION_UP][vc].GetMaxBufferSize() > 2) {
				buffer[DIRECTION_UP][vc].SetMaxBufferSize(buffer[DIRECTION_UP][vc].GetMaxBufferSize() - 1);
				buf_budget++;
			}
		}
		if (Temp_diff_down < 0) {
			if (buffer[DIRECTION_DOWN][vc].GetMaxBufferSize() > 2) {
				buffer[DIRECTION_DOWN][vc].SetMaxBufferSize(buffer[DIRECTION_DOWN][vc].GetMaxBufferSize() - 1);
				buf_budget++;
			}
		}


		if (Temp_diff_east > 0 && buf_budget > 0) {
			if (buffer[DIRECTION_EAST][vc].GetMaxBufferSize() < 8) {
				buffer[DIRECTION_EAST][vc].SetMaxBufferSize(buffer[DIRECTION_EAST][vc].GetMaxBufferSize() + 1);
				buf_budget--;
			}
		}
		if (Temp_diff_west > 0 && buf_budget > 0) {
			if (buffer[DIRECTION_WEST][vc].GetMaxBufferSize() < 8) {
				buffer[DIRECTION_WEST][vc].SetMaxBufferSize(buffer[DIRECTION_WEST][vc].GetMaxBufferSize() + 1);
				buf_budget--;
			}
		}
		if (Temp_diff_north > 0 && buf_budget > 0) {
			if (buffer[DIRECTION_NORTH][vc].GetMaxBufferSize() < 8) {
				buffer[DIRECTION_NORTH][vc].SetMaxBufferSize(buffer[DIRECTION_NORTH][vc].GetMaxBufferSize() + 1);
				buf_budget--;
			}
		}
		if (Temp_diff_south > 0 && buf_budget > 0) {
			if (buffer[DIRECTION_SOUTH][vc].GetMaxBufferSize() < 8) {
				buffer[DIRECTION_SOUTH][vc].SetMaxBufferSize(buffer[DIRECTION_SOUTH][vc].GetMaxBufferSize() + 1);
				buf_budget--;
			}
		}
		if (Temp_diff_up > 0 && buf_budget > 0) {
			if (buffer[DIRECTION_UP][vc].GetMaxBufferSize() < 8) {
				buffer[DIRECTION_UP][vc].SetMaxBufferSize(buffer[DIRECTION_UP][vc].GetMaxBufferSize() + 1);
				buf_budget--;
			}
		}
		if (Temp_diff_down > 0 && buf_budget > 0) {
			if (buffer[DIRECTION_DOWN][vc].GetMaxBufferSize() < 8) {
				buffer[DIRECTION_DOWN][vc].SetMaxBufferSize(buffer[DIRECTION_DOWN][vc].GetMaxBufferSize() + 1);
				buf_budget--;
			}
		}
	}

	/*
	if(Temp_diff_east<0){
	if(buffer[DIRECTION_EAST].GetMaxBufferSize()>2)
	buffer[DIRECTION_EAST].SetMaxBufferSize(buffer[DIRECTION_EAST].GetMaxBufferSize()-1);
	}
	if(Temp_diff_east>0){
	if(buffer[DIRECTION_EAST].GetMaxBufferSize()<8)
	buffer[DIRECTION_EAST].SetMaxBufferSize(buffer[DIRECTION_EAST].GetMaxBufferSize()+1);
	}
	if(Temp_diff_west<0){
	if(buffer[DIRECTION_WEST].GetMaxBufferSize()>2)
	buffer[DIRECTION_WEST].SetMaxBufferSize(buffer[DIRECTION_WEST].GetMaxBufferSize()-1);
	}
	if(Temp_diff_west>0){
	if(buffer[DIRECTION_WEST].GetMaxBufferSize()<8)
	buffer[DIRECTION_WEST].SetMaxBufferSize(buffer[DIRECTION_WEST].GetMaxBufferSize()+1);
	}
	if(Temp_diff_north<0){
	if(buffer[DIRECTION_NORTH].GetMaxBufferSize()>2)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(buffer[DIRECTION_NORTH].GetMaxBufferSize()-1);
	}
	if(Temp_diff_north>0){
	if(buffer[DIRECTION_NORTH].GetMaxBufferSize()<8)
	buffer[DIRECTION_NORTH].SetMaxBufferSize(buffer[DIRECTION_NORTH].GetMaxBufferSize()+1);
	}
	if(Temp_diff_south<0){
	if(buffer[DIRECTION_SOUTH].GetMaxBufferSize()>2)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(buffer[DIRECTION_SOUTH].GetMaxBufferSize()-1);
	}
	if(Temp_diff_south>0){
	if(buffer[DIRECTION_SOUTH].GetMaxBufferSize()<8)
	buffer[DIRECTION_SOUTH].SetMaxBufferSize(buffer[DIRECTION_SOUTH].GetMaxBufferSize()+1);
	}
	if(Temp_diff_up<0){
	if(buffer[DIRECTION_UP].GetMaxBufferSize()>2)
	buffer[DIRECTION_UP].SetMaxBufferSize(buffer[DIRECTION_UP].GetMaxBufferSize()-1);
	}
	if(Temp_diff_up>0){
	if(buffer[DIRECTION_UP].GetMaxBufferSize()<8)
	buffer[DIRECTION_UP].SetMaxBufferSize(buffer[DIRECTION_UP].GetMaxBufferSize()+1);
	}
	if(Temp_diff_down<0){
	if(buffer[DIRECTION_DOWN].GetMaxBufferSize()>2)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(buffer[DIRECTION_DOWN].GetMaxBufferSize()-1);
	}
	if(Temp_diff_down>0){
	if(buffer[DIRECTION_DOWN].GetMaxBufferSize()<8)
	buffer[DIRECTION_DOWN].SetMaxBufferSize(buffer[DIRECTION_DOWN].GetMaxBufferSize()+1);
	}

	*/

	/*
	cout<<"Router:"<<local.x<<local.y<<local.z;
	cout<<"Temp_diff_east:"<<Temp_diff_east<<" buf_east:"<<buffer[DIRECTION_EAST].GetMaxBufferSize()<<endl;

	cout<<"Router:"<<local.x<<local.y<<local.z;
	cout<<"Temp_diff_west:"<<Temp_diff_west<<" buf_west:"<<buffer[DIRECTION_WEST].GetMaxBufferSize()<<endl;

	cout<<"Router:"<<local.x<<local.y<<local.z;
	cout<<"Temp_diff_north:"<<Temp_diff_north<<" buf_north:"<<buffer[DIRECTION_NORTH].GetMaxBufferSize()<<endl;

	cout<<"Router:"<<local.x<<local.y<<local.z;
	cout<<"Temp_diff_south:"<<Temp_diff_south<<" buf_south:"<<buffer[DIRECTION_SOUTH].GetMaxBufferSize()<<endl;

	cout<<"Router:"<<local.x<<local.y<<local.z;
	cout<<"Temp_diff_up:"<<Temp_diff_up<<" buf_up:"<<buffer[DIRECTION_UP].GetMaxBufferSize()<<endl;

	cout<<"Router:"<<local.x<<local.y<<local.z;
	cout<<"Temp_diff_down:"<<Temp_diff_down<<" buf_down:"<<buffer[DIRECTION_DOWN].GetMaxBufferSize()<<end
	if(MTTT[local.x][local.y][local.z] < 2){

	int buffer_dep;

	buffer_dep = ceil(NoximGlobalParams::buffer_depth*MTTT[local.x][local.y][local.z]);
	if(buffer_dep > NoximGlobalParams::buffer_depth)
	buffer_dep =  NoximGlobalParams::buffer_depth;
	else if(buffer_dep < 1)
	buffer_dep = 1;

	for (int i = 0; i < DIRECTIONS+2; i++)
	buffer[i].SetMaxBufferSize(buffer_dep);

	for (int i = 0; i < DIRECTIONS+2; i++)
	if(buffer[i].GetMaxBufferSize()>2)
	buffer[i].SetMaxBufferSize(buffer[i].GetMaxBufferSize()-2);
	}
	else{
	for (int i = 0; i < DIRECTIONS+2; i++)
	if(buffer[i].GetMaxBufferSize()<8)
	buffer[i].SetMaxBufferSize(buffer[i].GetMaxBufferSize()+2);

	}
	*/
}

unsigned long NoximRouter::getRoutedFlits()
{
	unsigned long total = 0;
	for (int i = 0; i < DIRECTIONS + 1; i++)
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
			total += routed_flits[i][vc];
	return total;
}

unsigned long NoximRouter::getRoutedDWFlits()
{
	return routed_DWflits;
}

unsigned long NoximRouter::getRoutedFlits(int i, int vc)
{
	return routed_flits[i][vc];  //vc 0  --Need to review
}

unsigned long NoximRouter::getWaitingTime(int i, int vc)
{
	return waiting[i][vc];
}
unsigned long NoximRouter::getTotalWaitingTime()
{
	return _total_waiting;
}
unsigned long NoximRouter::getRoutedPackets()
{
	return routed_packets;
}
void NoximRouter::CalcDelay() {
	_buffer_pkt_count = 0;
	_buffer_pkt_nw_delay = 0;
	_buffer_pkt_ni_delay = 0;
	_buffer_pkt_msg_delay = 0;
	for (int i = 0; i < DIRECTIONS + 1; i++) {
		while (!buffer[i][0].IsEmpty()) {
			NoximFlit flit = buffer[i][0].Front();
			if (flit.flit_type == FLIT_TYPE_HEAD) {
				_buffer_pkt_msg_delay += (int)getCurrentCycleNum() - (int)flit.timestamp;
				_buffer_pkt_ni_delay += (int)getCurrentCycleNum() - (int)flit.timestamp_ni;
				_buffer_pkt_nw_delay += (int)getCurrentCycleNum() - (int)flit.timestamp_nw;
				_buffer_pkt_count++;
			}
			buffer[i][0].Pop();
		}
	}
}

int NoximRouter::getFlitRoute(int i) {
	NoximFlit flit = buffer[i][0].Front();
	NoximRouteData route_data;
	route_data.current_id = local_id;
	route_data.src_id = (flit.arr_mid) ? flit.mid_id : flit.src_id;
	route_data.dst_id = (flit.arr_mid) ? flit.dst_id : flit.mid_id;
	route_data.dir_in = i;
	route_data.routing = flit.routing_f;
	route_data.DW_layer = flit.DW_layer;
	route_data.arr_mid = flit.arr_mid;
	//taheri
	return route(route_data, &flit.south, &flit.east, &flit.vc);
}

int NoximRouter::getDirAvailable(int i, int vc) {

	return reservation_table.isAvailable(i, vc);

}
/////////////////////////////////////////////////////////////
////////////////////////////////////////////////	power
double NoximRouter::getRouterPower()
{
	//return stats.power.getPower();
	//return stats.power.getTransientRouterPower();
	//return stats.power.getSteadyStateRouterPower();
	return stats.power.getDynamicPower() + stats.power.getStaticPowerSteadyState() + (stats.power.getStaticPowerTransient() * (getCurrentCycleNum()% TEMP_REPORT_NUM));
}

double NoximRouter::getCorePower()
{
	return stats.power.getCorePower() + (stats.power.getCoreStaticPower() * getCurrentCycleNum());
}

double NoximRouter::getDynamicPower()
{
	return stats.power.getDynamicPower();
}

double NoximRouter::getStaticPower()
{
	return (stats.power.getStaticPowerSteadyState() + (stats.power.getStaticPowerTransient() * (getCurrentCycleNum() % TEMP_REPORT_NUM)));
}

int NoximRouter::reflexDirection(int direction) const
{
	switch (direction) {
	case DIRECTION_NORTH:return DIRECTION_SOUTH; break;
	case DIRECTION_EAST:return DIRECTION_WEST; break;
	case DIRECTION_WEST:return DIRECTION_EAST; break;
	case DIRECTION_SOUTH:return DIRECTION_NORTH; break;
	case DIRECTION_UP:return DIRECTION_DOWN; break;
	case DIRECTION_DOWN:return DIRECTION_UP; break;
	default:return NOT_VALID; break;
	}
}

int NoximRouter::getNeighborId(int _id, int direction) const
{
	NoximCoord my_coord = id2Coord(_id);
	switch (direction) {
	case DIRECTION_NORTH: if (my_coord.y == 0) return NOT_VALID; my_coord.y--; break;
	case DIRECTION_SOUTH: if (my_coord.y == NoximGlobalParams::mesh_dim_y - 1) return NOT_VALID; my_coord.y++; break;
	case DIRECTION_EAST: if (my_coord.x == NoximGlobalParams::mesh_dim_x - 1) return NOT_VALID; my_coord.x++; break;
	case DIRECTION_WEST: if (my_coord.x == 0) return NOT_VALID; my_coord.x--; break;
	case DIRECTION_UP: if (my_coord.z == 0) return NOT_VALID; my_coord.z--; break;
	case DIRECTION_DOWN: if (my_coord.z == NoximGlobalParams::mesh_dim_z - 1) return NOT_VALID; my_coord.z++; break;
	default: cout << "direction not valid : " << direction; assert(false);
	}
	return coord2Id(my_coord);
}

void NoximRouter::TraffThrottlingProcess()	//�Y�bemergency mode, �Btraffic�W�Ltraffic quota, �hthrottle
{

	NoximCoord local = id2Coord(local_id);
	if (reset.read())for (int i = 0; i<DIRECTIONS; i++) {
		on_off[i].write(0);
		TB[i].write(0);
		buf[0][i].write(0);
		buf[1][i].write(0);
		buf[2][i].write(0);
		buf[3][i].write(0);
		buf[4][i].write(0);
		buf[5][i].write(0);
		buf[6][i].write(0);
		buf[7][i].write(0);
		RST = 0;
		DFS = 8;
		//free_slots_PE[i].write( free_slots_neighbor[i]);
	}
	else              for (int i = 0; i<DIRECTIONS; i++) {
		on_off[i].write(_emergency);
		TB[i].write(MTTT[local.x][local.y][local.z]);
		PDT[i].write(stats.pre_temperature1 - stats.temperature);
		if (throttling[local.x][local.y][local.z]) {
			buf[0][i].write(100);
			buf[1][i].write(100);
			buf[2][i].write(100);
			buf[3][i].write(100);
			buf[4][i].write(100);
			buf[5][i].write(100);
			buf[6][i].write(100);
			buf[7][i].write(100);
		}
		else {
			/*
			if(MTTT[local.x][local.y][local.z] < 0.5 && MTTT[local.x][local.y][local.z] != 0){
			if(RST<4)
			RST++;
			}
			else{
			if(RST>0)
			RST--;
			}
			*/
			//if(buffer[DIRECTION_SOUTH].IsEmpty())
			//	buf[0][i].write(RST + buffer[DIRECTION_SOUTH].GetMaxBufferSize());
			//else
			buf[0][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_SOUTH][0].Size());//buffer[DIRECTION_SOUTH].GetMaxBufferSize());

																								 //if(buffer[DIRECTION_WEST].IsEmpty())
																								 //	buf[1][i].write(RST + buffer[DIRECTION_WEST].GetMaxBufferSize());
																								 //else
			buf[1][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_WEST][0].Size());//buffer[DIRECTION_WEST].GetMaxBufferSize());

																								//if(buffer[DIRECTION_NORTH].IsEmpty())
																								//	buf[2][i].write(RST + buffer[DIRECTION_NORTH].GetMaxBufferSize());
																								//else
			buf[2][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_NORTH][0].Size());//buffer[DIRECTION_NORTH].GetMaxBufferSize());

																								 //if(buffer[DIRECTION_EAST].IsEmpty())
																								 //	buf[3][i].write(RST + buffer[DIRECTION_EAST].GetMaxBufferSize());
																								 //else
			buf[3][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_EAST][0].Size());//buffer[DIRECTION_EAST].GetMaxBufferSize());

																								//if(buffer[DIRECTION_DOWN].IsEmpty())
																								//	buf[4][i].write(RST + buffer[DIRECTION_DOWN].GetMaxBufferSize());
																								//else
			buf[4][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_DOWN][0].Size());//buffer[DIRECTION_DOWN].GetMaxBufferSize());

																								//if(buffer[DIRECTION_UP].IsEmpty())
																								//	buf[5][i].write(RST + buffer[DIRECTION_UP].GetMaxBufferSize());
																								//else
			buf[5][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_UP][0].Size());//buffer[DIRECTION_UP].GetMaxBufferSize());

																							  //if(buffer[DIRECTION_LOCAL].IsEmpty())
																							  //	buf[6][i].write(RST + buffer[DIRECTION_LOCAL].GetMaxBufferSize());
																							  //else
			buf[6][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_LOCAL][0].Size());//buffer[DIRECTION_LOCAL].GetMaxBufferSize());

																								 //if(buffer[DIRECTION_SEMI_LOCAL].IsEmpty())
																								 //	buf[7][i].write(RST + buffer[DIRECTION_SEMI_LOCAL].GetMaxBufferSize());
																								 //else
			buf[7][i].write((1 + RST*0.125) + (1 + RST*0.125)*buffer[DIRECTION_SEMI_LOCAL][0].Size());//buffer[DIRECTION_SEMI_LOCAL].GetMaxBufferSize());

		}

		/*
		if(MTTT[local.x][local.y][local.z] < 0.5 && MTTT[local.x][local.y][local.z] != 0){
		if(RST<4)
		RST++;
		}
		else{
		if(RST>0)
		RST--;
		}
		*/
		//free_slots_PE[i].write( free_slots_neighbor[i]);
	}
	//for(int i=0; i<8; i++)
	//RCA_PE[i] = RCA_data_in[i];
	// free_slots_PE[i].write( free_slots_neighbor[i]);
}

void NoximRouter::IntoEmergency() {
	_emergency = true;
	if (_emergency_level < NoximGlobalParams::buffer_depth - 1)
		_emergency_level++;
}

void NoximRouter::OutOfEmergency() {
	_emergency = false;
	if (_emergency_level > 0)
		_emergency_level--;
}

bool NoximRouter::inCongestion()
{
	for (int i = 0; i < DIRECTIONS; i++) {
		int flits = NoximGlobalParams::buffer_depth - free_slots_neighbor[i];
		if (flits >(int) (NoximGlobalParams::buffer_depth * NoximGlobalParams::dyad_threshold))return true;
	}
	return false;
}
bool NoximRouter::Adaptive_ok(NoximCoord & sour, NoximCoord & dest)
{
	int x_diff = dest.x - sour.x;
	int y_diff = dest.y - sour.y;
	int z_diff = dest.z - sour.z;
	int x_search, y_search;

	for (int y_a = 0; y_a < abs(y_diff) + 1; y_a++)
		for (int x_a = 0; x_a < abs(x_diff) + 1; x_a++) {
			x_search = (x_diff > 0) ? (sour.x + x_a) : (sour.x - x_a);
			y_search = (y_diff > 0) ? (sour.y + y_a) : (sour.y - y_a);
			if (throttling[x_search][y_search][sour.z] == 1)
				return false;
		}
	return true;
}
int inRing(NoximCoord dest) {//Find the ring level of destination
	int a = dest.x;
	int b = NoximGlobalParams::mesh_dim_x - dest.x;
	int c = dest.y;
	int d = NoximGlobalParams::mesh_dim_y - dest.y;
	int e = (a < b) ? a : b;
	int f = (c < d) ? c : d;
	int g = (e < f) ? e : f;
	return g;
}
void  NoximRouter::DBA(int outgoing, NoximFlit* head) {
	NoximCoord local = id2Coord(local_id);
	NoximCoord cascade_node = id2Coord(head->mid_id);
	int cascade_node_ring = inRing(cascade_node);
	int ring = inRing(local);
	if (cascade_node_ring == 0 && ring != 0 && outgoing != DIRECTION_EAST && outgoing != DIRECTION_WEST) {

		if (cascade_node.x == 0)
			cascade_node.x += ring;
		else
			cascade_node.x -= ring;
		if (cascade_node.x == 0)
			cascade_node.y += ring;
		else
			cascade_node.y -= ring;
		if (Adaptive_ok(local, cascade_node))
			head->mid_id = coord2Id(cascade_node);
	}
}

