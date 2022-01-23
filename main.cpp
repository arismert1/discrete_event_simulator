#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include "randomc.h"
#include "mersenne.cpp"
constexpr auto T = 100000;

using namespace std;
const double NEVER = 1000000.0;

class Sim 
{
public:
	double meanTimeBetweenArrivals;
	double meanTimeToProduce;

	unsigned int queue;	//mege8os ouras
	unsigned int machine[3];	//0=idle, 1=working
	unsigned int slot;	//server indicator

	bool off_on; //switch
	double time;	//xronos prosomoiwsis

	vector<double> timesOfNextEvents;	//[0]=arrival, [1]=production
	unsigned int eventID;	//0=exe_arrival, 1=exe_production

	double meanTimeA;	//pragmatikos mesos xronos anamesa stis afikseis
	double meanTimeP;	//pragmatikos mesos xronos paragogis
	double meanTime;	//mege8os xronikis monadas

	Sim(double s_meanTimeBetweenArrivals, double s_meanTimeToProduce, TRandomMersenne* rand_gen)
	{
		meanTimeBetweenArrivals = s_meanTimeBetweenArrivals;
		meanTimeToProduce = s_meanTimeToProduce;
		queue = 0;
		slot = 0;
		machine[0] = 0; machine[1] = 0; machine[2] = 0;
		off_on = 0; 
		time = 0.0;
		timesOfNextEvents.resize(2);
		timesOfNextEvents[0]= (-1.0) * log(rand_gen->Random()) * meanTimeBetweenArrivals;
		//timesOfNextEvents[0] = time + exp(rand_gen->Random()) * meanTimeBetweenArrivals;
		timesOfNextEvents[1] = NEVER;
		meanTimeA = 0.0;
		meanTimeP = 0.0;
		meanTime = 0;
	}

	void findTimeOfNextEvent() 
	{
		time = (*min_element(timesOfNextEvents.begin(), timesOfNextEvents.end()));
		for (unsigned int i = 0; i < timesOfNextEvents.size(); i++)
		{
			if (timesOfNextEvents[i] == time)
				eventID = i;
		}
	}

	void executeArrival() 
	{
		queue++;
	}

	void executeProduction() 
	{
		machine[slot] = 0;
	}

	void eventExe() 
	{
		if (eventID == 0) executeArrival();
		if (eventID == 1) executeProduction();
	}

	void controller() 
	{
		for (unsigned int a = 0; a < 3; a++) 
		{
			if (queue > 0 && machine[a] == 0)
			{
				queue--;
				machine[a] = 1;
				off_on = 1;
				slot = 1;
				break;
			}
		}
	}

	void updateTimesOfNextEvents(TRandomMersenne* arrival_process, TRandomMersenne* production_process) 
	{
		if (eventID == 0)
			timesOfNextEvents[0] = time + ((-1.0) * log(arrival_process->Random()) * meanTimeBetweenArrivals);
			//timesOfNextEvents[0] = time + exp(arrival_process->Random()) * meanTimeBetweenArrivals;

		if (off_on)
		{
			timesOfNextEvents[1] = time + ((-1.0) * log(production_process->Random()) * meanTimeToProduce);
			//timesOfNextEvents[1] = time + exp(production_process->Random())*meanTimeToProduce;
			off_on = 0;
		}

		if (eventID == 1 && machine[slot] == 0)
			timesOfNextEvents[1] = NEVER;
	}
};

int main() 
{
	int32 seed1 = 123;
	int32 seed2 = 456;
	TRandomMersenne arrival_process(seed1);
	TRandomMersenne production_process(seed2);

	vector<double> tempArrival;
	vector<double> tempProduction;
	vector<double> tempTime;

	int countA = 0;
	int countP = 0;
	unsigned int i = 0; 
	Sim model(1.0/2.5,1.0/3,&arrival_process);
	//cout << "eventID " << "Queue	"<< "TONE[0]		" << "TONE[1]		"<<"Time	"<< endl;
	while (model.time<T) 
	{
		model.findTimeOfNextEvent();
		model.eventExe();
		model.controller();
		model.updateTimesOfNextEvents(&arrival_process, &production_process);

		tempArrival.push_back(model.timesOfNextEvents[0]);
		tempProduction.push_back(model.timesOfNextEvents[1]);
		tempTime.push_back(model.time);

		if (model.eventID == 1 && tempProduction[i] != 1e+06)
		{
			model.meanTimeP = model.meanTimeP + (tempProduction[i] - tempProduction[i-1]);
			countP++;
		}
		if (model.eventID == 0 && i>0)
		{
			model.meanTimeA = model.meanTimeA + (tempArrival[i]- tempArrival[i-1]);
			countA++;
		}
		if (i > 0)
			model.meanTime = model.meanTime + (tempTime[i] - tempTime[i - 1]);

		//cout << model.eventID << "	" << model.queue << "	" << tempArrival[i] << "		" << tempProduction[i] << "		" << model.time << endl;
		i++;
	}
	model.meanTimeA = model.meanTimeA / countA;
	model.meanTimeP = model.meanTimeP / countP;
	model.meanTime = model.meanTime / T;
	//cout <<"meanArrival=	"<<model.meanTimeA << "	" << model.meanTimeP <<"	meanTime=	"<<model.meanTime<< endl;

	double L; double W;
	double meanLamda; double meanMi;
	meanMi =1.0/model.meanTimeA;
	meanLamda = 1.0/model.meanTimeP;
	double Po;

	//TELIKES EKSISWSEIS
	Po = 1 / (1+(meanLamda/meanMi)+((0.5*meanLamda*meanLamda)/(meanMi*meanMi))+(meanLamda*meanLamda*meanLamda/(6*meanMi*meanMi*meanMi))*((3*meanMi)/(3*meanMi)-meanLamda));
	L = ((meanLamda* meanLamda* meanLamda* meanLamda*Po)/(2*(meanMi*meanMi)*((3*meanMi)-meanLamda)* ((3 * meanMi) - meanLamda)))+(meanLamda / meanMi);
	W = L / meanLamda;
	//END EKSISWSEIS

	cout << "lamda=	"<<meanLamda <<"	mi=	"<< meanMi << endl;
	cout << "L=	" << L << "	W=	" << W << endl;
	cout << "Xronos Prosomoiwsis	" << model.time << endl;
	return 0;
}