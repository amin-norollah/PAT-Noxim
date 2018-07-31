/**************************************************************************************************************************************
***************************************************************************************************************************************
*
* Noxim Parameters
*
* Default configuration can be overridden with command-line arguments
*
* LAST EDIT : (2017 NOV 4)
*
***************************************************************************************************************************************
***************************************************************************************************************************************/
// Architecture Routers added by Amin Norollah
#define DEFAULT_A_ROUTER                        1	
#define DEFAULT_ROUTING_COMPUTATION             0	//0:normal           , 1:pre-routing (pre_RC/VA)
#define DEFAULT_SWITCH_ALLOCATION               0	//0:normal           , 1:speculation SA (SA/VA)
#define DEFAULT_A_WITH_CREDIT                   1	

// other setting in Noxim
#define DEFAULT_VERBOSE_MODE                    VERBOSE_OFF
#define DEFAULT_TRACE_MODE                      false
#define DEFAULT_TRACE_FILENAME                  ""
#define DEFAULT_MESH_DIM_X                      4
#define DEFAULT_MESH_DIM_Y                      4
#define DEFAULT_MESH_DIM_Z                      2	//Default Number MESH_DIM_Z
#define DEFAULT_NUM_VC                          1	//Default VC Number
#define DEFAULT_BUFFER_DEPTH                    8
#define DEFAULT_MAX_PACKET_SIZE                 8
#define DEFAULT_MIN_PACKET_SIZE                 8
#define DEFAULT_ROUTING_ALGORITHM               ROUTING_XYZ  //ROUTING_XYZ  //ROUTING_WEST_FIRST //ROUTING_ODD_EVEN_3D //ROUTING_FULLY_ADAPTIVE
#define DEFAULT_ROUTING_TABLE_FILENAME          ""
#define DEFAULT_SELECTION_STRATEGY              SEL_RANDOM
#define DEFAULT_PACKET_INJECTION_RATE           0.01
#define DEFAULT_PROBABILITY_OF_RETRANSMISSION   0.01
#define DEFAULT_TRAFFIC_DISTRIBUTION            TRAFFIC_RANDOM
#define DEFAULT_TRAFFIC_TABLE_FILENAME          "test_traffic.txt"
#define DEFAULT_RESET_TIME                      1
#define DEFAULT_SIMULATION_TIME                 10000
#define DEFAULT_STATS_WARM_UP_TIME              DEFAULT_RESET_TIME
#define DEFAULT_DETAILED                        false
#define DEFAULT_DYAD_THRESHOLD                  0.6
#define DEFAULT_MAX_VOLUME_TO_BE_DRAINED        0
// MODIFY BY HUI-SHUN
#define DEFAULT_DW_LAYER_SEL                    DW_BL
#define DEFAULT_DOWN_LEVEL                      3
#define DEFAULT_THROTTLING_TYPE                 THROT_NORMAL
#define DEFAULT_THROTTLING_RATIO                0
#define DEFAULT_VERTICAL_LINK                   VERTICAL_MESH
#define DEFAULT_CASCADE_NODE                    false
#define DEFAULT_BELTWAY                         false
#define DEFAULT_MBELTWAY                        false
#define DEFAULT_SBELTWAY                        false
#define DEFAULT_MCASCADE                        false
#define DEFAULT_CLEAN_STAGE_TIME                10000
#define DEFAULT_CAL_TEMP                        true
//message level
#define DEFAULT_MESSAGE_LEVEL                   true
#define DEFAULT_PACKET_QUEUE_LENGTH             8	//not active -- NoximProcessingelement.cpp
//
#define MAX_ID                                  NoximGlobalParams::mesh_dim_x * NoximGlobalParams::mesh_dim_y * NoximGlobalParams::mesh_dim_z - 1
//latency histogram
#define HIST_OFF                                0
#define HIST_ON                                 1

#define DEFAULT_ODWL_RATIO                      false 
// MODIFY BY HUI-SHUN
// TODO by Fafa - this MUST be removed!!! Use only STL vectors instead!!!
#define MAX_STATIC_DIM                          16


/***************************************************************************************************************************************
****************************************************************************************************************************************
*
* Power and Area Model
*
* LAST EDIT : (2017 NOV 4)
*
****************************************************************************************************************************************
****************************************************************************************************************************************/
#define _SIM_PORT_H
/*Technology related parameters */
#define CORE_MODEL               2        // Core type, ARM_A9=0, NIAGARA1=1, NIAGARA2=2 or ALPHA=3
#define PARM_TECH_POINT          22       // unit : 90 , 65 , 45 , 32 , 22 nm
#define PARM_TRANSISTOR_TYPE     NVT      // transistor type, HVT, NVT, or LVT
#define PARM_Vdd                 1.0      // unit : v
#define PARM_Freq                1e9      // unit : Hz    //0.746e9
#define PARM_VDD_V               PARM_Vdd
#define PARM_FREQ_Hz             PARM_Freq
#define PARM_tr                  0.2      // unit : v


/* router module parameters */
/* general parameters */
#if DEFAULT_MESH_DIM_Z > 1  
#define PARM_in_port   DIRECTIONS + 1
#define PARM_out_port  DIRECTIONS + 1
#else  
#define PARM_in_port   DIRECTIONS - 1
#define PARM_out_port  DIRECTIONS - 1
#endif 
#define PARM_cache_in_port  0	/* # of cache input ports					no active*/
#define PARM_mc_in_port     0	/* # of memory controller input ports		no active*/
#define PARM_io_in_port     0	/* # of I/O device input ports				no active*/
#define PARM_cache_out_port 0	/* # of cache output ports					no active*/
#define PARM_mc_out_port    0	/* # of memory controller output ports		no active*/
#define PARM_io_out_port    0	/* # of I/O device output ports				no active*/
#define PARM_flit_width     128	/* flit width in bits */

/* virtual channel parameters */
#define PARM_v_class        1   /* # of total message classes */
#define PARM_v_channel      DEFAULT_NUM_VC	/* # of virtual channels per virtual message class*/
#define PARM_cache_class    0	/* # of cache port virtual classes				no active*/
#define PARM_mc_class       0	/* # of memory controller port virtual classes  no active*/
#define PARM_io_class       0	/* # of I/O device port virtual classes			no active*/
/* ?? */
#define PARM_in_share_buf   0	/* do input virtual channels physically share buffers? */
#define PARM_out_share_buf  0	/* do output virtual channels physically share buffers? */
/* ?? */
#define PARM_in_share_switch    1	/* do input virtual channels share crossbar input ports? */
#define PARM_out_share_switch   1	/* do output virtual channels share crossbar output ports? */

/* crossbar parameters */
#define PARM_crossbar_model     MULTREE_CROSSBAR     /* crossbar model type MATRIX_CROSSBAR, MULTREE_CROSSBAR, or CUT_THRU_CROSSBAR*/ 
#define PARM_crsbar_degree      4                    /* crossbar mux degree */
#define PARM_connect_type       TRISTATE_GATE        /* crossbar connector type */
#define PARM_trans_type         NP_GATE              /* crossbar transmission gate type */
#define PARM_crossbar_in_len    0                    /* crossbar input line length, if known */
#define PARM_crossbar_out_len   0                    /* crossbar output line length, if known */
#define PARM_xb_in_seg          0
#define PARM_xb_out_seg         0
/* HACK HACK HACK */
#define PARM_exp_xb_model       SIM_NO_MODEL   /* the other parameter is MATRIX_CROSSBAR */
#define PARM_exp_in_seg         2
#define PARM_exp_out_seg        2

/* input buffer parameters */
#define PARM_in_buf             1               /* have input buffer? */
#define PARM_in_buf_set         DEFAULT_BUFFER_DEPTH	
#define PARM_in_buf_rport       1               /* # of read ports */
#define PARM_in_buffer_type     REGISTER        /*buffer model type, SRAM or REGISTER*/

#define PARM_cache_in_buf          0
#define PARM_cache_in_buf_set      0
#define PARM_cache_in_buf_rport    0

#define PARM_mc_in_buf             0
#define PARM_mc_in_buf_set         0
#define PARM_mc_in_buf_rport       0

#define PARM_io_in_buf             0
#define PARM_io_in_buf_set         0
#define PARM_io_in_buf_rport       0

/* output buffer parameters */
#define PARM_out_buf               0         /* no output buffer */
#define PARM_out_buf_set           2
#define PARM_out_buf_wport         1
#define PARM_out_buffer_type       SRAM      /*buffer model type, SRAM or REGISTER*/

/* central buffer parameters */
#define PARM_central_buf          0          /* have central buffer? */
#define PARM_cbuf_set             2560       /* # of rows */
#define PARM_cbuf_rport           2          /* # of read ports */
#define PARM_cbuf_wport           2          /* # of write ports */
#define PARM_cbuf_width           4          /* # of flits in one row */
#define PARM_pipe_depth           4          /* # of banks */

/* array parameters shared by various buffers */
#define PARM_wordline_model     CACHE_RW_WORDLINE
#define PARM_bitline_model      RW_BITLINE
#define PARM_mem_model          NORMAL_MEM
#define PARM_row_dec_model      GENERIC_DEC
#define PARM_row_dec_pre_model  SINGLE_OTHER
#define PARM_col_dec_model      SIM_NO_MODEL
#define PARM_col_dec_pre_model  SIM_NO_MODEL
#define PARM_mux_model          SIM_NO_MODEL
#define PARM_outdrv_model       REG_OUTDRV

/* these 3 should be changed together */
/* use double-ended bitline because the array is too large */
#define PARM_data_end           2
#define PARM_amp_model          GENERIC_AMP
#define PARM_bitline_pre_model  EQU_BITLINE
//#define PARM_data_end         1
//#define PARM_amp_model        SIM_NO_MODEL
//#define PARM_bitline_pre_model SINGLE_OTHER

/* switch allocator arbiter parameters */
#define PARM_sw_in_arb_model        RR_ARBITER     /* input side arbiter model type, MATRIX_ARBITER , RR_ARBITER, QUEUE_ARBITER*/
#define PARM_sw_in_arb_ff_model     NEG_DFF        /* input side arbiter flip-flop model type */
#define PARM_sw_out_arb_model       RR_ARBITER     /* output side arbiter model type, MATRIX_ARBITER */
#define PARM_sw_out_arb_ff_model    NEG_DFF        /* output side arbiter flip-flop model type */

/* virtual channel allocator arbiter parameters */
#define PARM_vc_allocator_type      TWO_STAGE_ARB  /*vc allocator type, ONE_STAGE_ARB, TWO_STAGE_ARB, VC_SELECT*/
#define PARM_vc_in_arb_model        RR_ARBITER     /*input side arbiter model type for TWO_STAGE_ARB. MATRIX_ARBITER, RR_ARBITER, QUEUE_ARBITER*/
#define PARM_vc_in_arb_ff_model     NEG_DFF        /* input side arbiter flip-flop model type */
#define PARM_vc_out_arb_model       RR_ARBITER     /*output side arbiter model type (for both ONE_STAGE_ARB and TWO_STAGE_ARB). MATRIX_ARBITER, RR_ARBITER, QUEUE_ARBITER */
#define PARM_vc_out_arb_ff_model    NEG_DFF        /* output side arbiter flip-flop model type */
#define PARM_vc_select_buf_type     REGISTER       /* vc_select buffer type, SRAM or REGISTER */

/*link wire parameters*/
#define WIRE_LAYER_TYPE             GLOBAL         /*wire layer type, INTERMEDIATE or GLOBAL*/
#define PARM_width_spacing          DWIDTH_DSPACE  /*choices are SWIDTH_SSPACE, SWIDTH_DSPACE, DWIDTH_SSPACE, DWIDTH_DSPACE*/
#define PARM_buffering_scheme       MIN_DELAY      /*choices are MIN_DELAY, STAGGERED */
#define PARM_shielding              FALSE          /*choices are TRUE, FALSE */

/*clock power parameters*/
#define PARM_pipeline_stages        4              /*number of pipeline stages*/
#define PARM_H_tree_clock           0              /*1 means calculate H_tree_clock power, 0 means not calculate H_tree_clock*/
#define PARM_router_diagonal        442            /*router diagonal in micro-meter */

/* RF module parameters */
#define PARM_read_port          1
#define PARM_write_port         1
#define PARM_n_regs             64
#define PARM_reg_width          32

#define PARM_ndwl               1
#define PARM_ndbl               1
#define PARM_nspd               1


/***************************************************************************************************************************************
****************************************************************************************************************************************
*
* Thermal Parameters
*
* HotSpot setting
*
* LAST EDIT : (2017 NOV 16)
*
****************************************************************************************************************************************
***************************************************************************************************************************************/

#define POWER_SCALING                          2.2
#define Total_Simulation_Time_in_Sec           10
#define Accumulation_Interval_in_Sec           0.01
#define accEnergy_to_Transient_Power_coeff     (PARM_Freq / TEMP_REPORT_PERIOD)*POWER_SCALING
#define accEnergy_to_SteadyState_Power_coeff   ((PARM_Freq*Accumulation_Interval_in_Sec) / TEMP_REPORT_PERIOD / Total_Simulation_Time_in_Sec)*POWER_SCALING

//operating cycle period
#define CYCLE_PERIOD                1 / (PARM_Freq * 1e-9)           //unit: ns
#define TEMP_REPORT_NUM             100000                           //each 1e05 cycles calculate temperature
#define TEMP_REPORT_PERIOD          TEMP_REPORT_NUM * CYCLE_PERIOD   //unit: ns

//Initial temperature
#define INIT_TEMP                  (25 + 273.15)     //(18 + 273.15)    //(40 + 273.15)    //(60 + 273.15)
#define INIT_TEMP_80               (24.85 + 273.15)  //(17.85 + 273.15) //(39.85 + 273.15)
#define TEMP_THRESHOLD             98
#define BELTWAY_THRESHOLD          99.9

//<Specific heat capacity in J/(m^3K)>
#define HEAT_CAP_SILICON           1.75e6
#define HEAT_CAP_THERM_IF          4e6

//<Resistivity in (m-K)/W>
#define RESISTIVITY_SILICON        0.01
#define RESISTIVITY_THERM_IF       0.25

//<Thickness in m>
#define THICKNESS_SILICON          0.00015
#define THICKNESS_THERM_IF         2.0e-05


/***************************************************************************************************************************************
****************************************************************************************************************************************
*
* Other Parameters
*
* DO NOT CHANGE ANYTHING!
*
* LAST EDIT : (2017 May 25)
*
****************************************************************************************************************************************
***************************************************************************************************************************************/

// Define the directions as numbers
#define DIRECTIONS                   6
#define DIRECTION_NORTH              0
#define DIRECTION_EAST               1
#define DIRECTION_SOUTH              2
#define DIRECTION_WEST               3
#define DIRECTION_UP                 4 
#define DIRECTION_DOWN               5 
#define DIRECTION_LOCAL              6
#define DIRECTION_SEMI_LOCAL         7 //used for redirective pkt 

#define RCA_LOCAL_RATIO              0.3 //Traffic information based on local or remote
//0.7 for OE3D-version RCA
//0.8 optimization
#define RCA_1D                       1   //1D mode = 0, Fanin mode = 1
#define RCA_QUADRO                   0
// Generic not reserved resource
#define NOT_RESERVED                 -2
// To mark invalid or non exhistent values
#define NOT_VALID                    -1

// Routing algorithms
#define ROUTING_XYZ                  0
#define ROUTING_WEST_FIRST           1
#define ROUTING_NORTH_LAST           2
#define ROUTING_NEGATIVE_FIRST       3
#define ROUTING_ODD_EVEN             4
#define ROUTING_DYAD                 5
#define ROUTING_FULLY_ADAPTIVE       8
#define ROUTING_TABLE_BASED          9
#define ROUTING_PROPOSED             6
#define ROUTING_ZXY                  10
#define ROUTING_DOWNWARD             11
#define ROUTING_WEST_FIRST_DOWNWARD  12
#define ROUTING_ODD_EVEN_DOWNWARD    13
#define ROUTING_ODD_EVEN_3D          14
#define ROUTING_ODD_EVEN_Z           15
#define ROUTING_DLADR                16
#define ROUTING_DLAR                 17
#define ROUTING_DLDR                 18
#define ROUTING_DOWNWARD_CROSS_LAYER 19
#define ROUTING_DTBR                 20
#define ROUTING_off_on_xy2           21
#define INVALID_ROUTING              -1

// Selection strategies
#define SEL_RANDOM                   0
#define SEL_BUFFER_LEVEL             1
#define SEL_NOP                      2
#define SEL_RCA                      3
#define SEL_PROPOSED                 4
#define SEL_OBLIVIOUS                5
#define SEL_THERMAL                  6
#define INVALID_SELECTION            -1

// Traffic distribution              
#define TRAFFIC_RANDOM               0
#define TRAFFIC_TRANSPOSE1           1
#define TRAFFIC_TRANSPOSE2           2
#define TRAFFIC_HOTSPOT              3 //empty
#define TRAFFIC_TABLE_BASED          4
#define TRAFFIC_BIT_REVERSAL         5
#define TRAFFIC_SHUFFLE              6
#define TRAFFIC_BUTTERFLY            7
#define INVALID_TRAFFIC              -1
// Input Selection                   
#define INSEL_RANDOM                 0
#define INSEL_CS                     1
// Verbosity levels                  
#define VERBOSE_OFF                  0
#define VERBOSE_LOW                  1
#define VERBOSE_MEDIUM               2
#define VERBOSE_HIGH                 3

/****************MODIFY BY HUI-SHUN********************/
#define DW_BL                        0  //Bottom    Downward Layer
#define DW_ODWL                      1  //Oblivious Downward Layer
#define DW_ADWL                      2  //Adaptive  Downward Lyaer
#define DW_IPD                       3  //Increasing Path Diversity
#define DW_VBDR                      4  //Vertical Balanced Downward Routing
#define DW_ODWL_IPD                  5  //Oblivious Downward Layer + Increasing Path Diversity
/***************THROTTLING MECHANISM*******************/
#define THROT_NORMAL                 0
#define THROT_GLOBAL                 1
#define THROT_DISTRIBUTED            2
#define THROT_VERTICAL               3
#define THROT_TEST                   4
#define THROT_TAVT                   5
#define THROT_VERTICAL_MAX           6
#define THROT_TAVT_MAX               7
#define THROT_DYNAMIC                8
#define INVALID_THROT                9
/*** VERTICAL LINK***/
#define VERTICAL_MESH                0
#define VERTICAL_CROSSBAR            1

//Buffer allocation Uniform:1,2,2,11.
#define DEFAULT_BUFFER_ALLOC	     0
//Lateral
#define BUFFER_L_LAYER_0		     1 
#define BUFFER_L_LAYER_1		     2
#define BUFFER_L_LAYER_2		     2
#define BUFFER_L_LAYER_3		     11
//Up
#define BUFFER_U_LAYER_0		     1 
#define BUFFER_U_LAYER_1		     2
#define BUFFER_U_LAYER_2		     2
#define BUFFER_U_LAYER_3		     11
//Down
#define BUFFER_D_LAYER_0		     1 
#define BUFFER_D_LAYER_1		     2
#define BUFFER_D_LAYER_2		     2
#define BUFFER_D_LAYER_3		     11
/*******************************************************************/
/////	VC State
#define G_IDLE                    -1
#define G_ROUTING                  0
#define G_VCALLOCATING             1
#define G_ACTIVE                   2

#define G_VCS                      0
#define R_VCS                      1
#define O_VCS                      2
#define C_VCS                      3
#define ID_VCS                     4
/*******************************************************************/
/*******************************************************************/
/////	Power & Area
#define P_INPUT_BUFFER_RD         0
#define P_INPUT_BUFFER_WR         1
#define P_OUTPUT_BUFFER_RD        2
#define P_OUTPUT_BUFFER_WR        3
#define P_CENTRAL_BUFFER_RD       4
#define P_CENTRAL_BUFFER_WR       5
#define P_CROSSBAR                6
#define P_SA_A                    7
#define P_VC_A                    8
#define P_CLK                     9

#define A_BUFFER                  0
#define A_CROSSBAR                1
#define A_VCALLOCATOR             2
#define A_SWALLOCATOR             3
#define A_TOTAL                   4
#define A_CORE                    5
/*******************************************************************/
