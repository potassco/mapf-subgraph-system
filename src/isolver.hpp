#pragma once

#include <unistd.h>

#include "instance.hpp"
#include "corridor_maker.hpp"

class ISolver
{
public:
	ISolver(std::string a, Instance* i, CorridorMaker* c, std::string wd, std::string sd, std::string af) : alg(a), inst(i), corr(c), work_dir(wd), stat_dir(sd), agent_file(af) {}; 
	virtual int Solve(int, int) = 0;
	virtual void PrintInstance(int, int) = 0;
	virtual int ReadResults(int, int) = 0;

	void ResetStat(int to)
	{
		timeout = to;
		total_runtime = 0;
		total_solvertime = 0;
	}

	std::string name;

protected:
	Instance* inst;
	CorridorMaker* corr;

	std::string alg;
	std::string work_dir;
	std::string stat_dir;
	std::string agent_file;

	float timeout;
	float total_runtime;
	float total_solvertime;
};

/*class AspSolver : public ISolver
{
public:
	using ISolver::ISolver;
	int Solve() {return 1;};
};*/