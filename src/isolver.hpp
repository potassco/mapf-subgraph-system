#pragma once

#include <unistd.h>

#include "instance.hpp"
#include "subgraph_maker.hpp"

class ISolver
{
public:
	ISolver(bool d, std::string a, Instance* i, SubgraphMaker* c, std::string wd, std::string sd, std::string af, std::string rd) : debug(d) ,alg(a), inst(i), subg(c), work_dir(wd), stat_dir(sd), agent_file(af), run_dir(rd) {}; 
	virtual int Solve(int, int) = 0;
	virtual void PrintInstance(int, int) = 0;
	virtual int ReadResults(int, int) = 0;

	void ResetStat(int to)
	{
		timeout = to;
		total_runtime = 0;
		total_solvertime = 0;

		solver_call = 0;

		choices_vc.clear();
		conflicts_vc.clear();
		variables_vc.clear();
		constraints_vc.clear();
	}

	std::string name;

protected:
	Instance* inst;
	SubgraphMaker* subg;
	bool debug;

	std::string alg;
	std::string work_dir;
	std::string stat_dir;
	std::string agent_file;
	std::string run_dir;

	std::string io_file_name;
	std::string stat_file_name;

	float timeout;
	float total_runtime;
	float total_solvertime;

	int solver_call;

	std::vector<int> choices_vc;
	std::vector<int> conflicts_vc;
	std::vector<int> variables_vc;
	std::vector<int> constraints_vc;
};

