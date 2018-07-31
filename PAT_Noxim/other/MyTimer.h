#include<stdio.h>
#include<time.h>
#include<iostream>

using namespace std;

class MyTimer
{
	public:
		void start(){ time(&time_start);};
		void end(){ time(&time_end);}
		void print(){ period = (float)(time_end - time_start);
				cout<<"Time period = "<<period<<" s"<<endl;}
		void end_print(){ end();print();}
	private:
		time_t time_start;
		time_t time_end;
		float period;

};
