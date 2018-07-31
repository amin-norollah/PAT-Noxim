/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the power model
 *
 * Edited by Amin Norollah @BALRUG (2017 NOV 4)
 */

#include "NoximPower.h"
using namespace std;

NoximPower::NoximPower()
{
	clearEnergy();
	clearTransient();
	clearArea();
	//initialize Power
	SIM_router_init	(&GLOB(SIM_router_info), &GLOB(SIM_router_power), NULL, NoximGlobalParams::num_vcs, NoximGlobalParams::buffer_depth, 0);

	//calculate Power
	input_buffer_read_dynamic_power    = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_INPUT_BUFFER_RD	);	//buffer read power
	input_buffer_write_dynamic_power   = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_INPUT_BUFFER_WR	);	//buffer write power
	output_buffer_read_dynamic_power   = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_OUTPUT_BUFFER_RD	);	//feature work
	output_buffer_write_dynamic_power  = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_OUTPUT_BUFFER_WR	);	//feature work
	central_buffer_read_dynamic_power  = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_CENTRAL_BUFFER_RD);	//feature work
	central_buffer_write_dynamic_power = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_CENTRAL_BUFFER_WR);	//feature work
	crossbar_dynamic_power			   = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_CROSSBAR			);
	switch_allocation_dynamic_power    = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_SA_A				);
	vc_allocation_dynamic_power		   = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_VC_A				);
	clock_dynamic_power				   = SIM_router_report_Edynamic(&GLOB(SIM_router_info), &GLOB(SIM_router_power), P_CLK				);

	temp_corepower		= (ENERGY_IFU * IFU_duty_cycle) + ENERGY_RU + (ENERGY_RF * LS_duty_cycle) + ENERGY_IS + ((ENERGY_FPU * FPU_duty_cycle) + (ENERGY_CALU * MUL_duty_cycle) + (ENERGY_INTALU * ALU_duty_cycle)/3) + (ENERGY_DC * LS_duty_cycle) + (ENERGY_IC * IFU_duty_cycle) + ENERGY_SQ;
	//temp_mempower		= MEAN_num_mem_access*((ENERGY_DC * LS_duty_cycle) + (ENERGY_IC * IFU_duty_cycle) + ENERGY_SQ);
	temp_mempower		= ENERGY_L2;

	//initialize & calculate Area
	calculateArea();

	//power link between 2 routers
	LinkBetween2Router = sqrt(AREA_TOTAL + AREA_L2) - sqrt(AR_total);
	link_dynamic_power = LinkDynamicEnergyPerBitPerMeter(LinkBetween2Router, PARM_VDD_V);

	//static initiate
	EstaticTransient = 0;
	EstaticSteadyState = 0;
	LinkLeakagePower = LinkLeakagePowerPerMeter(LinkBetween2Router, PARM_VDD_V)* PARM_flit_width;
	EstaticTransient = SIM_router_report_Estatic(&GLOB(SIM_router_power)) + LinkLeakagePower;
}

void NoximPower::calculateArea()
{
	//initialize Area
	SIM_router_init(&GLOB(SIM_router_info), NULL, &GLOB(SIM_router_area), NoximGlobalParams::num_vcs, NoximGlobalParams::buffer_depth, 0);
	//calculate
	AR_buffer		= SIM_router_area(&GLOB(SIM_router_area), A_BUFFER	);
	AR_crossbar		= SIM_router_area(&GLOB(SIM_router_area), A_CROSSBAR);
	AR_vcallocator	= SIM_router_area(&GLOB(SIM_router_area), A_VCALLOCATOR);
	AR_swallocator	= SIM_router_area(&GLOB(SIM_router_area), A_SWALLOCATOR);
	AR_total		= SIM_router_area(&GLOB(SIM_router_area), A_TOTAL	);
}

void   NoximPower::clearEnergy()  // call at simulation setup
{
	FPMACPowerSteadyState = 0;
	RouterPowerSteadyState = 0; //clear dynamic and leakage power in router
	MEMPowerSteadyState = 0;
}
void   NoximPower::clearTransient()  // call each interval 
{
	// power and accumulated energy for transient and steady state temperature computation
	FPMACPowerTransient = 0;
	RouterPowerTransient = 0;
	MEMPowerTransient = 0;
}

void   NoximPower::clearArea()
{
	AR_buffer = 0;
	AR_crossbar = 0;
	AR_vcallocator = 0;
	AR_swallocator = 0;
	AR_total = 0;
}

/*************************************************************************************************/

void NoximPower::corePower()
{
	FPMACPowerTransient += temp_corepower;
	FPMACPowerSteadyState += temp_corepower;
	MEMPowerTransient += temp_mempower/2;
	MEMPowerSteadyState += temp_mempower/2;
}

void   NoximPower::input_buffer_write()   
{
	RouterPowerTransient += input_buffer_write_dynamic_power;
	RouterPowerSteadyState += input_buffer_write_dynamic_power;
}

void   NoximPower::input_buffer_read()  
{
	RouterPowerTransient += input_buffer_read_dynamic_power;
	RouterPowerSteadyState += input_buffer_read_dynamic_power;
}

void   NoximPower::output_buffer_write()   
{
	RouterPowerTransient += output_buffer_write_dynamic_power;
	RouterPowerSteadyState += output_buffer_write_dynamic_power;
}

void   NoximPower::output_buffer_read()  
{
	RouterPowerTransient += output_buffer_read_dynamic_power;
	RouterPowerSteadyState += output_buffer_read_dynamic_power;
}

void   NoximPower::central_buffer_write()   
{
	RouterPowerTransient += central_buffer_write_dynamic_power;
	RouterPowerSteadyState += central_buffer_write_dynamic_power;
}

void   NoximPower::central_buffer_read()  
{
	RouterPowerTransient += central_buffer_read_dynamic_power;
	RouterPowerSteadyState += central_buffer_read_dynamic_power;
}

void   NoximPower::crossbar()  
{
	RouterPowerTransient += crossbar_dynamic_power;
	RouterPowerSteadyState += crossbar_dynamic_power;
}

void   NoximPower::switchAllocation()  
{
	RouterPowerTransient += switch_allocation_dynamic_power;
	RouterPowerSteadyState += switch_allocation_dynamic_power;
}

void   NoximPower::vcAllocation()  
{
	RouterPowerTransient += vc_allocation_dynamic_power;
	RouterPowerSteadyState += vc_allocation_dynamic_power;
}

void   NoximPower::clock()  
{
	RouterPowerTransient += clock_dynamic_power;
	RouterPowerSteadyState += clock_dynamic_power;
}

void   NoximPower::linkFlit()
{
	RouterPowerTransient += (link_dynamic_power * PARM_flit_width);
	RouterPowerSteadyState += (link_dynamic_power * PARM_flit_width);
}

void   NoximPower::linkBit()
{
	RouterPowerTransient += link_dynamic_power;
	RouterPowerSteadyState += link_dynamic_power;
}
/*************************************************************************************************/
/*************************************************************************************************/

void NoximPower::Router2Lateral()
{
	if (NoximGlobalParams::arch_router == 1)
		input_buffer_read();
	crossbar();
	linkFlit();	//for flit_tx signals
	linkBit();	//for ack_rx signal
	linkBit();	//for req_tx signal
}

void NoximPower::Router2Local()
{
	if (NoximGlobalParams::arch_router == 1)
		input_buffer_read();
	crossbar();
	corePower();
}

void NoximPower::Local2Router()
{
	corePower();
}

/*************************************************************************************************/

void NoximPower::RouterRxLateral()
{
	input_buffer_write();
}

double NoximPower::getDynamicPower()
{
	return RouterPowerSteadyState;
}

double NoximPower::getStaticPowerSteadyState()
{
	return EstaticSteadyState;
}

double NoximPower::getStaticPowerTransient()
{
	return EstaticTransient;
}

double NoximPower::getCorePower()
{
	return FPMACPowerSteadyState + MEMPowerSteadyState;
}

double NoximPower::getCoreStaticPower()
{
	return ENERGY_STATIC;
}

// Temperature Effect Inversion
void NoximPower::staticPowerUnderTemperature(double Temp)
{
	SIM_router_init(&GLOB(SIM_router_info), &GLOB(SIM_router_power), NULL, NoximGlobalParams::num_vcs, NoximGlobalParams::buffer_depth, Temp);
	EstaticSteadyState += EstaticTransient * TEMP_REPORT_NUM;
	EstaticTransient = SIM_router_report_Estatic(&GLOB(SIM_router_power)) + LinkLeakagePower;
}
//
//
//
//
//
//
//
//
//
//
//
/*		// old power model
NoximPower::NoximPower()
{
	// Set time scale for Noxim, Hostpot, and Interface
	Total_Simulation_Time_in_Sec     = 10   ; // total simulated virtual time: 10 second
	Accumulation_Interval_in_Sec     = 0.01  ; // 0.01 second, interval of transient temp. est. in HotSpot
	Real_cycle_num_per_10ms_interval = (int)1e7  ; // 1GHz router, 0.01 second interval = 10^7 cycle 
	Sim_cycle_num_per_10ms_interval  = (int)1e5  ; // Sim. 10^4 cycle for each real 10^7 cycle interval
	
	// Set this veariable for shorten the simulation cycle number
	Energy_Extrapolation_Factor = (double)Real_cycle_num_per_10ms_interval/Sim_cycle_num_per_10ms_interval;
	
	// Set this variable for scaling estimated power
	// fitting target: steady state avergate temperature = 80-core paper's avg temp. (要再重新驗證!!)
	Power_Scaling_Factor = 0.33 ;//0.2625; //0.25
	
	// Transient_Power = accEnergy_Transient * accEnergy_to_Transient_Power_coeff;
	accEnergy_to_Transient_Power_coeff = (Energy_Extrapolation_Factor/Accumulation_Interval_in_Sec)
	                                     *Power_Scaling_Factor;
	                                     
	// SteadyState_Power = accEnergy_SteadyState * accEnergy_to_SteadyState_Power_coeff;
	accEnergy_to_SteadyState_Power_coeff = (Energy_Extrapolation_Factor/Total_Simulation_Time_in_Sec)
	                                     *Power_Scaling_Factor;
	
	clearAllAccEnergy();
}	

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void   NoximPower::clearTransientAccEnergy()  // call each interval 
{
    // power and accumulated energy for transient and steady state temperature computation
    accEnergy_Transient_Router   = 0;
    accEnergy_Transient_FPMAC    = 0;
    accEnergy_Transient_MEM      = 0;
}

// ---------------------------------------------------------------------------

void   NoximPower::clearAllAccEnergy() // call at simulation setup
{
    accEnergy_SteadyState_Router = 0;
    accEnergy_SteadyState_FPMAC  = 0;
    accEnergy_SteadyState_MEM    = 0;
    
    clearTransientAccEnergy();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

void NoximPower::QueuesNDataPath()
{
  accEnergy_SteadyState_Router += ENERGY_QUEUES_DATA_PATH;
  accEnergy_Transient_Router   += ENERGY_QUEUES_DATA_PATH;
}

// ---------------------------------------------------------------------------

void NoximPower::Msint()
{
  accEnergy_SteadyState_Router += ENERGY_MSINT;
  accEnergy_Transient_Router   += ENERGY_MSINT;
}

// ---------------------------------------------------------------------------

void NoximPower::ArbiterNControl()
{
  accEnergy_SteadyState_Router += ENERGY_ARBITER_CONTROL;
  accEnergy_Transient_Router   += ENERGY_ARBITER_CONTROL;
}

// ---------------------------------------------------------------------------

void NoximPower::Crossbar()
{
  accEnergy_SteadyState_Router += ENERGY_CROSSBAR;
  accEnergy_Transient_Router   += ENERGY_CROSSBAR;
}

// ---------------------------------------------------------------------------

void NoximPower::Links()
{
  accEnergy_SteadyState_Router += ENERGY_LINKS;
  accEnergy_Transient_Router   += ENERGY_LINKS;
}

// ---------------------------------------------------------------------------

void NoximPower::Clocking()
{
  accEnergy_SteadyState_Router += ENERGY_CLOCKING;
  accEnergy_Transient_Router   += ENERGY_CLOCKING;
}

// ---------------------------------------------------------------------------

void NoximPower::DualFpmacs()
{
  accEnergy_SteadyState_FPMAC += ENERGY_DUAL_FPMACS;
  accEnergy_Transient_FPMAC   += ENERGY_DUAL_FPMACS;
}

// ---------------------------------------------------------------------------
	
void NoximPower::RF()
{
  accEnergy_SteadyState_FPMAC += ENERGY_RF;
  accEnergy_Transient_FPMAC   += ENERGY_RF;
}

// ---------------------------------------------------------------------------
	
void NoximPower::Imem()
{
  accEnergy_SteadyState_MEM += ENERGY_IMEM;
  accEnergy_Transient_MEM   += ENERGY_IMEM;
}

// ---------------------------------------------------------------------------
	
void NoximPower::Dmem()
{
  accEnergy_SteadyState_MEM += ENERGY_DMEM;
  accEnergy_Transient_MEM   += ENERGY_DMEM;
}

// ---------------------------------------------------------------------------
	
void NoximPower::ClockDistribution()
{
  accEnergy_SteadyState_Router += ENERGY_CLOCK_DISTRIBUTION * 0.3;
  accEnergy_SteadyState_FPMAC  += ENERGY_CLOCK_DISTRIBUTION * 0.4;
  accEnergy_SteadyState_MEM    += ENERGY_CLOCK_DISTRIBUTION * 0.3;
  
  accEnergy_Transient_Router   += ENERGY_CLOCK_DISTRIBUTION * 0.3;
  accEnergy_Transient_FPMAC    += ENERGY_CLOCK_DISTRIBUTION * 0.4;
  accEnergy_Transient_MEM      += ENERGY_CLOCK_DISTRIBUTION * 0.3;
}

// ---------------------------------------------------------------------------

void NoximPower::LeakageRouter()
{
  accEnergy_SteadyState_Router += ENERGY_LEAKAGE_ROUTER;
  accEnergy_Transient_Router   += ENERGY_LEAKAGE_ROUTER;
}

// ---------------------------------------------------------------------------
	
void NoximPower::LeakageFPMAC()
{
  accEnergy_SteadyState_FPMAC += ENERGY_LEAKAGE_FPMAC + ENERGY_LEAKAGE_RF;
  accEnergy_Transient_FPMAC   += ENERGY_LEAKAGE_FPMAC + ENERGY_LEAKAGE_RF;
}

// ---------------------------------------------------------------------------
	
void NoximPower::LeakageMEM()
{
  accEnergy_SteadyState_MEM += ENERGY_LEAKAGE_IMEM + ENERGY_LEAKAGE_DMEM;
  accEnergy_Transient_MEM   += ENERGY_LEAKAGE_IMEM + ENERGY_LEAKAGE_DMEM;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

double NoximPower::getTransientRouterPower()   
{
	pwr_Transient_Router = accEnergy_Transient_Router * accEnergy_to_Transient_Power_coeff;
	return pwr_Transient_Router;
}

// ---------------------------------------------------------------------------

double NoximPower::getTransientFPMACPower()    
{
	pwr_Transient_FPMAC = accEnergy_Transient_FPMAC * accEnergy_to_Transient_Power_coeff;
	return pwr_Transient_FPMAC;
}

// ---------------------------------------------------------------------------

double NoximPower::getTransientMEMPower()      
{
	pwr_Transient_MEM = accEnergy_Transient_MEM * accEnergy_to_Transient_Power_coeff;
	return pwr_Transient_MEM;	
}

// ---------------------------------------------------------------------------

double NoximPower::getSteadyStateRouterPower() 
{
	pwr_SteadyState_Router = accEnergy_SteadyState_Router * accEnergy_to_SteadyState_Power_coeff;
	return pwr_SteadyState_Router;
}

// ---------------------------------------------------------------------------

double NoximPower::getSteadyStateFPMACPower()  
{
	pwr_SteadyState_FPMAC = accEnergy_SteadyState_FPMAC * accEnergy_to_SteadyState_Power_coeff;
	return pwr_SteadyState_FPMAC;
}

// ---------------------------------------------------------------------------

double NoximPower::getSteadyStateMEMPower()    
{
	pwr_SteadyState_MEM = accEnergy_SteadyState_MEM * accEnergy_to_SteadyState_Power_coeff;
	return pwr_SteadyState_MEM;	
}

// ---------------------------------------------------------------------------

void NoximPower::Router2Lateral()
{
	Crossbar();
	Links   ();
}

void NoximPower::Router2Local  ()
{
	DualFpmacs       ();	
	Imem             ();						
	Dmem             ();				
	RF               ();			
	ClockDistribution();
}

void NoximPower::RouterRxLateral()
{
	Msint();
	QueuesNDataPath();	
	Clocking();
}

void NoximPower::TileLeakage()
{
	LeakageRouter();
	LeakageFPMAC();	
	LeakageMEM(); 
}
  */
/*	// ------ Noxim Original Power Model <start> 
	pwr = 0.0;

    pwr_standby = PWR_STANDBY;
    pwr_forward = PWR_FORWARD_FLIT;
    pwr_incoming = PWR_INCOMING;

    if (NoximGlobalParams::routing_algorithm == ROUTING_XYZ)
	pwr_routing = PWR_ROUTING_XY;
    else if (NoximGlobalParams::routing_algorithm == ROUTING_ZXY)
	pwr_routing = PWR_ROUTING_XY;
    else if (NoximGlobalParams::routing_algorithm == ROUTING_WEST_FIRST)
	pwr_routing = PWR_ROUTING_WEST_FIRST;
    else if (NoximGlobalParams::routing_algorithm == ROUTING_NORTH_LAST)
	pwr_routing = PWR_ROUTING_NORTH_LAST;
    else if (NoximGlobalParams::routing_algorithm ==
	     ROUTING_NEGATIVE_FIRST)
	pwr_routing = PWR_ROUTING_NEGATIVE_FIRST;
    else if (NoximGlobalParams::routing_algorithm == ROUTING_ODD_EVEN)
	pwr_routing = PWR_ROUTING_ODD_EVEN;
    else if (NoximGlobalParams::routing_algorithm == ROUTING_DYAD)
	pwr_routing = PWR_ROUTING_DYAD;
    else if (NoximGlobalParams::routing_algorithm ==
	     ROUTING_FULLY_ADAPTIVE)
	pwr_routing = PWR_ROUTING_FULLY_ADAPTIVE;
    else if (NoximGlobalParams::routing_algorithm == ROUTING_TABLE_BASED)
	pwr_routing = PWR_ROUTING_TABLE_BASED;
    else
	assert(false);

    if (NoximGlobalParams::selection_strategy == SEL_RANDOM)
	pwr_selection = PWR_SEL_RANDOM;
    else if (NoximGlobalParams::selection_strategy == SEL_BUFFER_LEVEL)
	pwr_selection = PWR_SEL_BUFFER_LEVEL;
    else if (NoximGlobalParams::selection_strategy == SEL_NOP)
	pwr_selection = PWR_SEL_NOP;
    else
	assert(false);
}

void NoximPower::Routing()
{
    pwr += pwr_routing;
}

void NoximPower::Selection()
{
    pwr += pwr_selection;
}

void NoximPower::Standby()
{
    pwr += pwr_standby;
}

void NoximPower::Forward()
{
    pwr += pwr_forward;
}

void NoximPower::Incoming()
{
    pwr += pwr_incoming;
}
// ------ Noxim Original Power Model <end> */
