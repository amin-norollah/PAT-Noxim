/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file represents the top-level testbench
 */

#ifndef __NOXIMNOC_H__
#define __NOXIMNOC_H__

#include <systemc.h>
#include <sys/stat.h>
#include "NoximTile.h"
#include "NoximVLink.h"
#include "NoximGlobalRoutingTable.h"
#include "NoximGlobalTrafficTable.h"
#include "NoximThermal_IF.h"


using namespace std;

extern ofstream results_log_pwr;

SC_MODULE(NoximNoC)
{

    // I/O Ports
    sc_in_clk clock;		// The input clock for the NoC
    sc_in < bool > reset;	// The reset signal for the NoC

    // Signals
	/****************MODIFY BY HUI-SHUN********************/
    //Hui-shun add the third dim in the end
	//and add up/down signals

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////  added by Amin Norollah 
	//taheri//تعریف سیگنال های شبکه برای کانال مجازی
	sc_signal <bool> req_to_east                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
    sc_signal <bool> req_to_west                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
    sc_signal <bool> req_to_south                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
    sc_signal <bool> req_to_north                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	//To Tile and To Vertical Link
	sc_signal <bool> req_toT_up                    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	sc_signal <bool> req_toT_down                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	sc_signal <bool> req_toV_up                    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	sc_signal <bool> req_toV_down                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	
    sc_signal <bool> ack_to_east                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
    sc_signal <bool> ack_to_west                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
    sc_signal <bool> ack_to_south                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
    sc_signal <bool> ack_to_north                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	
	sc_signal <bool> ack_toT_up                    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	sc_signal <bool> ack_toT_down                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	sc_signal <bool> ack_toV_up                    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	sc_signal <bool> ack_toV_down                  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][DEFAULT_NUM_VC];
	
    sc_signal <NoximFlit> flit_to_east             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_west             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_south            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_to_north            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	//To Tile and To Vertical Link
	sc_signal <NoximFlit> flit_toT_up              [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_toT_down            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal <NoximFlit> flit_toV_up              [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximFlit> flit_toV_down            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

    sc_signal <int> free_slots_to_east             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_west             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_south            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <int> free_slots_to_north            [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal <int> free_slots_to_up               [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal <int> free_slots_to_down             [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	
    // NoP 
    sc_signal <NoximNoP_data> NoP_data_to_east     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_west     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_south    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_north    [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal <NoximNoP_data> NoP_data_to_up       [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
    sc_signal <NoximNoP_data> NoP_data_to_down     [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	// RCA Monitor Network
	 //  Derek ------------------2012.03.07
	sc_signal<int>    RCA_data_to_east0[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
	sc_signal<int>    RCA_data_to_east1[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
	sc_signal<int>    RCA_data_to_west0[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
	sc_signal<int>    RCA_data_to_west1[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
	sc_signal<int>    RCA_data_to_south0[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
	sc_signal<int>    RCA_data_to_south1[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
	sc_signal<int>    RCA_data_to_north0[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
	sc_signal<int>    RCA_data_to_north1[MAX_STATIC_DIM+1][MAX_STATIC_DIM+1][MAX_STATIC_DIM+1];
  
	sc_signal<double>    RCA_to_east               [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<double>    RCA_to_west               [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<double>    RCA_to_south              [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<double>    RCA_to_north              [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<double>    RCA_to_up                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<double>    RCA_to_down               [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	/*******THROTTLING******/
	sc_signal<bool>	on_off_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<bool>	on_off_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<bool>	on_off_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<bool>	on_off_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<bool>	on_off_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<bool>	on_off_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	
	sc_signal<float>	TB_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<float>	TB_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<float>	TB_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<float>	TB_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<float>	TB_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	sc_signal<float>	TB_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal<float>        PDT_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        PDT_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        PDT_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        PDT_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        PDT_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        PDT_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];


        sc_signal<float>        buf0_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf0_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf0_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf0_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf0_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf0_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal<float>        buf1_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf1_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf1_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf1_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf1_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf1_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal<float>        buf2_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf2_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf2_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf2_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf2_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf2_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal<float>        buf3_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf3_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf3_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf3_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf3_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf3_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	
	sc_signal<float>        buf4_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf4_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf4_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf4_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf4_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf4_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal<float>        buf5_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf5_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf5_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf5_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf5_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf5_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal<float>        buf6_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf6_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf6_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf6_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf6_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf6_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal<float>        buf7_to_east                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf7_to_west                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf7_to_south                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf7_to_north                [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf7_to_up                   [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
        sc_signal<float>        buf7_to_down                 [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];

	sc_signal< NoximNoP_data > vertical_free_slot  [MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1][MAX_STATIC_DIM + 1];
	// Matrix of tiles
    NoximTile  *t[MAX_STATIC_DIM][MAX_STATIC_DIM][MAX_STATIC_DIM];
	NoximVLink *v[MAX_STATIC_DIM][MAX_STATIC_DIM];
    // Global tables
    NoximGlobalRoutingTable grtable;
    NoximGlobalTrafficTable gttable;
    // Constructor
    SC_CTOR(NoximNoC) {
		// Build the Mesh
		buildMesh();
		//---------- Hot spot interface BY CMH <start>
		//HS_initial();
		HS_interface = new Thermal_IF(NoximGlobalParams::mesh_dim_x,NoximGlobalParams::mesh_dim_y, NoximGlobalParams::mesh_dim_z);
		instPowerTrace   .resize(3*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y*NoximGlobalParams::mesh_dim_z, 0);
		overallPowerTrace.resize(3*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y*NoximGlobalParams::mesh_dim_z, 0);
		TemperatureTrace .resize(3*NoximGlobalParams::mesh_dim_x*NoximGlobalParams::mesh_dim_y*NoximGlobalParams::mesh_dim_z, 0);
		SC_METHOD(entry);
		sensitive << reset;
		sensitive << clock.pos();
		//---------- Hot spot interface BY CMH <end>
    }
    ~NoximNoC() 
	{ 
	  delete HS_interface; 
	};
	// Support methods
    NoximTile *searchNode(const int id) const;
	bool EmergencyDecision();

  private:

	    void buildMesh();
		void entry();
	    Thermal_IF* HS_interface;
        //Power trace	
		vector<double> instPowerTrace;
		vector<double> overallPowerTrace;
		//get transient power in one sample period
		void transPwr2PtraceFile();
		void steadyPwr2PtraceFile();
		//Run-Time Thermal Management
		void TAVT(bool &isEmergency);
		void TAVT_MAX(bool &isEmergency);
		void Vertical(bool &isEmergency);
		void Vertical_MAX(bool &isEmergency);
		void GlobalThrottle(bool &isEmergency);
		void DistributedThrottle(bool &isEmergency);
		//Temperature trace 
		vector<double> TemperatureTrace;
        void setTemperature();
		void calROC(int &col_max, int &col_min, int &row_max, int &row_min,int non_beltway_layer);
		void setCleanStage();
		void EndCleanStage();
		void Reconfiguration();
		void findNonXLayer(int &non_throt_layer, int &non_beltway_layer);
		void TransientLog();
		void _throt_case_setting(int _thort_case);
		void _setThrot(int i, int j, int k);
		void _setNormal(int i, int j, int k);
		bool _equal(int x, int y, int z, int m, int n, int o);
		bool _CleanDone();
		bool _emergency;
		bool _clean;
};

#endif
