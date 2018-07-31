/*
 *
 * LAST EDIT : (2017 NOV 4)
 *
 */
#include "NoximParameters.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////			ARM A9			////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if(CORE_MODEL == 0)
	/*===================== Parameters for DELAY ===========================*/
	/*=======================		ARM_A9		============================*/
	#define IFU_duty_cycle		  (1.82)		//Instruction Fetch Unit + Branch Prediction + Instruction Buffer + Branch Target Buffer + Instruction Cache
	#define ALU_duty_cycle		  (1.63)		//mean of ALU Instruction cycles such as AND, EOR, SUB, RSB, ADD, ADC, SBC, ...
	#define MUL_duty_cycle		  (4.5)			//mean of MUL Instruction cycles such as SMULL(S), UMULL(S), SMLAL(S), UMLAL(S), ...
	#define FPU_duty_cycle		  (4.25)		//mean of FPU Instruction cycles
	#define IC_duty_cycle		  (1)			//iCache Load/Store cycles
	#define LS_duty_cycle		  ceil(PARM_flit_width/32)		//Register/L1 cache Load/Store cycles

	/*===================== Parameters for ENERGY ==========================*/
	/*=======================		ARM_A9		============================*/
	#if(PARM_TECH_POINT == 90)
	#define ENERGY_IFU		  (0.6367723e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.0653077e-9)//Instruction Cache
	#define ENERGY_RU         (0.0806768e-9)//Renaming Unit
	#define ENERGY_DC         (0.282958e-9)	//Load Store Unit
	#define ENERGY_SQ     	  (0.0320437e-9)//Load Store Unit
	#define ENERGY_RF         (0.101138e-9)	//Register Files
	#define ENERGY_IS         (0.254652e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.059308e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.059308e-9)	//Floating Point Units
	#define ENERGY_CALU       (0.0790773e-9)//Complex ALUs
	#define ENERGY_STATIC	  (0.0179861e-9 + 0.109211e-9)//Subthreshold Leakage + Gate Leakage
	#define ENERGY_L2	      (4.65289e-9)		//L2 Cache

	#elif(PARM_TECH_POINT == 65)
	#define ENERGY_IFU		  (0.2079858e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.0462112e-9)	//Instruction Cache
	#define ENERGY_RU         (0.0275134e-9)	//Renaming Unit
	#define ENERGY_DC         (0.190469e-9)		//Load Store Unit
	#define ENERGY_SQ     	  (0.0109485e-9)	//Load Store Unit
	#define ENERGY_RF         (0.0364594e-9)	//Register Files
	#define ENERGY_IS         (0.0887491e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.0248072e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0248072e-9)	//Floating Point Units
	#define ENERGY_CALU       (0.0330763e-9)	//Complex ALUs
	#define ENERGY_STATIC	  (0.00889745e-9 + 0.0394264e-9)//Subthreshold Leakage + Gate Leakage
	#define ENERGY_L2	      (3.85133e-9)		//L2 Cache

	#elif(PARM_TECH_POINT == 45)
	#define ENERGY_IFU		  (0.1451726e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.0330954e-9)	//Instruction Cache
	#define ENERGY_DC         (0.13623816e-9)	//Load Store Unit
	#define ENERGY_SQ     	  (0.00738092e-9)	//Load Store Unit
	#define ENERGY_RF         (0.02499385e-9)	//Register Files
	#define ENERGY_IS         (0.0608266e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.0177025e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0177025e-9)	//Floating Point Units
	#define ENERGY_CALU       (0.02360345e-9)	//Complex ALUs
	#define ENERGY_STATIC	  (0.00493096e-9 + 0.0472331e-9)//Subthreshold Leakage + Gate Leakage
	#define ENERGY_L2	      (2.55403e-9)		//L2 Cache

	#elif(PARM_TECH_POINT <= 32)
	#define ENERGY_IFU		  (0.0640076e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.0117428e-9)	//Instruction Cache
	#define ENERGY_RU         (0.00877449e-9)	//Renaming Unit
	#define ENERGY_DC         (0.0486775e-9)	//Load Store Unit
	#define ENERGY_SQ     	  (0.00353831e-9)	//Load Store Unit
	#define ENERGY_RF         (0.0122264e-9)	//Register Files
	#define ENERGY_IS         (0.0275733e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.0090245e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0090245e-9)	//Floating Point Units
	#define ENERGY_CALU       (0.0120327e-9)	//Complex ALUs
	#define ENERGY_STATIC	  (0.0383459e-9 + 0.00110656e-9)//Subthreshold Leakage + Gate Leakage
	#define ENERGY_L2	      (1.65702e-9)		//L2 Cache
	#endif

	/*====================== Parameters for Area ==========================*/
	/*======================		ARM_A9		===========================*/
	#if(PARM_TECH_POINT == 90)
	#define AREA_IFU		(2501310)		//Instruction Fetch Unit
	#define AREA_RU         (92361)			//Renaming Unit
	#define AREA_DC         (1211490)		//Load Store Unit
	#define AREA_SQ     	(45896.4)		//Load Store Unit
	#define AREA_RF         (857506)		//Register Files
	#define AREA_IS         (330488)		//Instruction Scheduler
	#define AREA_INTALU     (240240)		//Integer ALUs
	#define AREA_FPU        (4917000)		//Floating Point Units
	#define AREA_CALU       (240240)		//Complex ALUs
	#define AREA_TOTAL      (10436531.4)	//Total
	#define AREA_L2		    (12798800)		//L2 Cache

	#elif(PARM_TECH_POINT == 65)
	#define AREA_IFU		(809254)		//Instruction Fetch Unit
	#define AREA_RU         (34527)			//Renaming Unit
	#define AREA_DC         (439304)		//Load Store Unit
	#define AREA_SQ     	(17682.8)		//Load Store Unit
	#define AREA_RF         (322802)		//Register Files
	#define AREA_IS         (126212)		//Instruction Scheduler
	#define AREA_INTALU     (142943)		//Integer ALUs
	#define AREA_FPU        (1836290)		//Floating Point Units
	#define AREA_CALU       (142943)		//Complex ALUs
	#define AREA_TOTAL      (3871957.8)		//Total
	#define AREA_L2		    (6122050)		//L2 Cache

	#elif(PARM_TECH_POINT == 45)
	#define AREA_IFU		(541773)		//Instruction Fetch Unit
	#define AREA_RU         (23090.4)		//Renaming Unit
	#define AREA_DC         (294216)		//Load Store Unit
	#define AREA_SQ     	(11865.4)		//Load Store Unit
	#define AREA_RF         (216037)		//Register Files
	#define AREA_IS         (84625.6)		//Instruction Scheduler
	#define AREA_INTALU     (117718)		//Integer ALUs
	#define AREA_FPU        (1229250)		//Floating Point Units
	#define AREA_CALU       (117718)		//Complex ALUs
	#define AREA_TOTAL      (2636293.4)		//Total
	#define AREA_L2		    (3300800)		//L2 Cache

	#elif(PARM_TECH_POINT <= 32)
	#define AREA_IFU		(291471)		//Instruction Fetch Unit
	#define AREA_RU         (985.9)		//Renaming Unit
	#define AREA_DC         (161189)		//Load Store Unit
	#define AREA_SQ     	(6280.5)		//Load Store Unit
	#define AREA_RF         (112888)		//Register Files
	#define AREA_IS         (44195.7)		//Instruction Scheduler
	#define AREA_INTALU     (82402.3)		//Integer ALUs
	#define AREA_FPU        (621606)		//Floating Point Units
	#define AREA_CALU       (82402.3)		//Complex ALUs
	#define AREA_TOTAL      (1403420.7)		//Total
	#define AREA_L2		    (1828328)		//L2 Cache
	#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////			NIAGARA	1		////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if(CORE_MODEL == 1)
	/*===================== Parameters for DELAY ===========================*/
	/*=======================		NIAGARA		============================*/
	#define IFU_duty_cycle		  (1.42)		//Instruction Fetch Unit + Branch Prediction + Instruction Buffer + Branch Target Buffer + Instruction Cache
	#define ALU_duty_cycle		  (1.47)		//mean of ALU Instruction cycles such as AND, EOR, SUB, RSB, ADD, ADC, SBC, ...
	#define MUL_duty_cycle		  (3.0)			//mean of MUL Instruction cycles such as SMULL(S), UMULL(S), SMLAL(S), UMLAL(S), ...
	#define FPU_duty_cycle		  (3.5)			//mean of FPU Instruction cycles
	#define IC_duty_cycle		  (1)			//iCache Load/Store cycles
	#define LS_duty_cycle		  ceil(PARM_flit_width/32)		//Register/L1 cache Load/Store cycles

	/*===================== Parameters for ENERGY ==========================*/
	/*=======================		NIAGARA		============================*/
	#if(PARM_TECH_POINT == 90)
	#define ENERGY_IFU		  (0.869914e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.444446e-9) //Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.112366e-9)	//Load Store Unit
	#define ENERGY_SQ     	  (0.567747e-9)	//Load Store Unit
	#define ENERGY_RF         (0.276957e-9)	//Register Files
	#define ENERGY_IS         (0.251168e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (1.5704e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0588899e-9)//Floating Point Units
	#define ENERGY_CALU       (0.392599e-9)	//Complex ALUs
	#define ENERGY_L2	      (1.33594e-9)	//L2 Cache
	#define ENERGY_STATIC	  (0.26984e-9 + 0.0779165e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT == 65)
	#define ENERGY_IFU		  (0.468526e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.138288e-9)//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.0723931e-9)//Load Store Unit
	#define ENERGY_SQ     	  (0.364892e-9)	//Load Store Unit
	#define ENERGY_RF         (0.174966e-9)	//Register Files
	#define ENERGY_IS         (0.15547e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (1.11648e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0418681e-9)//Floating Point Units
	#define ENERGY_CALU       (0.279121e-9)	//Complex ALUs
	#define ENERGY_L2	      (0.835154e-9)	//L2 Cache
	#define ENERGY_STATIC	  (0.464871e-9 + 0.131448e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT == 45)
	#define ENERGY_IFU		  (0.3920975e-9)//Instruction Fetch Unit
	#define ENERGY_IC		  (0.0849185e-9)//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.0463425e-9)//Load Store Unit
	#define ENERGY_SQ     	  (0.24241e-9)	//Load Store Unit
	#define ENERGY_RF         (0.11282e-9)	//Register Files
	#define ENERGY_IS         (0.102598e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.774855e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0290571e-9)//Floating Point Units
	#define ENERGY_CALU       (0.193714e-9)	//Complex ALUs
	#define ENERGY_L2	      (0.22557e-9)	//L2 Cache
	#define ENERGY_STATIC	  (0.670804e-9 + 0.0747292e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT <= 32)
	#define ENERGY_IFU		  (0.2145615e-9)//Instruction Fetch Unit
	#define ENERGY_IC		  (0.0546695e-9)//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.0294721e-9)//Load Store Unit
	#define ENERGY_SQ     	  (0.14431e-9)	//Load Store Unit
	#define ENERGY_RF         (0.0708936e-9)	//Register Files
	#define ENERGY_IS         (0.0632859e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.537653e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.020162e-9)//Floating Point Units
	#define ENERGY_CALU       (0.134413e-9)	//Complex ALUs
	#define ENERGY_L2	      (0.296355e-9)	//L2 Cache	
	#define ENERGY_STATIC	  (1.16096e-9 + 0.0969549e-9)//Subthreshold Leakage + Gate Leakage
	#endif

	/*====================== Parameters for Area ==========================*/
	/*======================		NIAGARA		===========================*/
	#if(PARM_TECH_POINT == 90)
	#define AREA_IFU		(4582490)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (1246990)		//Load Store Unit
	#define AREA_SQ     	(290925)		//Load Store Unit
	#define AREA_RF         (470200)		//Register Files
	#define AREA_IS         (47863)			//Instruction Scheduler
	#define AREA_INTALU     (160160)		//Integer ALUs
	#define AREA_FPU        (1164630)		//Floating Point Units
	#define AREA_CALU       (480480)		//Complex ALUs
	#define AREA_TOTAL      (8443738)		//Total
	#define AREA_L2		    (26798800)		//L2 Cache

	#elif(PARM_TECH_POINT == 65)
	#define AREA_IFU		(1493920)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (657712)		//Load Store Unit
	#define AREA_SQ     	(154952)		//Load Store Unit
	#define AREA_RF         (243134)		//Register Files
	#define AREA_IS         (25420.8)		//Instruction Scheduler
	#define AREA_INTALU     (112112)		//Integer ALUs
	#define AREA_FPU        (607474)		//Floating Point Units
	#define AREA_CALU       (336336)		//Complex ALUs
	#define AREA_TOTAL      (3631060.8)		//Total
	#define AREA_L2		    (16687600)		//L2 Cache

	#elif(PARM_TECH_POINT == 45)
	#define AREA_IFU		(689369)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (303314)		//Load Store Unit
	#define AREA_SQ     	(72483.3)		//Load Store Unit
	#define AREA_RF         (110530)		//Register Files
	#define AREA_IS         (1193.1)		//Instruction Scheduler
	#define AREA_INTALU     (78478.4)		//Integer ALUs
	#define AREA_FPU        (291156)		//Floating Point Units
	#define AREA_CALU       (235435)		//Complex ALUs
	#define AREA_TOTAL      (1781958.8)		//Total
	#define AREA_L2		    (7901600)		//L2 Cache
	
	#elif(PARM_TECH_POINT <= 32)
	#define AREA_IFU		(355937)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (156050)		//Load Store Unit
	#define AREA_SQ     	(37472)		//Load Store Unit
	#define AREA_RF         (55503.3)		//Register Files
	#define AREA_IS         (614.9)		//Instruction Scheduler
	#define AREA_INTALU     (54934.9)		//Integer ALUs
	#define AREA_FPU        (147232)		//Floating Point Units
	#define AREA_CALU       (164805)		//Complex ALUs
	#define AREA_TOTAL      (972549.1)		//Total
	#define AREA_L2		    (4043160)		//L2 Cache
	#endif
#endif
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////			NIAGARA	2		////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if(CORE_MODEL == 2)
	/*===================== Parameters for DELAY ===========================*/
	/*=======================		NIAGARA2	============================*/
	#define IFU_duty_cycle		  (1.35)		//Instruction Fetch Unit + Branch Prediction + Instruction Buffer + Branch Target Buffer + Instruction Cache
	#define ALU_duty_cycle		  (1.42)		//mean of ALU Instruction cycles such as AND, EOR, SUB, RSB, ADD, ADC, SBC, ...
	#define MUL_duty_cycle		  (3.0)			//mean of MUL Instruction cycles such as SMULL(S), UMULL(S), SMLAL(S), UMLAL(S), ...
	#define FPU_duty_cycle		  (3.5)			//mean of FPU Instruction cycles
	#define IC_duty_cycle		  (1)			//iCache Load/Store cycles
	#define LS_duty_cycle		  ceil(PARM_flit_width/32)		//Register/L1 cache Load/Store cycles

	/*===================== Parameters for ENERGY ==========================*/
	/*=======================		NIAGARA2	============================*/
	#if(PARM_TECH_POINT == 90)
	#define ENERGY_IFU		  (3.73319e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (2.63751e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.24975e-9)	//Load Store Unit
	#define ENERGY_SQ     	  (3.87582e-9)	//Load Store Unit
	#define ENERGY_RF         (0.765946e-9)	//Register Files
	#define ENERGY_IS         (0.701019e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (4.52274e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0848014e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (5.65702e-9)	//L2 Cache
	#define ENERGY_STATIC	  (1.04209e-9 + 0.155379e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT == 65)
	#define ENERGY_IFU		  (1.7204e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (1.46974e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.138741e-9)	//Load Store Unit
	#define ENERGY_SQ     	  (2.16817e-9)	//Load Store Unit
	#define ENERGY_RF         (0.415622e-9)	//Register Files
	#define ENERGY_IS         (0.370992e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (2.70189e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0506604e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (3.21499e-9)	//L2 Cache
	#define ENERGY_STATIC	  (1.08889e-9 + 0.234225e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT == 45)
	#define ENERGY_IFU		  (1.158845e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.351705e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.0754285e-9)//Load Store Unit
	#define ENERGY_SQ     	  (0.016303e-9)	//Load Store Unit
	#define ENERGY_RF         (0.225605e-9)	//Register Files
	#define ENERGY_IS         (0.205196e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (1.54971e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0290571e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (1.70949e-9)	//L2 Cache
	#define ENERGY_STATIC	  (0.811782e-9 + 0.0990756e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT <= 32)
	#define ENERGY_IFU		  (0.518989e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.180129e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.0397974e-9)//Load Store Unit
	#define ENERGY_SQ     	  (0.273106e-9)	//Load Store Unit
	#define ENERGY_RF         (0.11628e-9)	//Register Files
	#define ENERGY_IS         (0.10333e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.870997e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0163312e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (1.00991e-9)	//L2 Cache
	#define ENERGY_STATIC	  (0.945488e-9 + 0.108064e-9)//Subthreshold Leakage + Gate Leakage
	#endif

	/*====================== Parameters for Area ==========================*/
	/*======================		NIAGARA2	===========================*/
	#if(PARM_TECH_POINT == 90)
	#define AREA_IFU		(6471980)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (1227460)		//Load Store Unit
	#define AREA_SQ     	(1124160)		//Load Store Unit
	#define AREA_RF         (717202)		//Register Files
	#define AREA_IS         (95858.3)		//Instruction Scheduler
	#define AREA_INTALU     (320320)		//Integer ALUs
	#define AREA_FPU        (9317000)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (19273980.3)	//Total
	#define AREA_L2		    (18849500)		//L2 Cache

	#elif(PARM_TECH_POINT == 65)
	#define AREA_IFU		(3425520)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (651877)		//Load Store Unit
	#define AREA_SQ     	(596287)		//Load Store Unit
	#define AREA_RF         (404016)		//Register Files
	#define AREA_IS         (50846)			//Instruction Scheduler
	#define AREA_INTALU     (224224)		//Integer ALUs
	#define AREA_FPU        (4859790)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (10212560)		//Total
	#define AREA_L2		    (9924980)		//L2 Cache

	#elif(PARM_TECH_POINT == 45)
	#define AREA_IFU		(890943)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (303314)		//Load Store Unit
	#define AREA_SQ     	(83817)			//Load Store Unit
	#define AREA_RF         (190583)		//Register Files
	#define AREA_IS         (23862)			//Instruction Scheduler
	#define AREA_INTALU     (156957)		//Integer ALUs
	#define AREA_FPU        (2329250)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (3978726)		//Total
	#define AREA_L2		    (4685020)		//L2 Cache

	#elif(PARM_TECH_POINT <= 32)
	#define AREA_IFU		(458593)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (157229)		//Load Store Unit
	#define AREA_SQ     	(43093.8)			//Load Store Unit
	#define AREA_RF         (97342.2)		//Register Files
	#define AREA_IS         (12259.3)			//Instruction Scheduler
	#define AREA_INTALU     (109870)		//Integer ALUs
	#define AREA_FPU        (1177850)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (2056237.3)		//Total - L2
	#define AREA_L2		    (2638990)		//L2 Cache
	#endif
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////			ALPHA			////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if(CORE_MODEL == 3)
	/*===================== Parameters for DELAY ===========================*/
	/*==========================	  ALPHA	    ============================*/
	#define IFU_duty_cycle		  (1.42)		//Instruction Fetch Unit + Branch Prediction + Instruction Buffer + Branch Target Buffer + Instruction Cache
	#define ALU_duty_cycle		  (1.47)		//mean of ALU Instruction cycles such as AND, EOR, SUB, RSB, ADD, ADC, SBC, ...
	#define MUL_duty_cycle		  (3.0)			//mean of MUL Instruction cycles such as SMULL(S), UMULL(S), SMLAL(S), UMLAL(S), ...
	#define FPU_duty_cycle		  (3.5)			//mean of FPU Instruction cycles
	#define IC_duty_cycle		  (1)			//iCache Load/Store cycles
	#define LS_duty_cycle		  ceil(PARM_flit_width/32)		//Register/L1 cache Load/Store cycles

	/*===================== Parameters for ENERGY ==========================*/
	/*=======================		ALPHA		============================*/
	#if(PARM_TECH_POINT == 90)
	#define ENERGY_IFU		  (0.91813e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (3.88874e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.614226e-9)//Load Store Unit
	#define ENERGY_SQ     	  (0.567747e-9)	//Load Store Unit
	#define ENERGY_RF         (0.2769574e-9)//Register Files
	#define ENERGY_IS         (0.481703e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (1.5704e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0588899e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (1.7395e-9)	//L2 Cache
	#define ENERGY_STATIC	  (0.304406e-9 + 1.08059e-9)// Gate Leakage + Subthreshold Leakage

	#elif(PARM_TECH_POINT == 65)
	#define ENERGY_IFU		  (0.4996e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (2.5496e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.199109e-9) //Load Store Unit
	#define ENERGY_SQ     	  (0.364892e-9)	//Load Store Unit
	#define ENERGY_RF         (0.174966e-9)//Register Files
	#define ENERGY_IS         (0.291876e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (1.11648e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0418681e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (1.08744e-9)	//L2 Cache
	#define ENERGY_STATIC	  (1.60271e-9 + 0.453675e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT == 45)
	#define ENERGY_IFU		  (0.413928e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.843022e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.122784e-9) //Load Store Unit
	#define ENERGY_SQ     	  (0.224241e-9)	//Load Store Unit
	#define ENERGY_RF         (0.11282e-9)//Register Files
	#define ENERGY_IS         (0.203805e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.774855e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.0290571e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (0.554127e-9)	//L2 Cache
	#define ENERGY_STATIC	  (1.79463e-9 + 0.194911e-9)//Subthreshold Leakage + Gate Leakage

	#elif(PARM_TECH_POINT <= 32)
	#define ENERGY_IFU		  (0.227743e-9)	//Instruction Fetch Unit
	#define ENERGY_IC		  (0.544498e-9)	//Instruction Cache
	#define ENERGY_RU         (0)			//Renaming Unit
	#define ENERGY_DC         (0.079146e-9) //Load Store Unit
	#define ENERGY_SQ     	  (0.14431e-9)	//Load Store Unit
	#define ENERGY_RF         (0.0708936e-9)//Register Files
	#define ENERGY_IS         (0.122116e-9)	//Instruction Scheduler
	#define ENERGY_INTALU     (0.537653e-9)	//Integer ALUs
	#define ENERGY_FPU        (0.020162e-9)//Floating Point Units
	#define ENERGY_CALU       (0)			//Complex ALUs
	#define ENERGY_L2	      (0.296355e-9)	//L2 Cache
	#define ENERGY_STATIC	  (2.65681e-9 + 0.217399e-9)//Subthreshold Leakage + Gate Leakage
	#endif

	/*====================== Parameters for Area ==========================*/
	/*======================		ALPHA		===========================*/
	#if(PARM_TECH_POINT == 90)
	#define AREA_IFU		(12702000)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (4392530)		//Load Store Unit
	#define AREA_SQ     	(290925)		//Load Store Unit
	#define AREA_RF         (470200)		//Register Files
	#define AREA_IS         (73740.7)		//Instruction Scheduler
	#define AREA_INTALU     (320320)		//Integer ALUs
	#define AREA_FPU        (15268000)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (33517715.7)	//Total
	#define AREA_L2		    (26798800)		//L2 Cache

	#elif(PARM_TECH_POINT == 65)
	#define AREA_IFU		(9061200)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (1393970)		//Load Store Unit
	#define AREA_SQ     	(154952)		//Load Store Unit
	#define AREA_RF         (243134)		//Register Files
	#define AREA_IS         (38979.5)		//Instruction Scheduler
	#define AREA_INTALU     (224224)		//Integer ALUs
	#define AREA_FPU        (12439200)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (23555659.5)	//Total
	#define AREA_L2		    (16687600)		//L2 Cache

	#elif(PARM_TECH_POINT == 45)
	#define AREA_IFU		(2586440)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (642128)		//Load Store Unit
	#define AREA_SQ     	(72483)			//Load Store Unit
	#define AREA_RF         (110530)		//Register Files
	#define AREA_IS         (18396)			//Instruction Scheduler
	#define AREA_INTALU     (156957)		//Integer ALUs
	#define AREA_FPU        (6317000)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (9903934)		//Total
	#define AREA_L2		    (7901600)		//L2 Cache

	#elif(PARM_TECH_POINT <= 32)
	#define AREA_IFU		(1833790)		//Instruction Fetch Unit
	#define AREA_RU         (0)				//Renaming Unit
	#define AREA_DC         (331875)		//Load Store Unit
	#define AREA_SQ     	(374720)			//Load Store Unit
	#define AREA_RF         (15413.6)		//Register Files
	#define AREA_IS         (943)			//Instruction Scheduler
	#define AREA_INTALU     (109870)		//Integer ALUs
	#define AREA_FPU        (4711410)		//Floating Point Units
	#define AREA_CALU       (0)				//Complex ALUs
	#define AREA_TOTAL      (7378021.6)		//Total
	#define AREA_L2		    (4043160)		//L2 Cache
	#endif

#endif

