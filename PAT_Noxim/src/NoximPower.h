/*
* Noxim - the NoC Simulator
*
* (C) 2005-2010 by the University of Catania
* For the complete list of authors refer to file ../doc/AUTHORS.txt
* For the license applied to these sources refer to file ../doc/LICENSE.txt
*
* This file contains the declaration of the power model
*/

#ifndef __NOXIMPOWER_H__
#define __NOXIMPOWER_H__

#include <cassert>
#include "NoximMain.h"
#include "NoximPEParam.h"
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
#include "ORION_router.h"
#include "ORION_link.h"
#include "ORION_parameter.h"
#ifdef __cplusplus
}
#endif
/********************************************************************************/
/**************************   New Power & Area model   **************************/
/**************************        2017.July.15        **************************/
/********************************************************************************/
class NoximPower {

public:
	/********************/
	/********area********/
	double	AR_buffer;
	double	AR_crossbar;
	double	AR_vcallocator;
	double	AR_swallocator;
	double	AR_total;
	double  LinkBetween2Router;
	void	clearArea();
	void	calculateArea();

	double getTotalRouterArea() { return AR_total; }
	double getBufferArea()		{ return AR_buffer; }
	double getVAArea()			{ return AR_vcallocator; }
	double getSAArea()			{ return AR_swallocator; }
	double getCrossbarArea()	{ return AR_crossbar; }
	double getCoreArea()		{ return AREA_TOTAL + AREA_L2; }
	//double getTotalArea()		{ return pow(sqrt(AR_total) + sqrt(AREA_TOTAL), 2); }
	double getTotalArea()		{ return AR_total + AREA_TOTAL; }

	/*********************/
	/********power********/
	double	input_buffer_read_dynamic_power;
	double	input_buffer_write_dynamic_power;

	double	output_buffer_read_dynamic_power;
	double	output_buffer_write_dynamic_power;

	double	central_buffer_read_dynamic_power;
	double	central_buffer_write_dynamic_power;

	double	crossbar_dynamic_power;
	double	switch_allocation_dynamic_power;
	double	vc_allocation_dynamic_power;
	double	clock_dynamic_power;
	double	link_dynamic_power;
	double  EstaticSteadyState;
	double  EstaticTransient;

	double	FPMACPowerTransient;
	double	RouterPowerTransient;
	double	MEMPowerTransient;

	double	FPMACPowerSteadyState;
	double	RouterPowerSteadyState;
	double	MEMPowerSteadyState;

	double	temp_corepower;
	double	temp_mempower;

	double LinkLeakagePower;

	double Real_cycle_num_interval;
	double Sim_cycle_num_interval;

	NoximPower();

	void clearEnergy();
	void input_buffer_write();
	void input_buffer_read();
	void output_buffer_write();
	void output_buffer_read();
	void central_buffer_write();
	void central_buffer_read();
	void crossbar();
	void switchAllocation();
	void vcAllocation();
	void clock();
	void linkFlit();
	void linkBit();
	void clearTransient();

	double getDynamicPower();
	double getStaticPowerSteadyState();
	double getStaticPowerTransient();
	double getCorePower();
	double getCoreStaticPower();
	void   staticPowerUnderTemperature(double Temp);

	void   resetPwr() { clearEnergy(); } // redundant, remove later
	void   resetTransientPwr() { clearTransient(); } // redundant, remove later

	// Get energy for per-flit operation
	double getFlitEnergyBufferWrite() { return input_buffer_write_dynamic_power; }
	double getFlitEnergyBufferRead() { return input_buffer_read_dynamic_power; }
	double getFlitEnergyVC() { return vc_allocation_dynamic_power; }
	double getFlitEnergyArbiterNControl() { return switch_allocation_dynamic_power; }
	double getFlitEnergyCrossbar() { return crossbar_dynamic_power; }
	double getFlitEnergyLinks() { return link_dynamic_power; }
	double getFlitEnergyClocking() { return clock_dynamic_power; }

	// Accumulation Set
	void Router2Lateral();
	void Router2Local();
	void RouterRxLateral();
	void Local2Router();
	void corePower();

	// Methods for getting short-term average power, in NoximNoC::entry(), before calling Hotspot
	double getTransientRouterPower() { return (RouterPowerTransient + EstaticTransient*TEMP_REPORT_NUM) * accEnergy_to_Transient_Power_coeff; }
	double getTransientFPMACPower() { return (FPMACPowerTransient + 0.5*ENERGY_STATIC*TEMP_REPORT_NUM) * accEnergy_to_Transient_Power_coeff; }
	double getTransientMEMPower() { return (MEMPowerTransient + 0.5*ENERGY_STATIC*TEMP_REPORT_NUM) * accEnergy_to_Transient_Power_coeff; }

	// Methods for getting steadystate average power, before calling Hotspot 
	double getSteadyStateRouterPower() { return (RouterPowerSteadyState + EstaticSteadyState + EstaticTransient*TEMP_REPORT_NUM) * accEnergy_to_SteadyState_Power_coeff; }
	double getSteadyStateFPMACPower() { return (FPMACPowerTransient + 0.5*ENERGY_STATIC*getCurrentCycleNum()) * accEnergy_to_SteadyState_Power_coeff; }
	double getSteadyStateMEMPower() { return (MEMPowerTransient + 0.5*ENERGY_STATIC*getCurrentCycleNum()) * accEnergy_to_SteadyState_Power_coeff; }



/*

The average energy dissipated by a flit for a hop switch was estimated
as being 0.151nJ, 0.178nJ, 0.182nJ and 0.189nJ for XY, Odd-Even, DyAD,
and NoP-OE respectively

We assumed the tile size to be 2mm x 2mm and that the tiles were
arranged in a regular fashion on the floorplan. The load wire
capacitance was set to 0.50fF per micron, so considering an average of
25% switching activity the amount of energy consumed by a flit for a
hop interconnect is 0.384nJ.

*/
/*// ------ Noxim Original Power Model <start>

#define PWR_ROUTING_XY             0.151e-9
#define PWR_ROUTING_WEST_FIRST     0.155e-9
#define PWR_ROUTING_NORTH_LAST     0.155e-9
#define PWR_ROUTING_NEGATIVE_FIRST 0.155e-9
#define PWR_ROUTING_ODD_EVEN       0.178e-9
#define PWR_ROUTING_DYAD           0.182e-9
#define PWR_ROUTING_FULLY_ADAPTIVE 0.0
#define PWR_ROUTING_TABLE_BASED    0.185e-9

#define PWR_SEL_RANDOM             0.002e-9
#define PWR_SEL_BUFFER_LEVEL       0.006e-9
#define PWR_SEL_NOP                0.012e-9

#define PWR_FORWARD_FLIT           0.384e-9
#define PWR_INCOMING               0.002e-9
#define PWR_STANDBY                0.0001e-9/2.0

// ------ Noxim Original Power Model <end> */

// ------- Intel 80-cores Power Model <start> 
//單位: Watt
//使用時會再乘上時脈週期(操作頻率倒數), 數值代表一週期所消耗之能量(E = P/f)
//數值為參考intel 80 core之數據, 後面的scaling factor用以調整至溫度符合該paper所述之110度
//ENERGY_SCALING_FACTOR 對累積的energy作scaling 加快模擬速度
/*
#define PWR_QUEUES_DATA_PATH            203.28e-3 *CYCLE_PERIOD*1e-9*2*2 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_MSINT                       55.44e-3  *CYCLE_PERIOD*1e-9*2*2 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_ARBITER_CONTROL             64.68e-3  *CYCLE_PERIOD*1e-9*2*2 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_CROSSBAR                    138.6e-3  *CYCLE_PERIOD*1e-9*2*2 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_LINKS                       157.08e-3 *CYCLE_PERIOD*1e-9*2*2 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_CLOCKING                    304.92e-3 *CYCLE_PERIOD*1e-9*2*2 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_LEAKAGE_ROUTER              70e-3     *CYCLE_PERIOD*1e-9     *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_DUAL_FPMACS                 1188e-3   *CYCLE_PERIOD*1e-9*2*6 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_IMEM                        363e-3    *CYCLE_PERIOD*1e-9*2*6 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_DMEM                        330e-3    *CYCLE_PERIOD*1e-9*2*6 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_RF                          132e-3    *CYCLE_PERIOD*1e-9*2*6 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_CLOCK_DISTRIBUTION          363e-3    *CYCLE_PERIOD*1e-9*2*6 *POWER_SCALING_FACTOR *ENERGY_SCALING_FACTOR
#define PWR_LEAKAGE_FPMAC               40e-3     *CYCLE_PERIOD*1e-9
#define PWR_LEAKAGE_IMEM                21e-3     *CYCLE_PERIOD*1e-9
#define PWR_LEAKAGE_DMEM                8e-3      *CYCLE_PERIOD*1e-9
#define PWR_LEAKAGE_RF                  7.5e-3    *CYCLE_PERIOD*1e-9
// ------- Intel 80-cores Power Model <end>
*/
/*
// ------- Intel 80-core Energy Table, modified by Chihhao <begin>
// 後面這些 *2 *2 與 *2 *6要再確認是甚麼
#define ENERGY_SCALING_FACTOR      1.00
#define ENERGY_QUEUES_DATA_PATH    0.20328 *ENERGY_SCALING_FACTOR*1e-9 *2 *2
#define ENERGY_MSINT               0.05544 *ENERGY_SCALING_FACTOR*1e-9 *2 *2
#define ENERGY_ARBITER_CONTROL     0.06468 *ENERGY_SCALING_FACTOR*1e-9 *2 *2
#define ENERGY_CROSSBAR            0.13860 *ENERGY_SCALING_FACTOR*1e-9 *2 *2
#define ENERGY_LINKS               0.15708 *ENERGY_SCALING_FACTOR*1e-9 *2 *2
#define ENERGY_CLOCKING            0.30492 *ENERGY_SCALING_FACTOR*1e-9 *2 *2
#define ENERGY_LEAKAGE_ROUTER      0.07000 *ENERGY_SCALING_FACTOR*1e-9
#define ENERGY_DUAL_FPMACS         1.18800 *ENERGY_SCALING_FACTOR*1e-9 *2 *6
#define ENERGY_RF                  0.13200 *ENERGY_SCALING_FACTOR*1e-9 *2 *6
#define ENERGY_IMEM                0.36300 *ENERGY_SCALING_FACTOR*1e-9 *2 *6
#define ENERGY_DMEM                0.33000 *ENERGY_SCALING_FACTOR*1e-9 *2 *6
#define ENERGY_CLOCK_DISTRIBUTION  0.36300 *ENERGY_SCALING_FACTOR*1e-9 *2 *6
#define ENERGY_LEAKAGE_FPMAC       0.04000 *ENERGY_SCALING_FACTOR*1e-9
#define ENERGY_LEAKAGE_IMEM        0.02100 *ENERGY_SCALING_FACTOR*1e-9
#define ENERGY_LEAKAGE_DMEM        0.00800 *ENERGY_SCALING_FACTOR*1e-9
#define ENERGY_LEAKAGE_RF          0.00750 *ENERGY_SCALING_FACTOR*1e-9
// ------- Intel 80-core Energy Model, modified by Chihhao <end>
*/
	/*
	// Accumulation functions called in Router
	void QueuesNDataPath();
	void Msint();
	void ArbiterNControl();
	void Crossbar();
	void Links();
	void Clocking();
	void LeakageRouter();

	// Accumulation functions called in tile
	void DualFpmacs();
	void Imem();
	void Dmem();
	void RF();
	void ClockDistribution();
	void LeakageFPMAC();
	void LeakageMEM();

	// Accumulation Set
	void Router2Lateral ();
	void Router2Local   ();
	void RouterRxLateral();
	void TileLeakage    ();

	// Methods for getting short-term average power, in NoximNoC::entry(), before calling Hotspot
	double getTransientRouterPower()   ;
	double getTransientFPMACPower()    ;
	double getTransientMEMPower()      ;

	// Methods for getting steadystate average power, before calling Hotspot
	double getSteadyStateRouterPower() ;
	double getSteadyStateFPMACPower()  ;
	double getSteadyStateMEMPower()    ;

	// Methods for resetting energy accumulation
	void   resetPwr()          { clearAllAccEnergy();       } // redundant, remove later
	void   resetTransientPwr() { clearTransientAccEnergy(); } // redundant, remove later
	void   clearAllAccEnergy();        // call at simulation setup (NoximNoC::entry() reset phase)
	void   clearTransientAccEnergy();  // call each interval in NoximNoC::entry()

	// Get energy for per-flit operation
	double getFlitEnergyQueuesNDataPath()   { return ENERGY_QUEUES_DATA_PATH    ; }
	double getFlitEnergyMsint()             { return ENERGY_MSINT               ; }
	double getFlitEnergyArbiterNControl()   { return ENERGY_ARBITER_CONTROL     ; }
	double getFlitEnergyCrossbar()          { return ENERGY_CROSSBAR            ; }
	double getFlitEnergyLinks()             { return ENERGY_LINKS               ; }
	double getFlitEnergyClocking()          { return ENERGY_CLOCKING            ; }
	double getFlitEnergyLeakageRouter()     { return ENERGY_LEAKAGE_ROUTER      ; }
	double getFlitEnergyLeakageFPMAC()      { return ENERGY_LEAKAGE_FPMAC + ENERGY_LEAKAGE_RF  ; }
	double getFlitEnergyLeakageMEM()        { return ENERGY_LEAKAGE_IMEM + ENERGY_LEAKAGE_DMEM ; }


	private:
	// parameters to convert energy and power, setting in NoximPower();
	int	   Real_cycle_num_per_10ms_interval;
	int	   Sim_cycle_num_per_10ms_interval;
	double Power_Scaling_Factor;
	double Energy_Extrapolation_Factor;
	double Accumulation_Interval_in_Sec;
	double accEnergy_to_Transient_Power_coeff;
	double accEnergy_to_SteadyState_Power_coeff;
	double Total_Simulation_Time_in_Sec;

	// power and accumulated energy for transient and steady state temperature computation
	double pwr_Transient_Router   , accEnergy_Transient_Router   ;
	double pwr_Transient_FPMAC    , accEnergy_Transient_FPMAC    ;
	double pwr_Transient_MEM      , accEnergy_Transient_MEM      ;
	double pwr_SteadyState_Router , accEnergy_SteadyState_Router ;
	double pwr_SteadyState_FPMAC  , accEnergy_SteadyState_FPMAC  ;
	double pwr_SteadyState_MEM    , accEnergy_SteadyState_MEM    ;
	/*
	// member data for power and energy computation of router
	double pwr_QueuesNDataPath	 , accEnergy_QueuesNDataPath	;
	double pwr_Msint             , accEnergy_Msint              ;
	double pwr_ArbiterNControl   , accEnergy_ArbiterNControl    ;
	double pwr_Crossbar          , accEnergy_Crossbar           ;
	double pwr_Links             , accEnergy_Links              ;
	double pwr_Clocking          , accEnergy_Clocking           ;
	double pwr_leakage_Router    , accEnergy_leakage_Router     ;
	// member data for power and energy computation of FPMAC and MEM and ClockDistribution
	double pwr_DualFpmacs        , accEnergy_DualFpmacs         ;
	double pwr_Imem              , accEnergy_Imem               ;
	double pwr_Dmem              , accEnergy_Dmem               ;
	double pwr_RF                , accEnergy_RF                 ;
	double pwr_ClockDistribution , accEnergy_ClockDistribution  ;
	double pwr_leakage_FPMAC     , accEnergy_leakage_FPMAC      ;
	double pwr_leakage_MEM       , accEnergy_leakage_MEM        ;
	*/


	/*// ------ Noxim Original Power Model <start>
	void Routing();
	void Selection();
	void Standby();
	void Forward();
	void Incoming();

	double getPower() {
	return pwr;
	} double getPwrRouting() {
	return pwr_routing;
	}
	double getPwrSelection() {
	return pwr_selection;
	}
	double getPwrForward() {
	return pwr_forward;
	}
	double getPwrStandBy() {
	return pwr_standby;
	}
	double getPwrIncoming() {
	return pwr_incoming;
	}

	private:

	double pwr_routing;
	double pwr_selection;
	double pwr_forward;
	double pwr_standby;
	double pwr_incoming;

	double pwr;
	// ------ Noxim Original Power Model <end> */
};

#endif
