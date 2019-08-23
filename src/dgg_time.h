#ifndef _DGG_TIME_H_
#define _DGG_TIME_H_

#include <Windows.h>
#include <ctime>
#include <string>
using std::string;

class ElapasedTime{
	double PCFreq;
	__int64 CounterStart;
	double previous_time;
	string prev_str;
public:
	ElapasedTime(bool startNow = true){
		if( startNow == false){
			PCFreq = 0.0;
			CounterStart = 0.0;
			return;
		}
		LARGE_INTEGER li;
		if( !QueryPerformanceFrequency(&li) ){
			fprintf(stderr,  "QueryPerformanceFrequency failed!\n");
			return;
		}
		PCFreq = double(li.QuadPart) / 1000.0;
		//printf( "inipcfreq %lf\n" , (double)PCFreq );
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
		previous_time = 0;
		prev_str = "";
	}
	void start(){
		LARGE_INTEGER li;
		if( !QueryPerformanceFrequency(&li) ){
			fprintf(stderr,  "QueryPerformanceFrequency failed!\n");
			return;
		}
		PCFreq = double(li.QuadPart) / 1000.0;
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
		previous_time = 0;
	}
	double getTime(){
	    LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return double(li.QuadPart-CounterStart)/PCFreq/1000.0;
	}
#if 0
	void printEstimateTime(double time_interval, double percent) {//percent in 0-1
		double current_time = getTime();
		double past_time = current_time - previous_time;
		if (past_time > time_interval) {
			previous_time = current_time;
			double remain_time = current_time / percent * (1 - percent);
			if (remain_time > 3600) {
				int hour = remain_time / 3600.;
				double seconds = remain_time - hour * 3600.;
				printf("Computed %.0lf percent, time %lf s, estimate_remain_time %d h %lf s\r",
					percent * 100.0, current_time, hour, seconds );
			} else{
				printf("Computed %.0lf percent, time %lf s, estimate_remain_time %lf s\r",
					percent * 100.0, current_time, remain_time);
			}
		}
	}
#else
	void printEstimateTime(double time_interval, double percent) {//percent in 0-1
		double current_time = getTime();
		double past_time = current_time - previous_time;
		if (past_time > time_interval) {
			string wp_str = "Windows Propagation: ";
			int pc = percent*100.0;
			char t_pc_str[4];
			sprintf(t_pc_str, "%d", pc);
			string pc_str(t_pc_str);
			pc_str += "% ";

			string tetl_str = "Time Elapsed: ";
			char t_tm_str[20];
			sprintf(t_tm_str, "%.2lf s", current_time);
			string tm_str(t_tm_str);
			previous_time = current_time;

			string curr_str = wp_str + pc_str + tetl_str + tm_str;
			for (int i = 0; i < prev_str.size(); i++) {
				printf("\b \b");
			}
			printf("%s", curr_str.c_str());
			prev_str = curr_str;
		}
	}

	//void printEstimateTime(double time_interval, double percent) {//percent in 0-1
	//	double current_time = getTime();
	//	double past_time = current_time - previous_time;
	//	if (past_time > time_interval) {
	//		//string wp_str = "Windows Propagation: ";
	//		int pc = percent*100.0;
	//		char t_pc_str[4];
	//		sprintf(t_pc_str, "%d", pc);
	//		string pc_str(t_pc_str);
	//		pc_str += "%";

	//		previous_time = current_time;

	//		string curr_str = pc_str ;
	//		for (int i = 0; i < prev_str.size(); i++) {
	//			printf("\b \b");
	//		}
	//		printf("%s", curr_str.c_str());
	//		prev_str = curr_str;
	//	}
	//}
#endif

	void printTime(string tmpMessage = ""){
	    LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		//printf( "li %lf\n" , (double)li.QuadPart);
		//printf( "pcfreq %lf\n" , PCFreq );
		//fz
		//if( tmpMessage.size() != 0 ) 
		//	tmpMessage = tmpMessage +",";
		fprintf(stderr, "%s time %lf\n" ,  tmpMessage.c_str() , double(li.QuadPart-CounterStart)/PCFreq/1000.0 );
		return;
	}
};


#endif

