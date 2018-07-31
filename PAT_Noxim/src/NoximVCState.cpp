/*
 * Noxim - the NoC Simulator
 *
 *
 *
 *
 * Creat by Amin Norollah @BALRUG (2017 Feb 1)
 * This file contains the implementation of the NoximVCState
 */

#include "NoximVCState.h"

NoximVCState::NoximVCState() {
	clear();
}

void NoximVCState :: clear() {
	for (int i = 0; i<DIRECTIONS + 2; i++)
		for (int j = 0; j < NoximGlobalParams::num_vcs; j++)
		{
			G	[i][j] = G_IDLE;
			R	[i][j] = 0;
			O	[i][j] = NOT_RESERVED;
			C	[i][j] = 0;
		}
}

void NoximVCState::global_state(const int in, const int vc, const int value) {
	if (value<3 && value>=-1)
		G[in][vc] = value;
}

void NoximVCState::routing(const int in, const int vc, const int value) {
	if (value<DIRECTIONS + 2 && value >= 0)
		R[in][vc] = value;
}

void NoximVCState::o(const int in, const int vc, const int value) {
	if (value<NoximGlobalParams::num_vcs && value >= -1)
		O[in][vc] = value;
}

void NoximVCState::credit(const int in, const int vc, const int value, const int mode) {
	switch (mode)
	{
	case 0: if (value >= 0 && value <= NoximGlobalParams::buffer_depth) C[in][vc] = value; break;
	case 1: if (C[in][vc]>0) C[in][vc]--;
			else assert(false); break;
	case 2:if (C[in][vc]<NoximGlobalParams::buffer_depth) C[in][vc]++; break;
	default: assert(false);
		break;
	}
}

void NoximVCState::id(const int in, const int vc, const int value) {
		ID[in][vc] = value;
}

int NoximVCState::getOut(const int in, const int vc, const int value) const {
	switch (value)
	{
	case G_VCS:
		return G[in][vc];
		break;
	case R_VCS:
		return R[in][vc];
		break;
	case O_VCS:
		return O[in][vc];
		break;
	case C_VCS:
		return C[in][vc];
		break;
	case ID_VCS:
		return ID[in][vc];
		break;
	}
}


 
