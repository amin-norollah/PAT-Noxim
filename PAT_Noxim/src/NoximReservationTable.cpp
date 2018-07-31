/*
 * Noxim - the NoC Simulator
 *
 * (C) 2005-2010 by the University of Catania
 * For the complete list of authors refer to file ../doc/AUTHORS.txt
 * For the license applied to these sources refer to file ../doc/LICENSE.txt
 *
 * This file contains the implementation of the switch reservation table
 *
 *
 * Edited by Amin Norollah @BALRUG (2017 Feb 1)
 *
 */

#include "NoximReservationTable.h"

NoximReservationTable::NoximReservationTable()
{
    clear();
}

void NoximReservationTable::clear()
{
	/*
	rtable1.resize(DIRECTIONS + 2);
	rtable2.resize(DIRECTIONS + 2);
	for (int i = 0; i < DIRECTIONS + 2; ++i) {
		rtable[i].resize(NoximGlobalParams::num_vcs);
		*/
		// note that NOT_VALID entries should remain untouched
	for (int i = 0; i < DIRECTIONS + 2; i++) {
		for (int vc = 0; vc < NoximGlobalParams::num_vcs; vc++)
			if (rtable1[i][vc] != NOT_VALID) {
				rtable1[i][vc] = NOT_RESERVED;
				rtable2[i][vc] = NOT_RESERVED;
			}
	}					
	//}
}

bool NoximReservationTable::isAvailable(const int port_out ,const int vc_out) const
{
    assert	(port_out >= 0	 &&		 port_out < DIRECTIONS + 2	);
	assert	(vc_out   >= 0	 &&		 vc_out	  < NoximGlobalParams::num_vcs	);
	if (rtable1[port_out][vc_out] == NOT_RESERVED && rtable2[port_out][vc_out] == NOT_RESERVED) return (true);
	else return (false);
}

void NoximReservationTable::reserve(const int port_in, const int vc_input, const int port_out, const int vc_out)
{
    // reservation of reserved/not valid ports is illegal. Correctness
    // should be assured by NoximReservationTable users
    assert(isAvailable(port_out, vc_out));

    // check for previous reservation to be released
	int x = getOutputPort	(port_in, vc_input);
	int y = getOutputVc		(port_in, vc_input);

	if (x != NOT_RESERVED || y != NOT_RESERVED)
		release(x, y);

    rtable1	[port_out][vc_out]	=	port_in	;
	rtable2 [port_out][vc_out]	=	vc_input;
}

void NoximReservationTable::release(const int port_out, const int vc_out)
{
    assert(port_out >= 0 && port_out < DIRECTIONS + 2);
	assert(vc_out >= 0 && vc_out < NoximGlobalParams::num_vcs);
    // there is a valid reservation on port_out
    assert((rtable1[port_out][vc_out] >= NOT_RESERVED) && (rtable1[port_out][vc_out] < DIRECTIONS + 2	));
	assert((rtable2[port_out][vc_out] >= NOT_RESERVED) && (rtable2[port_out][vc_out] < NoximGlobalParams::num_vcs			));

    rtable1	[port_out][vc_out]	=	 NOT_RESERVED;
	rtable2 [port_out][vc_out]	=	 NOT_RESERVED;
}

int NoximReservationTable::getOutputPort(const int port_in, const int vc_in) const
{
    assert(port_in >= 0 && port_in < DIRECTIONS + 2);

	for (int i = 0; i < DIRECTIONS + 2; i++)
		for (int k = 0; k < NoximGlobalParams::num_vcs; k++)
			if (rtable1[i][k] == port_in && rtable2[i][k] == vc_in)
				return i;		// port_in reserved outport i

    // semantic: port_in currently doesn't reserve any out port
    return NOT_RESERVED;
}

int NoximReservationTable::getOutputVc(const int port_in, const int vc_in) const
{
	assert(port_in >= 0 && port_in < DIRECTIONS + 2);

	for (int i = 0; i < DIRECTIONS + 2; i++)
		for (int k = 0; k < NoximGlobalParams::num_vcs; k++)
			if (rtable1[i][k] == port_in && rtable2[i][k] == vc_in)
				return k;		// port_in reserved outport i

							// semantic: port_in currently doesn't reserve any out port
	return NOT_RESERVED;
}

// makes port_out no longer available for reservation/release
void NoximReservationTable::invalidate(const int port_out)
{
	for (int k = 0; k < NoximGlobalParams::num_vcs; k++)
	{
		rtable1[port_out][k] = NOT_VALID;
		rtable2[port_out][k] = NOT_VALID;
	}
}
