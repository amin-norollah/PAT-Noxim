#include "NoximVLink.h"

void NoximVLink::buildVLink(){
	start_from_layer_U = 0;
	start_from_layer_D = 0;
}

void NoximVLink::entry(){
	if( reset.read() ){
		reservation_table.clear();
		for( int i = 0 ; i < NoximGlobalParams::mesh_dim_z - 1 ; i++ )
			for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
				ack_rx_to_DOWN	[i][vc].write(1);
				ack_tx_to_UP	[i][vc].write(1);
			}
	}
	else{
		if      ( NoximGlobalParams::vertical_link == VERTICAL_MESH){

			Mesh();}
		else if ( NoximGlobalParams::vertical_link == VERTICAL_CROSSBAR){

			CrossBar_UP  ();
			CrossBar_DOWN();
		}
		else
			assert(false);
	}
}

void NoximVLink::CrossBar_UP() {
	NoximFlit tmp;
	int tmp_dst_z;
	//1st phase: Reserve
	for (int j = 0; j < NoximGlobalParams::mesh_dim_z - 1; j++) {
		int i = (start_from_layer_U + j) % (NoximGlobalParams::mesh_dim_z - 1);
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
			if (req_rx_to_DOWN[i][vc].read()) {
				tmp = flit_rx_to_DOWN[i].read();
				if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
					cout << getCurrentCycleNum() << ": VLink[" << _id <<
						"] received from [" << i << "], flit: "
						<< tmp << "start_from_layer_U =" << start_from_layer_U << endl;
				}
				//tmp_dst_z = id2Coord( tmp.dst_id ).z;
				ack_rx_to_DOWN[i][vc].write(0);
				if (tmp.flit_type == FLIT_TYPE_HEAD) {
					NoximRouteData route_data;
					route_data.current_id = _id + (i + 1)*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y;
					route_data.src_id = (tmp.arr_mid) ? tmp.mid_id : tmp.src_id;
					route_data.dst_id = (tmp.arr_mid) ? tmp.dst_id : tmp.mid_id;
					route_data.routing = tmp.routing_f;
					route_data.DW_layer = tmp.DW_layer;
					route_data.arr_mid = tmp.arr_mid;
					tmp_dst_z = routingFunction(route_data);

					for (int vc_out = 0; vc_out < NoximGlobalParams::num_vcs; vc_out++) {
						if (reservation_table.isAvailable(tmp_dst_z, vc_out)) {
							reservation_table.reserve(i, vc, tmp_dst_z, vc_out);
							if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
								cout << getCurrentCycleNum() << ": VLink[" << _id <<
									"] Input[" << i << "], reserved UP output[" << tmp_dst_z << "], flit: "
									<< tmp << endl;
							}
						}
					}
				}
			}
		}
	}
	start_from_layer_U++;

	for (int o = 0; o < NoximGlobalParams::mesh_dim_z - 1; o++)
		for (int vc_out = 0; vc_out < NoximGlobalParams::num_vcs; vc_out++) {
			if (ack_rx_to_UP[o][vc_out] == 1) {
				req_rx_to_UP[o][vc_out].write(0);
				//ack_rx_to_DOWN[o].write(1);
			}
		}
	for (int i = 0; i < NoximGlobalParams::mesh_dim_z - 1; i++)
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
			//2nd phase: Transmit
			int o		= reservation_table.getOutputPort(i, vc);
			int vc_out  = reservation_table.getOutputVc  (i, vc);
			if (o != NOT_RESERVED) {
				if (ack_rx_to_UP[o][vc_out] == 1) {
					tmp = flit_rx_to_DOWN[i].read();
					if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
						cout << getCurrentCycleNum() << ": VLink[" << _id << "], Input["
							<< i << "] UPward to Output[" << o << "], flit: "
							<< tmp << endl;
					}
					flit_rx_to_UP		[o]			.write(flit_rx_to_DOWN	[i].read());
					req_rx_to_UP		[o][vc_out]	.write(req_rx_to_DOWN	[i][vc].read());
					ack_rx_to_DOWN		[i][vc]		.write(ack_rx_to_UP		[o][vc_out].read());
					if (tmp.flit_type == FLIT_TYPE_TAIL) {
						reservation_table.release(o, vc_out);
						if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
							cout << getCurrentCycleNum() << ": VLink[" << _id << "], release Up Output[" << o << "]" << endl;
						}
					}
				}
			}
		}
}

void NoximVLink::CrossBar_DOWN(){
	NoximFlit tmp;
	int tmp_dst_z;
	//1st phase: Reserve
	for (int j = 0; j < NoximGlobalParams::mesh_dim_z - 1; j++) {
		int i = (start_from_layer_D + j) % (NoximGlobalParams::mesh_dim_z - 1);
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
			if (req_tx_to_UP[i][vc].read()) {
				tmp = flit_tx_to_UP[i].read();
				if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
					cout << getCurrentCycleNum() << ": VLink[" << _id <<
						"] received from [" << i << "], flit: "
						<< tmp << "start_from_layer_D =" << start_from_layer_D << endl;
				}
				if (tmp.dst_id > 15)
					cout << "sd" << endl;
				tmp_dst_z = id2Coord(tmp.dst_id).z;
				ack_tx_to_UP[i][vc].write(0);
				if (tmp.flit_type == FLIT_TYPE_HEAD) {
					NoximRouteData route_data;
					route_data.current_id = _id + i*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y;
					route_data.src_id = (tmp.arr_mid) ? tmp.mid_id : tmp.src_id;
					route_data.dst_id = (tmp.arr_mid) ? tmp.dst_id : tmp.mid_id;
					route_data.routing = tmp.routing_f;
					route_data.DW_layer = tmp.DW_layer;
					route_data.arr_mid = tmp.arr_mid;
					tmp_dst_z = routingFunction(route_data);

					for (int vc_out = 0; vc_out < NoximGlobalParams::num_vcs; vc_out++) {
						if (reservation_table.isAvailable(tmp_dst_z, vc_out)) {
							reservation_table.reserve(i, vc, tmp_dst_z, vc_out);
							if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
								cout << getCurrentCycleNum() << ": VLink[" << _id <<
									"] Input[" << i << "], reserved output[" << tmp_dst_z << "], flit: "
									<< tmp << endl;
							}
						}
					}
				}
			}
		}
	}
	start_from_layer_D++;
	for (int o = 0; o<NoximGlobalParams::mesh_dim_z - 1; o++)
		for (int vc_out = 0; vc_out < NoximGlobalParams::num_vcs; vc_out++)
			if (ack_tx_to_DOWN[o][vc_out] == 1) {
				req_tx_to_DOWN[o][vc_out].write(0);
				//ack_tx_to_UP[o][vc_out]  .write(1);
			}
	for( int i = 0 ; i < NoximGlobalParams::mesh_dim_z - 1 ; i++ )
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++){
		//2nd phase: Transmit
		int o		= reservation_table.getOutputPort(i,vc) ;
		int vc_out  = reservation_table.getOutputVc	 (i, vc);
		if ( o != NOT_RESERVED ){
			tmp       = flit_tx_to_UP[i].read();
			if( ack_tx_to_DOWN[o-1][vc_out] == 1 ){
				if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
					cout << getCurrentCycleNum() << ": VLink[" << _id<< "], Input[" 
					<< i <<	"] DOWNward to Output[" << o << "], flit: "
					<< tmp << endl;
				}
				flit_tx_to_DOWN  [ o-1 ].write( flit_tx_to_UP [i].read() );
				req_tx_to_DOWN   [ o-1 ][vc_out].write( req_tx_to_UP  [i][vc].read() );
				ack_tx_to_UP     [ i ][vc].write( ack_tx_to_DOWN[o-1][vc_out].read() );
				if( tmp.flit_type == FLIT_TYPE_TAIL ){
					reservation_table.release( o, vc_out);
					if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
						cout << getCurrentCycleNum() << ": VLink[" << _id<< "], release Down Output[" << o << "]"<< endl;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////   added by Amin Norollah  & taheri
void NoximVLink::Mesh(){
	for( int i = 0 ; i < NoximGlobalParams::mesh_dim_z - 1 ; i++ )
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
		//flit
		flit_rx_to_UP  [i].write( flit_rx_to_DOWN[i].read() );
		flit_tx_to_DOWN[i].write( flit_tx_to_UP  [i].read() );
		//ack
		ack_rx_to_DOWN [i][vc].write( ack_rx_to_UP   [i][vc].read() );
		ack_tx_to_UP   [i][vc].write( ack_tx_to_DOWN [i][vc].read() );
		//req
		req_rx_to_UP   [i][vc].write( req_rx_to_DOWN [i][vc].read() );
		req_tx_to_DOWN [i][vc].write( req_tx_to_UP   [i][vc].read() );
	}
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

int NoximVLink::routingFunction(const NoximRouteData & route_data){
	if (route_data.current_id > 15)
		cout << "s1" << endl;
	if (route_data.src_id> 15)
		cout << "s2" << endl;
	if (route_data.dst_id > 15)
		cout << "s3" << endl;
    NoximCoord position  = id2Coord(route_data.current_id);
    NoximCoord src_coord = id2Coord(route_data.src_id);
    NoximCoord dst_coord = id2Coord(route_data.dst_id);
	int routing          = route_data.routing ;  
	int DW_layer         = route_data.DW_layer;
	
    switch (NoximGlobalParams::routing_algorithm) {
	  
	case ROUTING_DOWNWARD:
    	return routingDownward(position, src_coord, dst_coord); 
		
	case ROUTING_DLADR:
		return routingTLAR( position, src_coord , dst_coord,routing, DW_layer);
	
	case ROUTING_DLAR:
		return routingTLAR( position, src_coord , dst_coord,routing, DW_layer);
	
	case ROUTING_DLDR:
		return routingTLAR( position, src_coord , dst_coord,routing, DW_layer);
    default:
		return dst_coord.z;
    }
}

int NoximVLink::routingDownward(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination){
	int down_level = NoximGlobalParams::down_level;	
	int layer;	//means which layer that packet should be transmitted

	if( (source.z + down_level)> NoximGlobalParams::mesh_dim_z-1)
		layer = NoximGlobalParams::mesh_dim_z-1;
	else 
		layer = source.z + down_level;

	if( (current.x == destination.x && current.y == destination.y) ){	//same (x,y), forward to up
		return destination.z;
	}
	else
		return layer;
}

int NoximVLink::routingTLAR(const NoximCoord& current, const NoximCoord& source, const NoximCoord& destination,const int select_routing, int dw_layer){
	if( (current.x == destination.x && current.y == destination.y) )return destination.z;
	
	switch ( NoximGlobalParams::dw_layer_sel ){
	case DW_BL      :return NoximGlobalParams::mesh_dim_z-1;break;
	case DW_ODWL    :return dw_layer;break;
	default         :return dw_layer;
	}
}
