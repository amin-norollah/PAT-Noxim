/**********************************************************************************************************
Copyright  2012   The Regents of the University of California
All Rights Reserved
 
Permission to copy, modify and distribute any part of this ORION3.0 software distribution for educational, 
research and non-profit purposes, without fee, and without a written agreement is hereby granted, provided 
that the above copyright notice, this paragraph and the following three paragraphs appear in all copies.
 
Those desiring to incorporate this ORION 3.0 software distribution into commercial products or use for 
commercial purposes should contact the Technology Transfer Office.

Technology Transfer Office
University of California, San Diego 
9500 Gilman Drive 
Mail Code 0910 
La Jolla, CA 92093-0910

Ph: (858) 534-5815
FAX: (858) 534-7345
E-MAIL:invent@ucsd.edu.

 
IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE USE OF THIS ORION 3.0 
SOFTWARE DISTRIBUTION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
 
THE ORION 3.0 SOFTWARE DISTRIBUTION PROVIDED HEREIN IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF 
CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.  
THE UNIVERSITY OF CALIFORNIA MAKES NO REPRESENTATIONS AND EXTENDS NO WARRANTIES OF ANY KIND, EITHER 
IMPLIED OR EXPRESS, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS 
FOR A PARTICULAR PURPOSE, OR THAT THE USE OF THE ORION 3.0 SOFTWARE DISTRIBUTION WILL NOT INFRINGE ANY 
PATENT, TRADEMARK OR OTHER RIGHTS.
**********************************************************************************************************/


/*******************************************************************
*Unit :
* 	p = pico, n = nano, u = micro, um2 = mirometer squared	
*	power : nW
*	capacitance : pF
*	energy = J
*      Area = um2, micrometer squared
*                  
********************************************************************/

#ifndef _TECHNOLOGY_AREA_POWER_H
#define _TECHNOLOGY_AREA_POWER_H

#include <sys/types.h>

/* This file contains parameters for 65nm, 45nm*/

/*======================Parameters for Area===========================*/
#if(PARM(TECH_POINT) == 90)
#define Area_NOR_um2         (4.23)  
#define Area_INV_um2         (2.82)  
#define Area_AND_um2     	(4.23)  
#define Area_DFF_um2         (16.23) 
#define Area_MUX2_um2        (7.06)  
#define Area_MUX3_um2        (11.29) 
#define Area_MUX4_um2        (16.93) 

#elif(PARM(TECH_POINT) == 65)
#define Area_NOR_um2         (1.44)  
#define Area_INV_um2         (1.08)  
#define Area_AND_um2     	(2.16)  
#define Area_DFF_um2         (6.84) 
#define Area_MUX2_um2        (3.6)  
#define Area_MUX3_um2        (9.36 ) /*old value*/
#define Area_MUX4_um2        (12.6 ) /*old value*/
#define Area_AOI_um2		(2.52)

#elif(PARM(TECH_POINT) == 45)
#define Area_NOR_um2      (.7056)   
#define Area_INV_um2     (.5292)  
#define Area_DFF_um2     (5.1156) 
#define Area_AOI_um2     (1.0584)  
#define Area_MUX2_um2    (1.764)  
#define Area_MUX3_um2    (2.998) 
#define Area_MUX4_um2    (4.2336) 


#elif(PARM(TECH_POINT) <= 45)
#define Area_NOR_um2     (2.52)   
#define Area_INV_um2     (1.44)  
#define Area_DFF_um2     (8.28) 
#define Area_AOI_um2     (2.52)  
#define Area_MUX2_um2    (6.12)  
#define Area_MUX3_um2    (9.36) 
#define Area_MUX4_um2    (12.6) 
#endif

/*======================Parameters for Leakage Power===========================*/
#if(PARM(TECH_POINT) == 65)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_leak_nW     (4.256)   
#define INV_leak_nW     (3.709)  
#define DFF_leak_nW     (19.595) 
#define AOI_leak_nW     (6.975)  
#define MUX2_leak_nW    (6.229 )  


#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_leak_nW     (11.375)   
#define INV_leak_nW     (11.318)  
#define DFF_leak_nW     (53.712) 
#define AOI_leak_nW     (17.159)  
#define MUX2_leak_nW    ( 16.400)  



#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_leak_nW     (25.334)   
#define INV_leak_nW     (26.363)  
#define DFF_leak_nW     (121.833) 
#define AOI_leak_nW     (33.371)  
#define MUX2_leak_nW    (36.226)  

#endif

#elif(PARM(TECH_POINT) == 45)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_leak_nW     (3.253654)  
#define INV_leak_nW     (2.658218) 
#define DFF_leak_nW     (13.359276) 
#define AOI_leak_nW     (6.253391)  
#define MUX2_leak_nW    (4.528161)  


#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_leak_nW     (9.264710) 
#define INV_leak_nW     (8.462244)  
#define DFF_leak_nW     (34.708040) 
#define AOI_leak_nW     (17.188076) 
#define MUX2_leak_nW    (12.075659) 



#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_leak_nW     (20.125159) 
#define INV_leak_nW     (19.306026)  
#define DFF_leak_nW     (75.611883)
#define AOI_leak_nW     (36.134066) 
#define MUX2_leak_nW    (22.682192)  

#endif

#elif(PARM(TECH_POINT) == 32)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_leak_nW     (4.612524)  
#define INV_leak_nW     (4.372441) 
#define DFF_leak_nW     (16.52763) 
#define AOI_leak_nW     (8.1847975)  
#define MUX2_leak_nW    (5.750311)  


#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_leak_nW     (12.9150675) 
#define INV_leak_nW     (12.242835)  
#define DFF_leak_nW     (46.277381) 
#define AOI_leak_nW     (22.917433) 
#define MUX2_leak_nW    (16.100872) 



#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_leak_nW     (32.287668) 
#define INV_leak_nW     (30.607087)  
#define DFF_leak_nW     (115.69345)
#define AOI_leak_nW     (57.293575) 
#define MUX2_leak_nW    (40.252175)  

#endif

#elif(PARM(TECH_POINT) <= 22)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_leak_nW     (6.918786)  
#define INV_leak_nW     (6.9959056) 
#define DFF_leak_nW     (20.6595375) 
#define AOI_leak_nW     (14.7326355)  
#define MUX2_leak_nW    (8.0504354)  

#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_leak_nW     (17.6309026) 
#define INV_leak_nW     (19.66103453)  
#define DFF_leak_nW     (55.5328572) 
#define AOI_leak_nW     (27.5009196) 
#define MUX2_leak_nW    (19.3210464) 

#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_leak_nW     (41.9739684) 
#define INV_leak_nW     (48.9713392)  
#define DFF_leak_nW     (172.1518536)
#define AOI_leak_nW     (93.961463) 
#define MUX2_leak_nW    (64.40348)  

#endif

#endif

/*======================Parameters for Load Capacitance===========================*/
#if(PARM(TECH_POINT) == 65)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_load_pF     (0.0012+0.0011)   
#define INV_load_pF     (0.0012)  
#define DFF_load_pF     (0.0008+0.0011) 
#define AOI_load_pF     (0.0011+0.0012+0.0011+0.0011)  
#define MUX2_load_pF    (0.0007 +0.0007+.0017) 
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_load_pF     (0.0012+ 0.0011)   
#define INV_load_pF     (0.0012)  
#define DFF_load_pF     (.0008+0.0011) 
#define AOI_load_pF     (0.0012+0.0012+ 0.0011+0.0012)  
#define MUX2_load_pF    (0.0007+0.0007+0.0017) 
#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_load_pF     (0.0012+0.0012)   
#define INV_load_pF     (0.0012)  
#define DFF_load_pF     (0.0008+0.0011) 
#define AOI_load_pF     (0.0012+0.0012+0.0012+0.0012+0.0012)  
#define MUX2_load_pF    (0.0012+ 0.0009+ 0.0019) 

#endif

#elif(PARM(TECH_POINT) == 45)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_load_pF     (0.0005915+0.0006139)
#define INV_load_pF     (0.0006537)
#define DFF_load_pF     (0.0004264+0.0005393)
#define AOI_load_pF     (0.0005761+0.0006128+0.0006082+0.0006109)
#define MUX2_load_pF    (0.0004026+0.0004506+0.0009332+0.0004883)
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_load_pF     (0.000606+0.0006321)  
#define INV_load_pF     (0.000682)
#define DFF_load_pF     (0.0004413+0.0005526)
#define AOI_load_pF     (0.000588+0.0006294+0.0006235+0.0006313)
#define MUX2_load_pF    (0.0004169+ 0.0004638+0.0009579)
#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_load_pF     (0.0006184+0.0006476)  
#define INV_load_pF     (0.0006999)
#define DFF_load_pF     (0.0004516+0.0005616)
#define AOI_load_pF     (0.0005973+0.0006431+0.0006333+0.0006467)
#define MUX2_load_pF    (0.0004249+0.000473+0.0009765)

#endif
#endif

/*======================Parameters for Internal Energy===========================*/
#if(PARM(TECH_POINT) == 65)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_int_J     (0.00001+0.0002)   
#define INV_int_J     (0.00005)  /*0.0000 in lib*/
#define DFF_int_J     (0.0028) 
#define AOI_int_J     (0.0002+0.0002+0.0007+0.0007)  
#define MUX2_int_J    (0.0012+0.0009+0.0018)  
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_int_J     (0.0001+0.0002)   
#define INV_int_J     (0.00005)  /*0.0000 in lib*/
#define DFF_int_J     (0.0028) 
#define AOI_int_J     (0.0002+.0001+0.0006+0.0006)  
#define MUX2_int_J    (0.0011+0.0009+0.00175)  
#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_int_J     (0.0001+0.0003)   
#define INV_int_J     (0.0005)  
#define DFF_int_J     (0.00305) 
#define AOI_int_J     (0.00025+0.0002+0.0008+0.0008+0.00105)  
#define MUX2_int_J    (0.00225+0.0018+0.0028 )  
#endif

#elif(PARM(TECH_POINT) == 45)
#if(PARM(TRANSISTOR_TYPE) == HVT)
#define NOR_int_J     (0.0001331+0.0003787)  
#define INV_int_J     (0.00002634)
#define DFF_int_J     (0.001585)
#define AOI_int_J     (0.0001556+0.001207+0.001275+0.001487)  
#define MUX2_int_J    (0.0001022+0.0002146+0.0008544+0.001122)
#elif(PARM(TRANSISTOR_TYPE) == NVT)
#define NOR_int_J     (0.0001178+0.0003667)  
#define INV_int_J     (0.00001899)
#define DFF_int_J     (0.001571)
#define AOI_int_J     (0.0001414+0.0001192+0.0005183+0.0005243)  
#define MUX2_int_J    (0.0007351+0.000565+0.0006304)
#elif(PARM(TRANSISTOR_TYPE) == LVT)
#define NOR_int_J     (0.0001102+0.0003439)  
#define INV_int_J     (0.000024675)
#define DFF_int_J     (0.001559)
#define AOI_int_J     (0.00013005+0.0001045+0.0004761+0.0004814)  
#define MUX2_int_J    (0.0007219+0.0005528+0.0006113)
#endif
#endif

/* Calculate leakage current under temperature */
#if(PARM(TECH_POINT) >= 90)
#define SS_NMOS         (34.94) //Subthreshold swing for nmos transistor
#define SS_PMOS         (33.92) //Subthreshold swing for pmos transistor
#define LEAKAGE_COEFF   (0.667)
#elif(PARM(TECH_POINT) == 65)
#define SS_NMOS         (37.76) //Subthreshold swing for nmos transistor
#define SS_PMOS         (35.91) //Subthreshold swing for pmos transistor
#define LEAKAGE_COEFF   (0.75)
#elif(PARM(TECH_POINT) == 45)
#define SS_NMOS         (41.1) //Subthreshold swing for nmos transistor
#define SS_PMOS         (39.23) //Subthreshold swing for pmos transistor
#define LEAKAGE_COEFF   (0.875)
#elif(PARM(TECH_POINT) == 32)
#define SS_NMOS         (47.06) //Subthreshold swing for nmos transistor
#define SS_PMOS         (46.95) //Subthreshold swing for pmos transistor
#define LEAKAGE_COEFF   (0.9)
#elif(PARM(TECH_POINT) <= 32)
#define SS_NMOS         (56.21) //Subthreshold swing for nmos transistor
#define SS_PMOS         (54.64) //Subthreshold swing for pmos transistor
#define LEAKAGE_COEFF   (1.025)
#endif


#define FACTOR_LEAKAGE_CURRENT_N (2*300 - (1.602176e-019*(PARM_Vdd/2 - PARM_tr))/ (SS_NMOS*1.380648e-023))*LEAKAGE_COEFF
#define FACTOR_LEAKAGE_CURRENT_P (2*300 - (1.602176e-019*(PARM_tr - PARM_Vdd/2))/ (SS_PMOS*1.380648e-023))*LEAKAGE_COEFF


#endif /* _SIM_POWER_V2_H */

