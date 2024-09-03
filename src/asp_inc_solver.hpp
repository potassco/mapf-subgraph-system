#pragma once

#include <numeric>
#include <math.h>

#include "../solvers/ASP-INC/clingo.hh"

#include "isolver.hpp"

// info about vertices added as rules
struct AtomInfo
{
	int first_timestep;
	int last_timestep;
};

class AspIncSolver : public ISolver
{
public:
	using ISolver::ISolver;
	void ResetStat(int);
	int Solve(int, int);

private:
	void PrintInstance(int, int);
	int ReadResults(int, int);
	void CreateInitialInstance(int);

	std::vector<std::pair<int,int> > GetTRange(int, int, int, Vertex, int, int);

	std::vector<std::vector<AtomInfo> > added_timesteps; //added_timesteps[agent][vertex_ID]
	Clingo::Control* ctl = NULL;
	std::vector<Clingo::SymbolicLiteral> assumptions_vector;
	int GetDistance(Vertex, Vertex);

	bool solved;
};