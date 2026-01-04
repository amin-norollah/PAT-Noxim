/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the top-level of Noxim
 *
 *
 * Edit by Amin Norollah @BALRUG and Danesh Derafshi(2018 Feb 20)
 * Pipeline Router
 */
#include "NoximMain.h"
#include "NoximNoC.h"
#include "NoximGlobalStats.h"
#include "NoximCmdLineParser.h"
#include "NoximLog.h"
#include <sys/stat.h>

using namespace std;

// Global variable to track the total volume of flits/packets drained during simulation
// This is used for simulation stopping condition when "-volume" option is specified
unsigned int drained_volume;

// Log file streams for recording various simulation metrics
ofstream results_log_pwr_router;    // Logs router power consumption (initialized in NoximLog, recorded in thermal_IF.cpp)
ofstream results_log_pwr_mac;       // Logs MAC (Multiply-Accumulate) unit power consumption
ofstream results_log_pwr_mem;       // Logs memory power consumption
ofstream transient_log_throughput;  // Logs transient throughput metrics (initialized in NoximLog, recorded in NoximNoC.cpp)
ofstream log_get_error;             // Logs any errors encountered during simulation
ofstream transient_topology;        // Logs transient topology changes (e.g., throttling, beltway activation)
ofstream static_log_power;          // Logs static power consumption over time
ofstream temp_tei_power;            // Logs temperature effect inversion (TEI) power data
// Initialize global configuration parameters (can be overridden with command-line arguments)
int                          NoximGlobalParams::verbose_mode                  = DEFAULT_VERBOSE_MODE;
int                          NoximGlobalParams::trace_mode                    = DEFAULT_TRACE_MODE;
char                         NoximGlobalParams::trace_filename[128]           = DEFAULT_TRACE_FILENAME;
int                          NoximGlobalParams::mesh_dim_x                    = DEFAULT_MESH_DIM_X;
int                          NoximGlobalParams::mesh_dim_y                    = DEFAULT_MESH_DIM_Y;
int                          NoximGlobalParams::mesh_dim_z                    = DEFAULT_MESH_DIM_Z;
int                          NoximGlobalParams::num_vcs			              = DEFAULT_NUM_VC;				// added by Amin Norollah
int                          NoximGlobalParams::buffer_depth                  = DEFAULT_BUFFER_DEPTH;
int                          NoximGlobalParams::min_packet_size               = DEFAULT_MIN_PACKET_SIZE;
int                          NoximGlobalParams::max_packet_size               = DEFAULT_MAX_PACKET_SIZE;
int                          NoximGlobalParams::routing_algorithm             = DEFAULT_ROUTING_ALGORITHM;
char                         NoximGlobalParams::routing_table_filename[128]   = DEFAULT_ROUTING_TABLE_FILENAME;
int                          NoximGlobalParams::selection_strategy            = DEFAULT_SELECTION_STRATEGY;
float                        NoximGlobalParams::packet_injection_rate         = DEFAULT_PACKET_INJECTION_RATE;
float                        NoximGlobalParams::probability_of_retransmission = DEFAULT_PROBABILITY_OF_RETRANSMISSION;
int                          NoximGlobalParams::traffic_distribution          = DEFAULT_TRAFFIC_DISTRIBUTION;
char                         NoximGlobalParams::traffic_table_filename[128]   = DEFAULT_TRAFFIC_TABLE_FILENAME;
int                          NoximGlobalParams::simulation_time               = DEFAULT_SIMULATION_TIME;
int                          NoximGlobalParams::stats_warm_up_time            = DEFAULT_STATS_WARM_UP_TIME;
int                          NoximGlobalParams::rnd_generator_seed            = time(NULL);
bool                         NoximGlobalParams::detailed                      = DEFAULT_DETAILED;
float                        NoximGlobalParams::dyad_threshold                = DEFAULT_DYAD_THRESHOLD;
unsigned int                 NoximGlobalParams::max_volume_to_be_drained      = DEFAULT_MAX_VOLUME_TO_BE_DRAINED;
vector <pair <int, double> > NoximGlobalParams::hotspots;
int                          NoximGlobalParams::dw_layer_sel                  = DEFAULT_DW_LAYER_SEL;
int                          NoximGlobalParams::throt_type                    = DEFAULT_THROTTLING_TYPE;
int                          NoximGlobalParams::down_level                    = DEFAULT_DOWN_LEVEL;
float	                     NoximGlobalParams::throt_ratio	                  = DEFAULT_THROTTLING_RATIO;
bool                         NoximGlobalParams::buffer_alloc                  = DEFAULT_BUFFER_ALLOC;
int                          NoximGlobalParams::vertical_link                 = DEFAULT_VERTICAL_LINK;
bool                         NoximGlobalParams::cascade_node                  = DEFAULT_CASCADE_NODE;
bool                         NoximGlobalParams::Mcascade                      = DEFAULT_MCASCADE;
int                          NoximGlobalParams::Mcascade_step                 = 0;
bool                         NoximGlobalParams::beltway                       = DEFAULT_BELTWAY;
float                        NoximGlobalParams::beltway_trigger               = TEMP_THRESHOLD; 
bool                         NoximGlobalParams::Mbeltway                      = DEFAULT_MBELTWAY;
bool                         NoximGlobalParams::Sbeltway                      = DEFAULT_SBELTWAY;
int                          NoximGlobalParams::Sbeltway_ring                 = 0;
int                          NoximGlobalParams::ROC_UP                        = 4;
int                          NoximGlobalParams::ROC_DOWN                      = 3;
float                        NoximGlobalParams::beltway_ratio                 = 0.25;
bool                         NoximGlobalParams::Log_all_Temp                  = false;
int                          NoximGlobalParams::clean_stage_time              = 3000;
bool                         NoximGlobalParams::cal_temp                      = DEFAULT_CAL_TEMP;
int                          NoximGlobalParams::br_sel                        = INVALID_SELECTION;
bool                         NoximGlobalParams::message_level                 = DEFAULT_MESSAGE_LEVEL;
bool                         NoximGlobalParams::arch_router		              = DEFAULT_A_ROUTER;
bool                         NoximGlobalParams::arch_rc						  = DEFAULT_ROUTING_COMPUTATION;
bool                         NoximGlobalParams::arch_sa				          = DEFAULT_SWITCH_ALLOCATION;
bool                         NoximGlobalParams::arch_with_credit	          = DEFAULT_A_WITH_CREDIT;
int                          NoximGlobalParams::dynamic_throt_case			  = 0;

//---------------------------------------------------------------------------

// SystemC main function - entry point of the simulation
// This function initializes the NoC simulator and runs the simulation
int sc_main(int arg_num, char *arg_vet[])
//int main()		//for windows
{
    // Initialize the drained volume counter to zero at the start of simulation
    drained_volume = 0;

	////////////////////////////////////////
	////	Noxim - the NoC Simulator	////
	////	(C) University of Catania	////
	////            version IUST		////
	////			Baseline NOC		////
	////////////////////////////////////////
	
    // Display simulation header information
	cout << "\t//////////////////////////////////////////// "<< endl;
	cout << "\t////\tNoxim - the NoC Simulator\t//// "<< endl;
	cout << "\t////\t(C) University of Catania\t//// "<< endl;
	cout << "\t//////////////////////////////////////////// " << endl;
	cout << "\t//////////////////////////////////////////// " << endl;
	cout << "\t////\t\tversion IUST\t\t//// "<< endl;
	cout << "\t//////////////////////////////////////////// "<< endl << endl << endl;

    // Parse command-line arguments to configure simulation parameters
    parseCmdLine(arg_num, arg_vet);			//for linux

    // Create SystemC clock signal (1 nanosecond period)
    sc_clock clock("clock", 1, SC_NS);
    // Create reset signal for the NoC
    sc_signal <bool> reset;

    // Instantiate the Network-on-Chip (NoC) module
    NoximNoC *n = new NoximNoC("NoC");
    // Connect clock and reset signals to the NoC instance
    n->clock(clock);
    n->reset(reset);

	// Initialize logging system for simulation results
	NoximLog log;
	// Create results directory for storing output files
	if(!mkdir("results",0777)) cout<<"Making new directory results"<<endl;
	//if (!_mkdir("results")) cout << "Making new directory results" << endl;
	// Enable signal tracing if trace mode is enabled
	if (NoximGlobalParams::trace_mode)log.TraceSignal(n);
	// Initialize log files for different metrics
	log.PowerLog();        // Transient power trace file
	log.Throughput();      // Throughput logging
	log.staticPowerLog();  // Static power logging
	
    // Reset phase: Assert reset signal and hold it for DEFAULT_RESET_TIME cycles
    reset.write(1);
    cout << "Reset...";
    // Initialize random number generator with the configured seed
    srand(NoximGlobalParams::rnd_generator_seed);	// time(NULL));
	//DEFAULT_RESET_TIME = # of simulation cycle
    // Run simulation for reset duration (DEFAULT_RESET_TIME cycles)
    sc_start(DEFAULT_RESET_TIME * CYCLE_PERIOD + 1, SC_NS);
    // De-assert reset signal to start normal operation
    reset.write(0);
	cout << "done!\nNow running for " << NoximGlobalParams::simulation_time << " cycles..." << endl;
	cout << "|----------------------- ProgressBar ----------------------|" << endl;
	// sc_start( TEMP_REPORT_PERIOD - DEFAULT_RESET_TIME * CYCLE_PERIOD  , SC_NS);
	// int i;
	// for ( i = TEMP_REPORT_PERIOD ; i < NoximGlobalParams::simulation_time ; i += TEMP_REPORT_PERIOD ){
		// reset.write(1);
		// sc_start( DEFAULT_RESET_TIME * CYCLE_PERIOD , SC_NS);
		// reset.write(0);
		// cout<<getCurrentCycleNum()<<": Reset done."<<endl;
		// sc_start( TEMP_REPORT_PERIOD - DEFAULT_RESET_TIME * CYCLE_PERIOD  , SC_NS);
		// cout<<getCurrentCycleNum()<<" cycles run"<<endl;
		
		
	// }
	// sc_start(NoximGlobalParams::simulation_time * CYCLE_PERIOD - getCurrentCycleNum() , SC_NS);
	// Run the main simulation for the specified number of cycles
	sc_start(NoximGlobalParams::simulation_time * CYCLE_PERIOD , SC_NS);

    // Simulation complete - perform cleanup and final reporting
	cout << "Noxim simulation completed."                        << endl;
    cout << " ( " << getCurrentCycleNum() << " cycles executed)" << endl;
    
	// Close all log files and finalize logging
	if (NoximGlobalParams::trace_mode)log.TraceEnd();
	log.BufferLog(n);          // Log buffer statistics
	log.TrafficLog(n);         // Log traffic statistics
	log.PowerLogEnd();         // Close power log file
	log.ThroughputEnd();       // Close throughput log file
	log.staticPowerLogEnd();   // Close static power log file
	
    // Calculate and display global statistics
    NoximGlobalStats gs(n);
    gs.showStats( NoximGlobalParams::detailed);
    // Check if volume-based stopping condition was met
    if ((NoximGlobalParams::max_volume_to_be_drained > 0) && getCurrentCycleNum() >= NoximGlobalParams::simulation_time ) {
	cout << "\nWARNING! the number of flits specified with -volume option" << endl;
	cout << "has not been reached."                                        << endl;
	cout << "You might want to try an higher value of simulation cycles"   << endl;
	cout << "using -sim option."                                           << endl;
	cout << "\n Effective drained volume: "   << drained_volume            << endl;
    }

	return 0;
}
