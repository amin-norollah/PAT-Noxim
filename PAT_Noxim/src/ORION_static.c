/*-------------------------------------------------------------------------
 *                             ORION 2.0 
 *
 *         					Copyright 2009 
 *  	Princeton University, and Regents of the University of California 
 *                         All Rights Reserved
 *
 *                         
 *  ORION 2.0 was developed by Bin Li at Princeton University and Kambiz Samadi at
 *  University of California, San Diego. ORION 2.0 was built on top of ORION 1.0. 
 *  ORION 1.0 was developed by Hangsheng Wang, Xinping Zhu and Xuning Chen at 
 *  Princeton University.
 *
 *  If your use of this software contributes to a published paper, we
 *  request that you cite our paper that appears on our website 
 *  http://www.princeton.edu/~peh/orion.html
 *
 *  Permission to use, copy, and modify this software and its documentation is
 *  granted only under the following terms and conditions.  Both the
 *  above copyright notice and this permission notice must appear in all copies
 *  of the software, derivative works or modified versions, and any portions
 *  thereof, and both notices must appear in supporting documentation.
 *
 *  This software may be distributed (but not offered for sale or transferred
 *  for compensation) to third parties, provided such third parties agree to
 *  abide by the terms and conditions of this notice.
 *
 *  This software is distributed in the hope that it will be useful to the
 *  community, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *
 *-----------------------------------------------------------------------*/

#include "ORION_static.h"

#if (PARM(TECH_POINT) == 90 && PARM(TRANSISTOR_TYPE) == LVT)
double NMOS_TAB[1] = {19.9e-9};
double PMOS_TAB[1] = {16.6e-9};
double NAND2_TAB[4] = {7.8e-9, 24.6e-9, 14.1e-9, 34.3e-9};
double NOR2_TAB[4] = {51.2e-9, 23.9e-9, 19.5e-9, 8.4e-9};
double DFF_TAB[1] = {219.7e-9};
#elif (PARM(TECH_POINT) == 90 && PARM(TRANSISTOR_TYPE) == NVT)
double NMOS_TAB[1] = {15.6e-9};
double PMOS_TAB[1] = {11.3e-9};
double NAND2_TAB[4] = {2.8e-9, 19.6e-9, 10.4e-9, 29.3e-9};
double NOR2_TAB[4] = {41.5e-9, 13.1e-9, 14.5e-9, 1.4e-9};
double DFF_TAB[1] = {194.7e-9};
#elif (PARM(TECH_POINT) == 90 && PARM(TRANSISTOR_TYPE) == HVT)
double NMOS_TAB[1] = {12.2e-9};
double PMOS_TAB[1] = {9.3e-9};
double NAND2_TAB[4] = {1.8e-9, 12.4e-9, 8.9e-9, 19.3e-9};
double NOR2_TAB[4] = {29.5e-9, 8.3e-9, 11.1e-9, 0.9e-9};
double DFF_TAB[1] = {194.7e-9};

/*
#elif (PARM(TECH_POINT) <= 65 && PARM(TRANSISTOR_TYPE) == LVT) 
double NMOS_TAB[1] = {311.7e-9};
double PMOS_TAB[1] = {674.3e-9};
double NAND2_TAB[4] = {303.0e-9, 423.0e-9, 498.3e-9, 626.3e-9};
double NOR2_TAB[4] ={556.0e-9, 393.7e-9, 506.7e-9, 369.7e-9};
double DFF_TAB[1] = {970.4e-9};
#elif (PARM(TECH_POINT) <= 65 && PARM(TRANSISTOR_TYPE) == NVT) 
double NMOS_TAB[1] = {115.1e-9};
double PMOS_TAB[1] = {304.8e-9};
double NAND2_TAB[4] = {111.4e-9, 187.2e-9, 230.7e-9, 306.9e-9};
double NOR2_TAB[4] ={289.7e-9, 165.7e-9, 236.9e-9, 141.4e-9};
double DFF_TAB[1] ={400.3e-9};
#elif (PARM(TECH_POINT) <= 65 && PARM(TRANSISTOR_TYPE) == HVT) 
double NMOS_TAB[1] = {18.4e-9};
double PMOS_TAB[1] = {35.2e-9};
double NAND2_TAB[4] = {19.7e-9, 51.3e-9, 63.0e-9, 87.6e-9};
double NOR2_TAB[4] ={23.4e-9, 37.6e-9, 67.9e-9, 12.3e-9};
double DFF_TAB[1] = {231.3e-9};
*/

#elif (PARM(TECH_POINT) == 65 && PARM(TRANSISTOR_TYPE) == LVT) 
double NMOS_TAB[1] = { 232.85e-9 };//
double PMOS_TAB[1] = { 232.685e-9 };//
double NAND2_TAB[4] = { 69.392e-9, 28.94925e-9, 49.59985e-9, 83.3595e-9 };
double NOR2_TAB[4] = { 70.044e-9, 70.05375e-9, 30.5344e-9, 49.492625e-9 };//
double DFF_TAB[1] = { 771.4e-9 };//
#elif (PARM(TECH_POINT) == 65 && PARM(TRANSISTOR_TYPE) == NVT) 
double NMOS_TAB[1] = { 21.545e-9 };//
double PMOS_TAB[1] = { 21.445e-9 };//
double NAND2_TAB[4] = { 21.685e-9, 8.7725e-9, 19.5275e-9, 19.8475e-9 };//
double NOR2_TAB[4] = { 23.348e-9, 23.35125e-9, 9.542e-9, 14.14075e-9 };//
double DFF_TAB[1] = { 260.3e-9 };//
#elif (PARM(TECH_POINT) == 65 && PARM(TRANSISTOR_TYPE) == HVT) 
double NMOS_TAB[1] = { 4.31e-9 };//
double PMOS_TAB[1] = { 4.25e-9 };//
double NAND2_TAB[4] = { 5.42125e-9, 2.92416e-9, 4.881875e-9, 5.0891e-9 };//
double NOR2_TAB[4] = { 4.9325e-9, 4.9325e-9, 2.04e-9, 3.0825e-9 };//
double DFF_TAB[1] = { 110.3e-9 };//

#elif (PARM(TECH_POINT) == 45 && PARM(TRANSISTOR_TYPE) == LVT) 
double NMOS_TAB[1] = { 291.028e-9 };  //
double PMOS_TAB[1] = { 285.621e-9 };  //
double NAND2_TAB[4] = { 58.3031e-9, 105.1575e-9, 110.9475e-9, 118.914e-9 };//
double NOR2_TAB[4] = { 80.7162e-9, 89.8642e-9, 25.1806e-9, 86.1595e-9 };//
double DFF_TAB[1] = { 995.2e-9 };//
#elif (PARM(TECH_POINT) == 45 && PARM(TRANSISTOR_TYPE) == NVT) 
double NMOS_TAB[1] = { 71.63e-9 };  //
double PMOS_TAB[1] = { 71.534e-9 }; //
double NAND2_TAB[4] = { 45.9125e-9, 56.735e-9, 69.3075e-9, 69.5475e-9 };//
double NOR2_TAB[4] = { 57.629975e-9, 57.63e-9, 32.7175e-9, 59.885e-9 };//
double DFF_TAB[1] = { 413.14e-9 };//
#elif (PARM(TECH_POINT) == 45 && PARM(TRANSISTOR_TYPE) == HVT) 
double NMOS_TAB[1] = { 12.83e-9 };  //
double PMOS_TAB[1] = { 12.772e-9 }; //
double NAND2_TAB[4] = { 6.425e-9, 5.825e-9, 12.265e-9, 12.465 e-9 };//
double NOR2_TAB[4] = { 9.2775e-9, 9.2775e-9, 3.025e-9, 6.4325e-9 };//
double DFF_TAB[1] = { 121.01e-9 };//

#elif (PARM(TECH_POINT) == 32 && PARM(TRANSISTOR_TYPE) == LVT) 
double NMOS_TAB[1] = { 475.932e-9 };//
double PMOS_TAB[1] = { 475.628e-9 };//
double NAND2_TAB[4] = { 169.84125e-9, 189.1725e-9, 234.4725e-9, 234.838125e-9 };//
double NOR2_TAB[4] = { 255.395e-9, 255.427e-9, 77.032e-9, 87.123e-9 };//
double DFF_TAB[1] = { 1287.02e-9 };
#elif (PARM(TECH_POINT) == 32 && PARM(TRANSISTOR_TYPE) == NVT) 
double NMOS_TAB[1] = { 158.25e-9 }; //
double PMOS_TAB[1] = { 158.15e-9 }; //
double NAND2_TAB[4] = { 123.2275e-9, 146.115e-9, 186.315e-9, 186.55875e-9 };//
double NOR2_TAB[4] = { 137.965e-9, 138.383e-9, 46.012e-9, 46.453e-9 };//
double DFF_TAB[1] = { 481.80e-9 };
#elif (PARM(TECH_POINT) == 32 && PARM(TRANSISTOR_TYPE) == HVT) 
double NMOS_TAB[1] = { 29.39e-9 }; //
double PMOS_TAB[1] = { 29.33e-9 }; //
double NAND2_TAB[4] = { 14.6375e-9, 9.41e-9, 24.0475e-9, 24.2225e-9 };//
double NOR2_TAB[4] = { 17.095e-9, 17.102e-9, 8.23e-9, 7.61e-9 };//
double DFF_TAB[1] = { 151.12e-9 };

#elif (PARM(TECH_POINT) <= 22 && PARM(TRANSISTOR_TYPE) == LVT) 
double NMOS_TAB[1] = { 475.932e-9 };//
double PMOS_TAB[1] = { 475.628e-9 };//
double NAND2_TAB[4] = { 169.84125e-9, 189.1725e-9, 234.4725e-9, 234.838125e-9 };//
double NOR2_TAB[4] = { 255.395e-9, 255.427e-9, 77.032e-9, 87.123e-9 };//
double DFF_TAB[1] = { 1287.02e-9 };
#elif (PARM(TECH_POINT) <= 22 && PARM(TRANSISTOR_TYPE) == NVT) 
double NMOS_TAB[1] = { 158.25e-9 }; //
double PMOS_TAB[1] = { 158.15e-9 }; //
double NAND2_TAB[4] = { 123.2275e-9, 146.115e-9, 186.315e-9, 186.55875e-9 };//
double NOR2_TAB[4] = { 137.965e-9, 138.383e-9, 46.012e-9, 46.453e-9 };//
double DFF_TAB[1] = { 481.80e-9 };
#elif (PARM(TECH_POINT) <= 22 && PARM(TRANSISTOR_TYPE) == HVT) 
double NMOS_TAB[1] = { 29.39e-9 }; //
double PMOS_TAB[1] = { 29.33e-9 }; //
double NAND2_TAB[4] = { 14.6375e-9, 9.41e-9, 24.0475e-9, 24.2225e-9 };//
double NOR2_TAB[4] = { 17.095e-9, 17.102e-9, 8.23e-9, 7.61e-9 };//
double DFF_TAB[1] = { 151.12e-9 };

#elif(PARM(TECH_POINT) >= 110 ) 
/* HACK: we assume leakage power above 110nm is small, so we don't provide leakage power for 110nm and above */
double NMOS_TAB[1] = {0};
double PMOS_TAB[1] = {0};
double NAND2_TAB[4] = {0,0,0,0};
double NOR2_TAB[4] ={0,0,0,0};
double DFF_TAB[1] = {0};
#endif

