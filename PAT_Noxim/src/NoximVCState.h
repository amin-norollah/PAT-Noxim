/*
 * Noxim - the NoC Simulator
 *
 *
 *
 *
 * Creat by Amin Norollah @BALRUG (2017 Feb 1)
 * This file contains the declaration of the NoximVCState
 */

#ifndef __NoximVCState_H__
#define __NoximVCState_H__

#include <cassert>
#include <queue>
#include "NoximMain.h"
#define		SET_MODE	 0
#define		MINUS_MODE	 1
#define		PLUS_MODE	 2
using namespace std;

class NoximVCState {
public:

	NoximVCState	 ();
	void clear		 ();
	void global_state(const int in, const int vc, const int value);
	void routing	 (const int in, const int vc, const int value);
	void o			 (const int in, const int vc, const int value);
	void credit		 (const int in, const int vc, const int value, const int mode);
	void id			 (const int in, const int vc, const int value);

	int getOut		 (const int in, const int vc, const int value) const;


private:

	int G	[DIRECTIONS + 2]	[DEFAULT_NUM_VC];	//General state
	int R	[DIRECTIONS + 2]	[DEFAULT_NUM_VC];   //temp of routing
	int O	[DIRECTIONS + 2]	[DEFAULT_NUM_VC];	
	int C	[DIRECTIONS + 2]	[DEFAULT_NUM_VC];
	int ID	[DIRECTIONS + 2]	[DEFAULT_NUM_VC];
};

#endif
