/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the processing element
 *
 *
 * Edited by Amin Norollah @BALRUG and Danesh Derafshi(2017 May 25)
 *
 */
#include <math.h>
#include "NoximProcessingElement.h"
int NoximProcessingElement::randInt(int min, int max)
{
    return min +
	(int) ((double) (max - min + 1) * rand() / (RAND_MAX + 1.0));
}

/////////////////////////////
//////    RxProcess	   //////
//...........................
void NoximProcessingElement::rxProcess(){		
    if (reset.read()) {	// RESET
		start_from_vc = 0;
		start_from_message = 0;
		_round_MC = 0;
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
			ack_rx[vc].write(1);
			ack_semi_rx[vc].write(1);
			temp_p[vc].vc = 0;
			temp_p[vc].flit_left = 0;
		}
		_adaptive_transmit              =0;
		_dor_transmit                   =0;
		_dw_transmit                    =0;
		_mid_adaptive_transmit          =0;
		_mid_dor_transmit               =0;
		_mid_dw_transmit                =0;
		_beltway_transmit               =0;
		_Transient_adaptive_transmit    =0; 
		_Transient_dor_transmit         =0; 
		_Transient_dw_transmit          =0; 
		_Transient_mid_adaptive_transmit=0; 
		_Transient_mid_dor_transmit     =0;
		_Transient_mid_dw_transmit      =0;
		_Transient_beltway_transmit 	=0;	
		refly_pkt            = 0;
		while( !flit_queue.empty() ) 
			flit_queue.pop();
		/*DFS = 4;
  		RST = 0;*/
    }
	else{
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
			if (req_rx[vc].read() == 1 && ack_rx[vc].read() == 1) {
				NoximFlit flit_tmp = flit_rx.read();
				if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
					cout << sc_simulation_time() << ": ProcessingElement[" <<
						local_id << "] RECEIVING " << flit_tmp << " vc[" << vc << "]" << endl;
				}
				_flit_static(flit_tmp);//statics

				// semi
				if (req_semi_rx[vc].read() == 1 && ack_semi_rx[vc].read() == 1) {
					NoximFlit flit_tmp = flit_semi_rx.read();
					if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
						cout << sc_simulation_time() << ": ProcessingElement[" <<
							local_id << "] RECEIVING " << flit_tmp << " vc[0]" << endl;
					}
					_flit_static(flit_tmp);//statics
				}
			}

		for (int w = 0; w < NoximGlobalParams::num_vcs; w++) {
			ack_rx[w].write(1);
			ack_semi_rx[w].write(!_emergency);
		}
    }
}
/////////////////////////////
//////    TxProcess	   //////
//...........................
void NoximProcessingElement::txProcess() {	
    if (reset.read()){ // RESET
		start_from_vc = 0;
		start_from_vc_2 = 0;
		start_from_message = 0;
		wait = false;
		packet_queue_length = 0;
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
			req_tx[vc].write(0);
			req_semi_tx[vc].write(0);
			temp_p[vc].flit_left = 0;
			temp_p[vc].vc = 0;
			if (NoximGlobalParams::arch_with_credit == 1)
				credit[vc] = NoximGlobalParams::buffer_depth;
			else if (NoximGlobalParams::arch_with_credit == 0)
				credit[vc] = 1;
		}
		if (NoximGlobalParams::arch_with_credit == 1)
			credit_semi = NoximGlobalParams::buffer_depth;
		else if (NoximGlobalParams::arch_with_credit == 0)
			credit_semi = 1;
		reservation_table_pe.clear();
		while (!message_queue.empty())
			message_queue.pop();
		not_transmit      = 0;
		transmit          = 0;
		while (!packet_queue.empty())
			packet_queue.pop();

		transmittedAtPreviousCycle = false;
		_clean_all = false;

        num_pkt = 716800;/*
		DFS = 4;
        RST = 0;*/
    } 
	else {
		NoximPacket  packet;
		NoximFlit flit;
		//if(getCurrentCycleNum()%DFS < (4-RST)){	
		if (!_emergency && !_clean_all) {
			// if( !_clean_all ){
			if (canShot(packet)/* && packet_queue_length < DEFAULT_PACKET_QUEUE_LENGTH && packet.routing != -1*/) {
				// message_queue.push(packet);
			/*	NoximCoord packet_dst = id2Coord(packet.dst_id);
				NoximCoord packet_src = id2Coord(packet.src_id);
				if(throttling[packet_dst.x][packet_dst.y][packet_dst.z] == 0 && throttling[packet_src.x][packet_src.y][packet_src.z] == 0){
					packet_queue.push( packet );
					transmittedAtPreviousCycle = true;
					packet.timestamp_ni = getCurrentCycleNum();
					packet_queue_length++;
				}
				else*/
				//cout << "packet : " << packet.dst_id << endl;
				message_queue.push(packet);
				transmittedAtPreviousCycle = true;
			}
			else {
				transmittedAtPreviousCycle = false;
				//message_queue.push(packet);	
			}
		}

		// TODO : In clean stage, don't let message queue push into packetqueue
		if (packet_queue_length < DEFAULT_PACKET_QUEUE_LENGTH && !message_queue.empty() && !_clean_all) {
			//cout << "packet_queue_length " << vc << " : " << packet_queue_length << endl;
			packet = message_queue.front();
			//num_pkt--;
			if (TLA(packet)) {//The routing > 10 all have downward routing
				if (NoximGlobalParams::routing_algorithm > 10 || packet.routing != ROUTING_DOWNWARD_CROSS_LAYER) {
					NoximCoord packet_dst = id2Coord(packet.dst_id);
					NoximCoord packet_src = id2Coord(packet.src_id);
					if (throttling[packet_dst.x][packet_dst.y][packet_dst.z] == 0 && throttling[packet_src.x][packet_src.y][packet_src.z] == 0) {
						TAAR(packet);
						packet.timestamp_ni = getCurrentCycleNum();
						//cout<<"packet.timestamp_ni ="<<packet.timestamp_ni<<endl;
						packet_queue.push(packet);
						message_queue.pop();
						packet_queue_length++;
						_continue = false;
					}
				}
			}
		}
		start_from_message++;
		//////////////////////////////////////////////////
		//////			clean up all data			//////
		//................................................
		if (_clean_all) {
			if (!flit_queue.empty()) {
				flit = flit_queue.front();
				if (flit.flit_type == FLIT_TYPE_HEAD &&  refly_pkt > 0) {
					while (refly_pkt > 0 && NoximGlobalParams::message_level) {
						//while( refly_pkt > 0 ){
						if (flit.flit_type == FLIT_TYPE_TAIL)
							refly_pkt--;
						flit_queue.pop();
						if (flit_queue.empty())
							break;
						else
							flit = flit_queue.front();
					}
				}
			}
			while (!message_queue.empty()) {
				message_queue.pop();
			}
		}
		//////////////////////////////////////////////////////////////////////////////////
		//////			Reserve VCout for packets			Added by A.Norollah		//////
		//................................................................................
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
			if (temp_p[vc].flit_left == 0 && !packet_queue.empty()) {
				wait = false;
				for (int k = 0; k < NoximGlobalParams::num_vcs; k++) {
					int vc_output = (start_from_vc_2 + k) % NoximGlobalParams::num_vcs;
					if (reservation_table_pe.isAvailable(DIRECTION_LOCAL, vc_output) && !wait) {
						reservation_table_pe.reserve(DIRECTION_LOCAL, vc, DIRECTION_LOCAL, vc_output);
						temp_p[vc] = packet_queue.front();
						temp_p[vc].vc = vc_output;
						temp_p[vc].south = 0;
						temp_p[vc].east = 0;
						temp_p[vc].test_id = rand();
						packet_queue.pop();
						packet_queue_length--;
						wait = true;
					}
				}
			}
		start_from_vc_2++;
		//////////////////////////////////////////////////////////////////////////////////////
		//////			 Clear old signals && insert credits		Added by A.Norollah	//////
		//....................................................................................
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++) {
			req_tx[vc].write(0);
			if (ack_tx[vc].read() == 1) credit[vc]++;
		}
		//////////////////////////////////////////////////////////////////////////////////////
		//////			 Send										Added by A.Norollah	//////
		//....................................................................................
		wait = false;
		for (int j = 0; j < NoximGlobalParams::num_vcs; j++) {
			int vc_input = (start_from_vc + j) % NoximGlobalParams::num_vcs;
			int	vc_output = reservation_table_pe.getOutputVc(DIRECTION_LOCAL, vc_input);
			if (vc_output != NOT_RESERVED && temp_p[vc_input].flit_left != 0 && !wait) {
				if (credit[vc_output] > 0) {
					flit = nextFlit(vc_input);
					if (flit.dst_id != NOT_VALID) {
						if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
							cout << getCurrentCycleNum() << ": ProcessingElement[" << local_id <<
								"] SENDING " << flit << " in vc[" << vc_output << "]" << endl;
						}
						wait = true;
						flit_tx->write(flit);	// Send the generated flit
						req_tx[vc_output].write(1);
						credit[vc_output]--;

						if (flit.flit_type == FLIT_TYPE_HEAD)
							cnt_local++;

						else if (flit.flit_type == FLIT_TYPE_TAIL)
							reservation_table_pe.release(DIRECTION_LOCAL, vc_output);
						temp_p[vc_input].flit_left--;
					}
				}
			}
		}
		start_from_vc++;
		//////////////////////////////////////////////////
		//////			 Semi TxProcess				//////
		//................................................  
		// just use vc 0 -- need review
		req_semi_tx[0].write(0);
		if (ack_semi_tx[0].read() == 1)
			credit_semi++;
		if (!flit_queue.empty() && credit_semi>0) {
			if (refly_pkt > 0) {
				flit = flit_queue.front();
				NoximPacket p;
				assert(flit.mid_id < MAX_ID + 1);
				assert(flit.dst_id < MAX_ID + 1);
				p.src_id = flit.mid_id;
				p.dst_id = flit.dst_id;
				while (!TLA(p)) {
					if (flit.flit_type == FLIT_TYPE_HEAD) {
						NoximCoord flit_dst = id2Coord(flit.dst_id);
						if (throttling[flit_dst.x][flit_dst.y][flit_dst.z])cout << getCurrentCycleNum() << ":Packet drop.(dst)\t" << "refly pkt: " << refly_pkt << endl;
						else cout << getCurrentCycleNum() << ":" << flit << " drop." << endl;
					}
					flit_queue.pop();
					if (flit.flit_type == FLIT_TYPE_TAIL) refly_pkt--;
					if (refly_pkt == 0)break;
					flit = flit_queue.front();
					p.src_id = flit.mid_id;
					p.dst_id = flit.dst_id;
				}
			}
			if (refly_pkt > 0) {
				flit = flit_queue.front();
				NoximPacket p;
				assert(flit.mid_id < MAX_ID + 1);
				assert(flit.dst_id < MAX_ID + 1);
				p.src_id = flit.mid_id;
				p.dst_id = flit.dst_id;
				if (!TLA(p)) {
					cout << getCurrentCycleNum() << ":" << flit << endl;
					cout << "flit.current_id" << "=" << local_id << id2Coord(local_id) << endl;
					cout << "flit.src_id    " << "=" << flit.src_id << id2Coord(flit.src_id) << endl;
					cout << "flit.mid_id    " << "=" << flit.mid_id << id2Coord(flit.mid_id) << endl;
					cout << "flit.dst_id    " << "=" << flit.dst_id << id2Coord(flit.dst_id) << endl;
					cout << "flit.routing   " << "=" << flit.routing_f << endl;
					cout << "flit.DW_layer  " << "=" << flit.DW_layer << endl;
					cout << "flit.arr_mid   " << "=" << flit.arr_mid << endl;
					cout << "refly pkt: " << refly_pkt << endl;
					assert(0);
				}
				//
				flit.arr_mid = true;
				//
				if (flit.beltway && NoximGlobalParams::beltway && p.routing != ROUTING_DOWNWARD_CROSS_LAYER)
					flit.routing_f = ROUTING_WEST_FIRST; //20130923
				else
					flit.routing_f = p.routing;
				flit_semi_tx->write(flit);
				req_semi_tx->write(1);
				credit_semi--;
				//flit_queue.pop();
				if (flit.flit_type == FLIT_TYPE_TAIL) refly_pkt--;
				flit_queue.pop();
				if (NoximGlobalParams::verbose_mode > VERBOSE_OFF) {
					cout << getCurrentCycleNum() << ": ProcessingElement[" << local_id <<
						"] SENDING " << flit << " in semi_local" << endl;
				}
			}
		}
	}
}

NoximFlit NoximProcessingElement::nextFlit(int vc_in) {
    NoximFlit flit;
	//flit.vc		 = temp_p[vc_in].vc;
	//flit.east		 = temp_p[vc_in].east;
    //flit.south	 = temp_p[vc_in].south;
	flit.test_id	 = temp_p[vc_in].test_id;
    flit.src_id      = temp_p[vc_in].src_id;
    flit.dst_id      = temp_p[vc_in].dst_id;
	flit.mid_id      = temp_p[vc_in].mid_id;
    flit.timestamp   = temp_p[vc_in].timestamp;
	flit.timestamp_ni= temp_p[vc_in].timestamp_ni;
	flit.timestamp_nw= getCurrentCycleNum();
    flit.sequence_no = temp_p[vc_in].size - temp_p[vc_in].flit_left;
    flit.hop_no      = 0;
	flit.routing_f   = temp_p[vc_in].routing;
	flit.DW_layer    = temp_p[vc_in].DW_layer;
	flit.arr_mid     = temp_p[vc_in].arr_mid;
	flit.beltway     = temp_p[vc_in].beltway;
    //flit.payload   = DEFAULT_PAYLOAD;

    if (temp_p[vc_in].size == temp_p[vc_in].flit_left){
		flit.flit_type = FLIT_TYPE_HEAD;
		// taheri
		flit.east=0;
		flit.south=0;
		// end taheri
	    NoximCoord src_coord = id2Coord(flit.src_id);
	    NoximCoord dst_coord = id2Coord(flit.dst_id);

	    //if(src_coord.y>dst_coord.y) flit.vc=0; //taheri
	    // else flit.vc=1;

		//flit.vc = rand() % NoximGlobalParams::num_vcs; //taheri
		flit.vc = temp_p[vc_in].vc;
		flit.pre_routing = -1;
		// Added by A.Norollah
		//temp_p[vc_in].vc = flit.vc;
		//flit.test_id = rand()% 5000;
    }
    else if (temp_p[vc_in].flit_left == 1)
		flit.flit_type = FLIT_TYPE_TAIL;
    else
		flit.flit_type = FLIT_TYPE_BODY;
/*
    packet_queue.front().flit_left--;
    if (packet_queue.front().flit_left == 0){
	packet_queue.pop();
	packet_queue_length--;
	}*/
    return flit;
}

bool NoximProcessingElement::canShot(NoximPacket & packet) {
    bool shot;
    double threshold;

    if (NoximGlobalParams::traffic_distribution != TRAFFIC_TABLE_BASED){
		if (!transmittedAtPreviousCycle)
			threshold = NoximGlobalParams::packet_injection_rate;
		else
			threshold = NoximGlobalParams::probability_of_retransmission;
		
		shot = (((double) rand()) / RAND_MAX < threshold);
		if (shot) {
			switch (NoximGlobalParams::traffic_distribution) {
			case TRAFFIC_RANDOM:
			packet = trafficRandom();
			break;
	
			case TRAFFIC_TRANSPOSE1:
			packet = trafficTranspose1();
			break;
	
			case TRAFFIC_TRANSPOSE2:
			packet = trafficTranspose2();
			break;
	
			case TRAFFIC_BIT_REVERSAL:
			packet = trafficBitReversal();
			break;
	
			case TRAFFIC_SHUFFLE:
			packet = trafficShuffle();
			break;
	
			case TRAFFIC_BUTTERFLY:
			packet = trafficButterfly();
			break;
	
			default:
			assert(false);
			}
		}
    }
	else 
	{			// Table based communication traffic
		if (never_transmit)
			return false;

		int now = getCurrentCycleNum() ;
		bool use_pir = (transmittedAtPreviousCycle == false);
		vector < pair < int, double > > dst_prob;
		double threshold = traffic_table->getCumulativePirPor(local_id, now, use_pir, dst_prob);
		double prob = (double) rand() / RAND_MAX;
		shot = (prob < threshold);
		if (shot){
			for (unsigned int i = 0; i < dst_prob.size(); i++) {
				packet.make(local_id, dst_prob[i].first, now, getRandomSize());
				if ( prob < dst_prob[i].second && TLA(packet) ) {
					
					TAAR(packet);
					packet.timestamp = getCurrentCycleNum();
					packet.size = packet.flit_left = getRandomSize();
				break;
			}
			}
		}
    }
    return shot;
}


NoximPacket NoximProcessingElement::trafficRandom() {
	// cout<<"Enter Random"<<endl;
    int max_id = MAX_ID;//x*y*z - 1
	NoximPacket p;
    p.src_id = local_id;
    double rnd = rand() / (double) RAND_MAX;
    double range_start = 0.0;
    //cout << "\n " << getCurrentCycleNum() << " PE " << local_id << " rnd = " << rnd << endl;
	int re_transmit = 1; //1
    // Random destination distribution
    do {
		transmit++;
		p.dst_id = randInt(0, max_id);
		assert( p.dst_id < MAX_ID + 1 );
		// check for hotspot destination
		for (unsigned int i = 0; i < NoximGlobalParams::hotspots.size(); i++) {
	    //cout << getCurrentCycleNum() << " PE " << local_id << " Checking node " << NoximGlobalParams::hotspots[i].first << " with P = " << NoximGlobalParams::hotspots[i].second << endl;
	    if (rnd >= range_start && rnd <	range_start + NoximGlobalParams::hotspots[i].second) {
			if (local_id != NoximGlobalParams::hotspots[i].first) {
				//cout << getCurrentCycleNum() << " PE " << local_id <<" That is ! " << endl;
				p.dst_id = NoximGlobalParams::hotspots[i].first;
			}	
			break;
	    } 
		else
			range_start += NoximGlobalParams::hotspots[i].second;	// try next
		}
	if (p.dst_id == p.src_id)
		re_transmit = 1;
	else{
		re_transmit = !TLA(p);
		TAAR(p);
	}
	
	if(re_transmit)
		transmit--;

    } while ((p.dst_id == p.src_id) || re_transmit);
	
	assert( NoximGlobalParams::message_level || p.routing >= 0 );
	//assert( p.routing >= 0 );
	p.timestamp = getCurrentCycleNum();
	p.size = p.flit_left = getRandomSize();
	//taheri
    //if(rand()%2==0) p.vc=0;
   // else p.vc=1;
   // p.east=0;
    //p.south=0;
    return p;
}

NoximPacket NoximProcessingElement::trafficTranspose1() {
    NoximPacket p;
    p.src_id = local_id;
    NoximCoord src, dst;

    // Transpose 1 destination distribution
    src   = id2Coord(p.src_id);
	dst.x = NoximGlobalParams::mesh_dim_x - 1 - src.y;
    dst.y = NoximGlobalParams::mesh_dim_y - 1 - src.x;
    dst.z = NoximGlobalParams::mesh_dim_z - 1 - src.z;
	fixRanges(src, dst);
    p.dst_id = coord2Id(dst);

    p.timestamp = getCurrentCycleNum() ;
    p.size = p.flit_left = getRandomSize();
	/*** Cross Layer Solution***/
	bool tmp = !TLA(p);
	if(tmp && !NoximGlobalParams::message_level)
		p.dst_id = NOT_VALID;
	
	TAAR(p);
    return p;
}

NoximPacket NoximProcessingElement::trafficTranspose2()
{
    NoximPacket p;
    p.src_id = local_id;
    NoximCoord src, dst;

    // Transpose 2 destination distribution
    src   = id2Coord(p.src_id);
    dst.x = src.y;
    dst.y = src.x;
	dst.z = src.z;
    fixRanges(src, dst);
    p.dst_id = coord2Id(dst);

    p.timestamp = getCurrentCycleNum();
    p.size = p.flit_left = getRandomSize();
	/*** Cross Layer Solution***/
	bool tmp = !TLA(p);
	if(tmp && !NoximGlobalParams::message_level)
		p.dst_id = NOT_VALID;
	TAAR(p);
    return p;
}

void NoximProcessingElement::setBit(int &x, int w, int v)
{
    int mask = 1 << w;

    if (v == 1)
	x = x | mask;
    else if (v == 0)
	x = x & ~mask;
    else
	assert(false);
}

int NoximProcessingElement::getBit(int x, int w)
{
    return (x >> w) & 1;
}

inline double NoximProcessingElement::log2ceil(double x)
{
    return ceil(log(x) / log(2.0));
}

NoximPacket NoximProcessingElement::trafficBitReversal()
{
    int nbits = (int)log2ceil(
		(double)(NoximGlobalParams::mesh_dim_x *
                 NoximGlobalParams::mesh_dim_y *
				 NoximGlobalParams::mesh_dim_z  ));
    int dnode = 0;
    for (int i = 0; i < nbits; i++)
	setBit(dnode, i, getBit(local_id, nbits - i - 1));

    NoximPacket p;
    p.src_id = local_id;
    p.dst_id = dnode;

    p.timestamp = getCurrentCycleNum();
    p.size      = p.flit_left = getRandomSize();
	p.arr_mid = true;
	p.mid_id  = p.src_id;
    return p;
}

NoximPacket NoximProcessingElement::trafficShuffle()
{
    int nbits = (int)log2ceil(
		(double)(NoximGlobalParams::mesh_dim_x *
                 NoximGlobalParams::mesh_dim_y *
				 NoximGlobalParams::mesh_dim_z  ));
    int dnode = 0;
    for   (int i = 0; i < nbits - 1; i++)
	setBit(dnode, i + 1, getBit(local_id, i        ));
    setBit(dnode, 0    , getBit(local_id, nbits - 1));

    NoximPacket p;
    p.src_id = local_id;
    p.dst_id = dnode   ;

    p.timestamp = getCurrentCycleNum();
    p.size      = p.flit_left = getRandomSize();
	p.arr_mid = true;
	p.mid_id  = p.src_id;
	
    return p;
}

NoximPacket NoximProcessingElement::trafficButterfly()
{
    int nbits = (int)log2ceil(
		(double)(NoximGlobalParams::mesh_dim_x *
                 NoximGlobalParams::mesh_dim_y *
				 NoximGlobalParams::mesh_dim_z  ));
    int dnode = 0;
    for   (int i = 1; i < nbits - 1; i++)
	setBit(dnode, i        , getBit(local_id, i        ));
    setBit(dnode, 0        , getBit(local_id, nbits - 1));
    setBit(dnode, nbits - 1, getBit(local_id, 0        ));

    NoximPacket p;
    p.src_id = local_id;
    p.dst_id = dnode  ;

    p.timestamp = getCurrentCycleNum() ;
    p.size      = p.flit_left = getRandomSize();
	p.arr_mid = true;
	p.mid_id  = p.src_id;
    return p;
}

void NoximProcessingElement::fixRanges(const NoximCoord src,
				       NoximCoord & dst)
{
    // Fix ranges
    if (dst.x < 0)
		dst.x = 0;
    if (dst.y < 0)
		dst.y = 0;
	if (dst.z < 0)
		dst.z = 0;
    if (dst.x >= NoximGlobalParams::mesh_dim_x)
		dst.x = NoximGlobalParams::mesh_dim_x - 1;
    if (dst.y >= NoximGlobalParams::mesh_dim_y)
		dst.y = NoximGlobalParams::mesh_dim_y - 1;
	if (dst.z >= NoximGlobalParams::mesh_dim_z)
		dst.z = NoximGlobalParams::mesh_dim_z - 1;
}

int NoximProcessingElement::getRandomSize()
{
    return randInt(NoximGlobalParams::min_packet_size,
		   NoximGlobalParams::max_packet_size);
}

void NoximProcessingElement::TraffThrottlingProcess(){
    /*if( reset.read() ) _clean_all = false;
	else{
	    if(NoximGlobalParams::throt_type == THROT_VERTICAL){	
			if( getCurrentCycleNum()%(int)TEMP_REPORT_PERIOD > TEMP_REPORT_PERIOD - NoximGlobalParams::clean_stage_time )//clean-up stage
			     _clean_all = true ;
			else _clean_all = false;
		}
	}*//*
	NoximCoord local = id2Coord(local_id);
	if(MTTT[local.x][local.y][local.z] < 0.5 && MTTT[local.x][local.y][local.z] != 0){
        	if(RST<3)
                     RST++;
        }
        else{
                if(RST>0)
                     RST--;
        }*/
}

void NoximProcessingElement::IntoEmergency(){
	_emergency  = true;
	if(_emergency_level < NoximGlobalParams::buffer_depth-1)
		_emergency_level++;
}

void NoximProcessingElement::OutOfEmergency(){
	_emergency       = false;
	if(_emergency_level > 0)
		_emergency_level--;
}

void NoximProcessingElement::IntoCleanStage(){
	_clean_all       = true;
}
void NoximProcessingElement::OutOfCleanStage(){
	_clean_all       = false;
}

void NoximProcessingElement::RTM_set_var(int non_throt_layer, int non_beltway_layer, int RoC_col_max, int RoC_col_min, int RoC_row_max, int RoC_row_min){
    _non_throt_layer   = non_throt_layer  ;
    _non_beltway_layer = non_beltway_layer;
    _RoC_col_min       = RoC_col_min      ;
    _RoC_col_max       = RoC_col_max      ;
    _RoC_row_min       = RoC_row_min      ;
    _RoC_row_max       = RoC_row_max      ;
}

bool NoximProcessingElement::TLA( NoximPacket & packet )
{
 NoximCoord curr = id2Coord( packet.src_id );
 NoximCoord dest = id2Coord( packet.dst_id );

 int x_diff = dest.x - curr.x;
 int y_diff = dest.y - curr.y;
 int z_diff = dest.z - curr.z;

 int y_a,x_a,z_a;
 int a_x_search, a_y_search, a_z_search;
                    
	bool adaptive_fail         = false;
	bool xy_fail               = false;
	bool dw_fail_src,dw_fail_dest;
	int  routing               = ROUTING_XYZ;
	if( NoximGlobalParams::routing_algorithm == ROUTING_WEST_FIRST_DOWNWARD ){
		if( _non_throt_layer == 0 )
			packet.routing = ROUTING_WEST_FIRST;
		else
			packet.routing = ROUTING_WEST_FIRST_DOWNWARD;
		return true;
	}
	// cout<<"TLA start-A"<<endl;
    if(throttling[dest.x][dest.y][dest.z] == 0){//destination not throttle
	    //XY-Plane
	for( z_a = 0 ; z_a < abs(z_diff) + 1 ; z_a++ ){
		for( y_a = 0 ; y_a < abs(y_diff) + 1 ; y_a ++ )
		for( x_a = 0 ; x_a < abs(x_diff) + 1 ; x_a ++ ){
		    a_x_search = (x_diff > 0)?(curr.x + x_a):(curr.x - x_a);
			a_y_search = (y_diff > 0)?(curr.y + y_a):(curr.y - y_a);
			a_z_search = (z_diff>0)?(curr.z + z_a):(curr.z - z_a);
			adaptive_fail |= throttling[a_x_search][a_y_search][a_z_search];
		}
		//Z-direction
        //for( z_a = 1 ; z_a < abs(z_diff) + 1 ; z_a++ ){
			//a_z_search = (z_diff>0)?(curr.z + z_a):(curr.z - z_a);
			//adaptive_fail |= throttling[dest.x][dest.y][a_z_search];
		}
// cout<<"TLA start-XY"<<endl;
	 /***Into XY Routing***/
	if( adaptive_fail == true ){
        //X-direction
		for( x_a = 1 ; x_a < abs(x_diff) + 1 ; x_a++ ){
			a_x_search = (x_diff>0)?(curr.x + x_a):(curr.x - x_a);
			xy_fail |= throttling[a_x_search][curr.y][curr.z];
		}
		//Y-direction
        for( y_a = 1 ; y_a < abs(y_diff) + 1 ; y_a++ ){
			a_y_search = (y_diff > 0)?(curr.y + y_a):(curr.y - y_a);
			xy_fail |= throttling[dest.x][a_y_search][curr.z];
		}
		//Z-direction
        for( z_a=1;z_a<abs(z_diff)+1;z_a++){
			a_z_search = (z_diff>0)?(curr.z + z_a):(curr.z - z_a);
			xy_fail |= throttling[dest.x][dest.y][a_z_search]; 
		}
	}
	// cout<<"TLA start-DW"<<endl;
    /***Into Downward Routing***/
	if(xy_fail == true){
		dw_fail_src = false;
	    int z_diff_dw_s = (NoximGlobalParams::mesh_dim_z-1) - curr.z;
	    for( int zzt = 1 ; zzt < z_diff_dw_s + 1 ; zzt++ ){
			dw_fail_src |= throttling[curr.x][curr.y][curr.z + zzt];
		}
		dw_fail_dest = false;
		int z_diff_dw_d = (NoximGlobalParams::mesh_dim_z-1) - dest.z;
	    for( int zzt = 1 ; zzt < z_diff_dw_d + 1 ; zzt++ ){
			dw_fail_dest |= throttling[dest.x][dest.y][dest.z + zzt];
		}
	}
	// cout<<"TLA start-ASSIGN"<<endl;
	//adaptive_fail = false;
	if     ( adaptive_fail                 == false )routing = ROUTING_WEST_FIRST;
	else if(      xy_fail                  == false )routing = ROUTING_XYZ;
	else if( (dw_fail_dest || dw_fail_src) == false )routing = ROUTING_DOWNWARD_CROSS_LAYER;
	else {  routing = INVALID_ROUTING;
			//cout<<"fuck routing function all fail"<<endl;
			return false;
	}
	
	packet.routing   = routing;
	//packet.timestamp = getCurrentCycleNum() ;
    //packet.size      = packet.flit_left = getRandomSize();
	if ( NoximGlobalParams::mesh_dim_z - _non_throt_layer != 0)
		packet.DW_layer  = _non_throt_layer + rand() % ( NoximGlobalParams::mesh_dim_z - _non_throt_layer);
	else
		packet.DW_layer  = -1;
    // return TAAR(packet);
	return true;
	}
	else{//destination throttled
		not_transmit++;
		packet.routing = INVALID_ROUTING;
		//cout<<"fuck! dst throttle"<<endl;
		return false;
	}	
}

bool NoximProcessingElement::TAAR( NoximPacket & p ){
	//Routing mode and cascaded node decision
	if ( NoximGlobalParams::beltway )//&& ( _non_beltway_layer > id2Coord(local_id).z) )
		Beltway(p);
	else if ( p.routing == ROUTING_DOWNWARD_CROSS_LAYER && NoximGlobalParams::cascade_node ){
		assert( p.src_id < MAX_ID + 1);
		assert( p.dst_id < MAX_ID + 1);
		if( NoximGlobalParams::Mcascade )
			p.mid_id    = sel_int_node_Mcascade(p.src_id,p.dst_id);
		else
			p.mid_id    = sel_int_node(p.src_id,p.dst_id);
			
		if(p.mid_id == p.src_id){//if the packet needs to downward directly
			p.arr_mid = true;
		}
		else {
			p.arr_mid   = false;
			p.routing   = ROUTING_WEST_FIRST;
		}
	}
	else{
		p.arr_mid = true;
		p.mid_id  = p.src_id;
	}
	if ( p.routing  > 19 ){
		cout<<p.routing<<endl;
		assert(false);
	}
	return true;
}

bool NoximProcessingElement::Beltway( NoximPacket & p ){
	
	int routing;

	p.mid_id = sel_int_node_belt(p.src_id,p.dst_id,p.beltway,routing);
	
	if( (id2Coord(p.mid_id).x == id2Coord(p.dst_id).x) && (id2Coord(p.mid_id).y == id2Coord(p.dst_id).y) ){//mid = dst, packet send directly
		p.arr_mid = true;
		p.mid_id  = p.src_id;
		p.routing = ROUTING_WEST_FIRST;//ROUTING_ODD_EVEN_3D;
		//cout<<"fuck~ mid node errer1"<<endl;
	}
	else if ( (id2Coord(p.mid_id).x == id2Coord(p.src_id).x) && (id2Coord(p.mid_id).y == id2Coord(p.src_id).y) ){//mid = src, Needs downward
		p.arr_mid = true;
		if(p.beltway)
			p.routing = ROUTING_WEST_FIRST;
		else
			p.routing = ROUTING_DOWNWARD_CROSS_LAYER;
		//cout<<"fuck~ mid node errer2"<<endl;
	}
	else{
		p.arr_mid = false;
		if( p.beltway ){
			p.routing = ROUTING_WEST_FIRST;//ROUTING_ODD_EVEN_3D;
		}
		else{
			p.routing = ROUTING_WEST_FIRST;//ROUTING_ODD_EVEN_3D;
		}
		//cout<<"fuck~ mid node errer3"<<endl;
	}
	return true;
}

/*Select the intemedium node (Jimmy modified on 2011.07.01)*/
int NoximProcessingElement::sel_int_node(int source, int  destination) 
{
	
	assert(source      < MAX_ID + 1 );
	assert(destination < MAX_ID + 1 );
	NoximCoord s = id2Coord(source     ); //transfer source id to coordination
	NoximCoord d = id2Coord(destination); //transfer destination id to coordination
	NoximCoord int_node; //intermedium node
	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
		cout<<getCurrentCycleNum()<<":sel_int_node-"<<s<<","<<d<<endl;
	int lateral_bound; //find the lateral searching bound
	int vertical_bound; //find the vertical searching bound
	float sel_candidate; //the area of intermedium node region
	float candidate_tmp;
	
	if((d.x > s.x) & (d.y > s.y))
	{
		if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
		cout<<"In case d.x > s.x & d.y > s.y"<<endl;
		vertical_bound = d.y;
		lateral_bound = d.x;
		sel_candidate = 0;
		for(int x=s.x; x<=d.x; x++)
		{
			if(throttling[x][s.y][s.z]==1)
			{
				lateral_bound = x - 1;
				break;
			}
			for(int y=s.y; y<=vertical_bound; y++)
			{
				if(throttling[x][y][s.z]==1 || y==vertical_bound)
				{
					candidate_tmp = (y==vertical_bound && !throttling[x][y][s.z]) ? (x - s.x +1) * (y - s.y + 1) : (x - s.x +1) * ((y-1) - s.y + 1);
					if(candidate_tmp >= sel_candidate)
					{
						vertical_bound = (y == s.y) ? s.y : (y == vertical_bound && !throttling[x][y][s.z]) ? y : (y-1);
						lateral_bound = (x == s.x) ? s.x :  x; //always is x-1, because souce node cannot be throttled in this phase
						sel_candidate = candidate_tmp;
					}
					break;
				}
			}
		}//end outer-for
		int_node.x = lateral_bound;
		int_node.y = vertical_bound;
		int_node.z = s.z;

		//if(int_node.x == d.x) int_node.y = d.y; //XY routable
		return coord2Id(int_node);
	}
	else if((d.x > s.x) & (d.y <= s.y))
	{
		if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
		cout<<"In case d.x > s.x & d.y < s.y"<<endl;
		vertical_bound = d.y;
		lateral_bound = d.x;
		sel_candidate = 0;
		for(int x=s.x; x<=d.x; x++)
		{
			if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
			cout<<"In for loop of x="<<x<<endl;
			if(throttling[x][s.y][s.z]==1){
				lateral_bound = x - 1;
				break;
			}
			for(int y=s.y; y>=vertical_bound; y--)
			{
				if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
				cout<<"In for loop of y="<<y<<endl;
				if(throttling[x][y][s.z]==1 || y==vertical_bound)
				{
					candidate_tmp = (y == vertical_bound && !throttling[x][y][s.z]) ? (x - s.x +1) * (s.y - y + 1) * MTTT[s.x][s.y][s.z] : (x - s.x +1) * (s.y - (y+1) + 1) * MTTT[s.x][s.y][s.z] ;
					if(candidate_tmp >=sel_candidate)
					{
						vertical_bound = (y == s.y) ? s.y : (y == vertical_bound && !throttling[x][y][s.z]) ? y : (y+1);
						lateral_bound  = (x == s.x) ? s.x :  x; //always is x-1, because souce node cannot be throttled in this phase
						sel_candidate  = candidate_tmp;
					}
					break;
				}
			}
		}//end outer-for
		int_node.x = lateral_bound;
		int_node.y = vertical_bound;
		int_node.z = s.z;

		//if(int_node.x == d.x) int_node.y = d.y; //XY routable
		return coord2Id(int_node);
	}
	else if((d.x <= s.x) & (d.y > s.y))
	{	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
		cout<<"In case d.x < s.x & d.y > s.y"<<endl;
		vertical_bound = d.y;
		lateral_bound = d.x;
		sel_candidate = 0;
		for(int x=s.x; x>=d.x; x--)
		{
			if(throttling[x][s.y][s.z]==1)
			{
				lateral_bound = x + 1;
				break;
			}
			for(int y=s.y; y<=vertical_bound; y++)
			{
				if(throttling[x][y][s.z]==1 || y==vertical_bound)
				{
					candidate_tmp = (y==vertical_bound && !throttling[x][y][s.z]) ? (s.x - x +1) * (y -s.y + 1) * MTTT[s.x][s.y][s.z] : (s.x - x +1) * ((y-1) -s. y + 1) * MTTT[s.x][s.y][s.z] ;
					if(candidate_tmp >= sel_candidate)
					{
						vertical_bound = (y == s.y) ? s.y : (y == vertical_bound && !throttling[x][y][s.z]) ? y : (y-1);
						lateral_bound = (x == s.x) ? s.x : x; //always is x+1, because souce node cannot be throttled in this phase
						sel_candidate = candidate_tmp;
					}
					break;
				}
			}
		}//end outer-for
		int_node.x = lateral_bound;
		int_node.y = vertical_bound;
		int_node.z = s.z;

		//if(int_node.x == d.x) int_node.y = d.y; //XY routable
		return coord2Id(int_node);
	}
	else if((d.x <= s.x) & (d.y <= s.y))
	{	if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
		cout<<"In case d.x < s.x & d.y < s.y"<<endl;
		vertical_bound = d.y;
		lateral_bound = d.x;
		sel_candidate = 0;
		for(int x=s.x; x>=d.x; x--)
		{
			if(throttling[x][s.y][s.z]==1)
			{
				lateral_bound = x + 1;
				break;
			}
			for(int y=s.y; y>=vertical_bound; y--)
			{
				if(throttling[x][y][s.z]==1 || y==vertical_bound)
				{
					candidate_tmp = (y==vertical_bound && !throttling[x][y][s.z]) ? (s.x - x +1) * (s.y - y + 1) * MTTT[s.x][s.y][s.z] : (s.x - x +1) * (s.y - (y+1) + 1) * MTTT[s.x][s.y][s.z] ;
					if(candidate_tmp >= sel_candidate)
					{
						vertical_bound = (y == s.y) ? s.y : (y == vertical_bound && !throttling[x][y][s.z]) ? y : (y+1);
						lateral_bound = (x == s.x) ? s.x : x; //always is x+1, because souce node cannot be throttled in this phase
						sel_candidate = candidate_tmp;
					}
					break;
				}
			}
		}//end outer-for
		int_node.x = lateral_bound;
		int_node.y = vertical_bound;
		int_node.z = s.z;

		//if(int_node.x == d.x) int_node.y = d.y; //XY routable
		return coord2Id(int_node);
	}
	else{
		if (NoximGlobalParams::verbose_mode > VERBOSE_LOW ) 
		cout<<"Mid node return destination"<<endl;
		return destination; //if(d.x == s.x & d.y > s.y),  if(d.x == s.x & d.y < s.y),  if(d.x > s.x & d.y == s.y),  if(d.x < s.x & d.y == s.y)
	}
}

int NoximProcessingElement::sel_int_node_belt(int source, int  destination, bool &beltway, int &routing){
	NoximCoord s = id2Coord(source     ); //transfer source id to coordination
	NoximCoord d = id2Coord(destination); //transfer destination id to coordination
	NoximCoord m =(NoximGlobalParams::cascade_node)? id2Coord(sel_int_node(source,destination)):id2Coord(destination); //intermedium node  3DWF
	int ring_level,mid,i;
	int min_hop = 2*(abs(s.x - d.x) + abs(s.y - d.y));
	int beltway_hop;
	bool beltway_decision;
	if(NoximGlobalParams::br_sel == SEL_RANDOM)
		beltway_decision = _beltway_RND(s,d);
	else if(NoximGlobalParams::br_sel == SEL_BUFFER_LEVEL)
		beltway_decision = _beltway_OBL(s,d);
	else if(NoximGlobalParams::br_sel == SEL_NOP)
		beltway_decision = _beltway_RCA(s,d);
	else if(NoximGlobalParams::br_sel == SEL_RCA)
		beltway_decision = _beltway_RCA(s,d);
	else if(NoximGlobalParams::br_sel == SEL_THERMAL)  // Derek 2012.12.21
		beltway_decision = _beltway_THERMAL(s,d);			
	else if(NoximGlobalParams::br_sel == SEL_OBLIVIOUS){
		float rnd = (float) rand();
		beltway_decision = inROC(s,d) && (( rnd  / RAND_MAX) < NoximGlobalParams::beltway_ratio );
		}
	else
		assert(false);
		
	if( NoximGlobalParams::Mbeltway )
		ring_level = inRing(d);
	else
		ring_level = 0;
		
	if( s.x > d.x && beltway_decision ){
		if( d.y >  NoximGlobalParams::mesh_dim_y/2 ){
			mid = xyz2Id( NoximGlobalParams::mesh_dim_x - 1 - ring_level, NoximGlobalParams::mesh_dim_y - 1 - ring_level, d.z );
			NoximPacket p( source, mid, 0, 2);
			beltway_hop = NoximGlobalParams::mesh_dim_x - 1 - s.x + NoximGlobalParams::mesh_dim_y - 1 - s.y + NoximGlobalParams::mesh_dim_x - 1 - d.x + NoximGlobalParams::mesh_dim_y - 1 - d.y;
			if( TLA(p) && ( p.routing == ROUTING_WEST_FIRST || p.routing == ROUTING_XYZ ) && ( beltway_hop < min_hop )){
				beltway = true;
				routing = p.routing;
				return mid;
				}
			//else
			//	cout<<"fuck 1! sel_int_node_belt"<<endl;
		}
		else{
			mid = xyz2Id( NoximGlobalParams::mesh_dim_x - 1 - ring_level, ring_level, d.z );
			NoximPacket p( source, mid, 0, 2);
			beltway_hop = NoximGlobalParams::mesh_dim_x - 1 - s.x + s.y + NoximGlobalParams::mesh_dim_x - 1 - d.x + d.y;
			if( TLA(p) && ( p.routing ==  ROUTING_WEST_FIRST || p.routing == ROUTING_XYZ ) && ( beltway_hop < min_hop )){
				beltway = true;
				routing = p.routing;
				return mid;
				}
			//else
			//	cout<<"fuck 2! sel_int_node_belt"<<endl;
		}
	}
	 if ( s.x < d.x && beltway_decision ){
		if( d.y > NoximGlobalParams::mesh_dim_y/2 ){
			mid = xyz2Id( ring_level, NoximGlobalParams::mesh_dim_y - 1 - ring_level, d.z );
			NoximPacket p( source, mid, 0, 2);
			beltway_hop = s.x + NoximGlobalParams::mesh_dim_y - 1 - s.y + d.x + NoximGlobalParams::mesh_dim_y - 1 - d.y;
			if( TLA(p) && ( p.routing == ROUTING_WEST_FIRST || p.routing == ROUTING_XYZ ) && ( beltway_hop < min_hop )){
				beltway = true;
				routing = p.routing;
				return mid;
				}
			//else
			//	cout<<"fuck 3! sel_int_node_belt"<<endl;
		}
		else{
			mid = xyz2Id( ring_level, ring_level, d.z );
			NoximPacket p( source, mid, 0, 2);
			beltway_hop = s.x + s.y + d.x + d.y;
			if( TLA(p) && ( p.routing == ROUTING_WEST_FIRST || p.routing == ROUTING_XYZ ) && ( beltway_hop < min_hop )){
				beltway = true;
				routing = p.routing;
				return mid;
				}
			//else
			//	cout<<"fuck 4! sel_int_node_belt"<<endl;
		}
	}
	return coord2Id(m);
}
int NoximProcessingElement::sel_int_node_Mcascade(int source, int  destination) {
	
	assert(source      < MAX_ID + 1 );
	assert(destination < MAX_ID + 1 );
	NoximCoord s = id2Coord(source     ); //transfer source id to coordination
	NoximCoord d = id2Coord(destination); //transfer destination id to coordination
	NoximCoord m = id2Coord(sel_int_node(source,destination)); //intermediate node
	if( m == s || m == d )
		return coord2Id(m);
	int i;
	
	if( m.x != s.x){
		if      ( abs(m.x - s.x) > 2 && abs(m.y - s.y) > 2 && NoximGlobalParams::Mcascade_step > 2){
			if( ( _round_MC % 4 ) == 0 ){
				if      ( m.x > s.x ) i = m.x - 3;
				else if ( m.x < s.x ) i = m.x + 3;
				else                  i = m.x    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_x) )
					m.x = i;
				}
			else if( ( _round_MC % 4 ) == 1 ){
				if      ( m.y > s.y ) i = m.y - 3;
				else if ( m.y < s.y ) i = m.y + 3;
				else                  i = m.y    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_y) )
					m.y = i;
			}
			else if ( ( _round_MC % 4 ) == 2 ){
				if      ( m.y > s.y ) i = m.y - 1;
				else if ( m.y < s.y ) i = m.y + 1;
				else                  i = m.y    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_y) )
					m.y = i;
				if      ( m.x > s.x ) i = m.x - 2;
				else if ( m.x < s.x ) i = m.x + 2;
				else                  i = m.x    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_x) )
					m.x = i;
			}
			else {
				if      ( m.y > s.y ) i = m.y - 2;
				else if ( m.y < s.y ) i = m.y + 2;
				else                  i = m.y    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_y) )
					m.y = i;
				if      ( m.x > s.x ) i = m.x - 1;
				else if ( m.x < s.x ) i = m.x + 1;
				else                  i = m.x    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_x) )
					m.x = i;
			}
		}
		else if      ( abs(m.x - s.x) > 1 && abs(m.y - s.y) > 1 && NoximGlobalParams::Mcascade_step > 1){
			if( ( _round_MC % 3 ) == 0 ){
				if      ( m.x > s.x ) i = m.x - 2;
				else if ( m.x < s.x ) i = m.x + 2;
				else                  i = m.x    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_x) )
					m.x = i;
				}
			else if( ( _round_MC % 3 ) == 1 ){
				if      ( m.y > s.y ) i = m.y - 2;
				else if ( m.y < s.y ) i = m.y + 2;
				else                  i = m.y    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_y) )
					m.y = i;
			}
			else{
				if      ( m.y > s.y ) i = m.y - 1;
				else if ( m.y < s.y ) i = m.y + 1;
				else                  i = m.y    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_y) )
					m.y = i;
				if      ( m.x > s.x ) i = m.x - 1;
				else if ( m.x < s.x ) i = m.x + 1;
				else                  i = m.x    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_x) )
					m.x = i;
			}
		}
		else if ( abs(m.x - s.x) > 0 && abs(m.y - s.y) > 0  && NoximGlobalParams::Mcascade_step > 0){
			// if( ( rand() % 2 ) == 0 ){
			if( ( _round_MC % 2 ) == 0 ){
				if      ( m.x > s.x ) i = m.x - 1;
				else if ( m.x < s.x ) i = m.x + 1;
				else                  i = m.x    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_x) )
					m.x = i;
			}
			else{
				if      ( m.y > s.y ) i = m.y - 1;
				else if ( m.y < s.y ) i = m.y + 1;
				else                  i = m.y    ;
				if( (i > -1) && ( i < NoximGlobalParams::mesh_dim_y) )
					m.y = i;
			}
		}
		else if ( NoximGlobalParams::Mcascade_step == 0){
			return coord2Id(m);
			
		}
	}
	_round_MC++;
	return coord2Id(m);
}

bool NoximProcessingElement::inROC(NoximCoord s,NoximCoord d){
	/*if(  ((d.x < _RoC_col_min) && ( s.x < _RoC_col_min)) ||
	     ((d.x > _RoC_col_max) && ( s.x > _RoC_col_max)) ||
		 ((d.x < _RoC_col_min) && ( s.x < _RoC_col_min)) ||
		 ((d.y > _RoC_row_max) && ( s.x > _RoC_row_max)) 	)
		return true;
	else if( inROC_S(d) )
    	return true;
	else
		return false;*/
	return inROC_S(d);
	// return false;
}

bool NoximProcessingElement::inROC_S(NoximCoord s){
	if( s.x >= _RoC_col_min && s.x <= _RoC_col_max && 
	    s.y >= _RoC_row_min && s.y <= _RoC_row_max )
	   	return false;
	else
		return true;
}

int NoximProcessingElement::inRing(NoximCoord dest){//Find the ring level of destination
	//Find Min. of a,b,c,d
	if( NoximGlobalParams::Mbeltway ){
		return (int)( rand() % NoximGlobalParams::Sbeltway_ring );
	}
	else if( NoximGlobalParams::Sbeltway && NoximGlobalParams::Sbeltway_ring > -1 ){
		return NoximGlobalParams::Sbeltway_ring;
	}
	else if( NoximGlobalParams::Sbeltway && NoximGlobalParams::Sbeltway_ring == -1){
		int a = _RoC_col_min;
		int b = NoximGlobalParams::mesh_dim_x - _RoC_col_max;
		int c = _RoC_row_min;
		int d = NoximGlobalParams::mesh_dim_y - _RoC_row_max;
		int e = ( a < b )?a:b;
		int f = ( c < d )?c:d;
		int g = ( e < f )?e:f;
		g--;
		if ( g < 0 ) g = 0;
		return g;
	}
	else{
	int a = dest.x;
	int b = NoximGlobalParams::mesh_dim_x - dest.x;
	int c = dest.y;
	int d = NoximGlobalParams::mesh_dim_y - dest.y;
	int e = ( a < b )?a:b;
	int f = ( c < d )?c:d;
	int g = ( e < f )?e:f;
	return g;
	}
	
}

bool NoximProcessingElement::_beltway_RND(NoximCoord s,NoximCoord d){
	vector< int >minimal_path;
	int beltway_path;
	if( d.x > s.x ){
		minimal_path.push_back(DIRECTION_EAST);
		minimal_path.push_back(DIRECTION_WEST);
		beltway_path = DIRECTION_WEST;
	}
	else if( d.x < s.x ){
		minimal_path.push_back(DIRECTION_WEST);
		minimal_path.push_back(DIRECTION_EAST);
		beltway_path = DIRECTION_EAST;
	}
	else //not use beltway
		return false;
	if( d.y > s.y )
		minimal_path.push_back(DIRECTION_SOUTH);
	else if(d.y < s.y)
		minimal_path.push_back(DIRECTION_NORTH);
	if ( beltway_path == minimal_path[rand() % minimal_path.size()])
		return true;
	else
		return false;
}

bool NoximProcessingElement::_beltway_OBL(NoximCoord s,NoximCoord d){
	vector< int >minimal_path;
	int beltway_path;
	if( d.x > s.x + 3 && s.x > 0){  //20130923
		minimal_path.push_back(DIRECTION_EAST);
		beltway_path = DIRECTION_WEST;
	}
	else if( d.x < s.x - 3 && s.x < 7){ //20130923
		minimal_path.push_back(DIRECTION_WEST);
		beltway_path = DIRECTION_EAST;
	}
	else //not use beltway
		return false;
	if( d.y > s.y)
		minimal_path.push_back(DIRECTION_SOUTH);
	else if(d.y < s.y)
		minimal_path.push_back(DIRECTION_NORTH);

	for(unsigned int i = 0 ; i < minimal_path.size() ; i++){
		if( free_slots_router[minimal_path[i]] > free_slots_router[beltway_path])
			return false;
	}
	return true;
}

bool NoximProcessingElement::_beltway_RCA(NoximCoord s,NoximCoord d){
	int minimal_path;
	int beltway_path;
	if( d.x > s.x ){//EAST
		if( d.y > s.y ){//SOUTH
			minimal_path = (RCA_router[2*DIRECTION_EAST+1] + RCA_router[DIRECTION_SOUTH*2+0])/2;//SE
			beltway_path = RCA_router[2*DIRECTION_WEST+0];//SW
		}
		else{//NORTH
			minimal_path = (RCA_router[2*DIRECTION_EAST+0] + RCA_router[DIRECTION_NORTH*2+1])/2;//NE
			beltway_path = RCA_router[2*DIRECTION_WEST+1];//WN
		}
	}
	else if( d.x < s.x ){//WEST
		if( d.x > s.y){//SOUTH
			minimal_path = (RCA_router[2*DIRECTION_WEST+0] + RCA_router[DIRECTION_SOUTH*2+1])/2;//WS
			beltway_path = RCA_router[2*DIRECTION_EAST+1];//ES
		}
		else{//NORTH
			minimal_path =(RCA_router[2*DIRECTION_WEST+1] + RCA_router[DIRECTION_SOUTH*2+0])/2;//WSpush_back(2*DIRECTION_WEST+1);
			beltway_path = RCA_router[2*DIRECTION_EAST+0];//EN 
		}
	}
	else //not use beltway
		return false;
	if( minimal_path >= beltway_path )
		return false;
	else
		return true;
}

bool NoximProcessingElement::_beltway_NoP(NoximCoord s,NoximCoord d){
	vector< int >minimal_path;
	vector< int >beltway_path;
	if( d.x > s.x ){
		minimal_path.push_back(DIRECTION_EAST);
		beltway_path.push_back(DIRECTION_WEST);
	}
	else if( d.x < s.x ){
		minimal_path.push_back(DIRECTION_WEST);
		beltway_path.push_back(DIRECTION_EAST);
	}
	else //not use beltway
		return false;
	if( d.y > s.y )
		minimal_path.push_back(DIRECTION_SOUTH);
	else if(d.y < s.y)
		minimal_path.push_back(DIRECTION_NORTH);
		
	int beltway_score = _NoPScore(NoP_router[beltway_path[0]],beltway_path);
	int minimal_score;
	
	if(minimal_path.size()==1){
		minimal_score = _NoPScore(NoP_router[minimal_path[0]],minimal_path);
		if(minimal_score >= beltway_score)
			return false;
		else
			return true;
	}else{
		for(unsigned int i = 0 ; i < minimal_path.size() ; i++){
			minimal_score = _NoPScore(NoP_router[minimal_path[i]],minimal_path)/2;			
			if(minimal_score >= beltway_score)
				return false;
			else
				return true;
	
		}
	}
}

bool NoximProcessingElement::_beltway_THERMAL(NoximCoord s,NoximCoord d){

	vector< int >minimal_path;
	double minimal_budget = 0;
	int beltway_path;
	double beltway_budget;
	//return false;
	
	if( d.x > s.x + 3 ){
		minimal_path.push_back(DIRECTION_EAST);
		minimal_budget = minimal_budget + MTTT[s.x+1][s.y][s.z]; // accumulative way		
		beltway_path = DIRECTION_WEST;
		if(s.x > 0)
			beltway_budget = MTTT[s.x-1][s.y][s.z];
		else
			beltway_budget = 0;
	}
	else if( d.x < s.x - 3 ){
		minimal_path.push_back(DIRECTION_WEST);
		minimal_budget = minimal_budget + MTTT[s.x-1][s.y][s.z];		
		beltway_path = DIRECTION_EAST;
		if(s.x < 7)
			beltway_budget = MTTT[s.x+1][s.y][s.z];
		else
			beltway_budget = 0;
	}
	else
		return false;
		
	if( d.y > s.y){
		minimal_path.push_back(DIRECTION_SOUTH);
		minimal_budget = minimal_budget + MTTT[s.x][s.y+1][s.z];	
	}	
	else if(d.y < s.y){
		minimal_path.push_back(DIRECTION_NORTH);
		minimal_budget = minimal_budget + MTTT[s.x][s.y-1][s.z];	
	}
	else
		return false;
	// budget normalization
	double beltway_budget_normal = fabs(beltway_budget)/(fabs(beltway_budget) + fabs(minimal_budget));//fabs(beltway_budget)/(fabs(beltway_budget) + fabs(minimal_budget));
	double rand_num = (double)rand()/(double)RAND_MAX;
	//if(_beltway_RCA(s, d)){
		if(beltway_budget <0)
			return false;
		else if(minimal_budget <0)
			return true;
		else if(beltway_budget==0 && minimal_budget==0)
			return _beltway_RCA(s, d);
		else{
			if( rand_num < beltway_budget_normal){
				return true;
			}
			else{ 
				return false;
			}
		}
	//}
	//else
	//	return false;
	
}

int NoximProcessingElement::_NoPScore(const NoximNoP_data & nop_data,
			  const vector < int >&nop_channels)
{
    int score = 0;
	
    for (unsigned int i = 0; i < nop_channels.size(); i++) {
		int available;
		if (nop_data.channel_status_neighbor[nop_channels[i]].available)available = 1;
		else			available = 0;
		int free_slots = nop_data.channel_status_neighbor[nop_channels[i]].free_slots;
		score += (int) available*free_slots; //traffic-&throttling-aware
    }

    return score;
}

void NoximProcessingElement::_flit_static(NoximFlit flit_tmp){
	_Transient_total_transmit++;
	_total_transmit++;
	if( flit_tmp.src_id == flit_tmp.mid_id ){//not cascade flit
		if     (flit_tmp.routing_f ==  ROUTING_WEST_FIRST  || flit_tmp.routing_f == ROUTING_ODD_EVEN_3D        ){
			_adaptive_transmit++;
			_Transient_adaptive_transmit++;
		}
		else if(flit_tmp.routing_f == ROUTING_XYZ                 ){
			_dor_transmit       ++;	
			_Transient_dor_transmit++;	
		}
		else if(flit_tmp.routing_f == ROUTING_DOWNWARD_CROSS_LAYER){
			_dw_transmit        ++;	
			_Transient_dw_transmit++;
		}
		if( flit_tmp.beltway == true){
                        _Transient_beltway_transmit++;
                        _beltway_transmit++;
                }
	}
	else{//cascade flit
		if( flit_tmp.beltway == true){
			_Transient_beltway_transmit++;
			_beltway_transmit++;
		}
		else if(flit_tmp.routing_f == ROUTING_WEST_FIRST  || flit_tmp.routing_f == ROUTING_ODD_EVEN_3D        ){
			_mid_adaptive_transmit++;
			_Transient_mid_adaptive_transmit++;
		}
		else if(flit_tmp.routing_f == ROUTING_XYZ                 ){
			_mid_dor_transmit     ++;
			_Transient_mid_dor_transmit     ++;
		}
		else if(flit_tmp.routing_f == ROUTING_DOWNWARD_CROSS_LAYER){
			_mid_dw_transmit++;	
			_Transient_mid_dw_transmit++;
		}
	}
}

void NoximProcessingElement::CalcDelay(){
	_packet_queue_num   = 0;
	_packet_queue_msg_delay = 0;
	_packet_queue_ni_delay  = 0;

		while (!packet_queue.empty()) {
			NoximPacket packet = packet_queue.front();
			if (packet.flit_left == packet.size) {//only calc packets
				_packet_queue_num++;
				_packet_queue_msg_delay += (int)getCurrentCycleNum() - (int)packet.timestamp;
				_packet_queue_ni_delay	+= (int)getCurrentCycleNum() - (int)packet.timestamp_ni;
			}
			packet_queue.pop();
		}
}

void NoximProcessingElement::CalcMessageDelay(){
	_message_queue_num   = 0;
	_message_queue_delay = 0;
	while (!message_queue.empty()) {
		NoximPacket packet = message_queue.front();
		_message_queue_num++;
		_message_queue_delay += ((int)getCurrentCycleNum() - (int)packet.timestamp);
		message_queue.pop();
	}
}

void NoximProcessingElement::ResetTransient_Transmit(){
	_Transient_adaptive_transmit    =0;
	_Transient_dor_transmit         =0;
	_Transient_dw_transmit          =0;
	_Transient_mid_adaptive_transmit=0;
	_Transient_mid_dor_transmit     =0;
	_Transient_mid_dw_transmit      =0;
	_Transient_beltway_transmit     =0;
}
