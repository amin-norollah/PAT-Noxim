#ifndef __NOXIMLOG_H__
#define __NOXIMLOG_H__

#include <systemc.h>
#include "NoximMain.h"
#include "NoximNoC.h"

using namespace std;

class NoximLog{

	public:
	void BufferLog          ( NoximNoC *n );
	void TrafficLog         ( NoximNoC *n );
	void TraceSignal        ( NoximNoC *n );
	void TraceEnd           (             );
	void PowerLog           (             );
	void PowerLogEnd        (             );
	void Throughput         (             );
	void ThroughputEnd      (             );
	void staticPowerLog     (             );
	void staticPowerLogEnd  (             );
	void get_error	  ();
	void get_errorend ();
	private:
	sc_trace_file *tf;
};

#endif
