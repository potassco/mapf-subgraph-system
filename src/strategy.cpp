#include "strategy.hpp"

using namespace std;

Strategy::Strategy(char c, string af, string bs, int to, string wd, string sd, string id, string md)
{
	inst = new Instance(md, id, GetFilename(af));
	corr = new CorridorMaker(inst);

	timeout = to;

	B = false, M = false, P = false, C = false;
	switch (c)
	{
		case 'b':
			//cout << "Using Baseline strategy" << endl;
			B = true;
			alg = "b";
			break;
		case 'm':
			//cout << "Using MakespanAdd strategy" << endl;
			M = true;
			alg = "m";
			break;
		case 'p':
			//cout << "Using PruningCut strategy" << endl;
			P = true;
			alg = "p";
			break;
		case 'c':
			//cout << "Using Combined strategy" << endl;
			C = true;
			alg = "c";
			break;
		default:
			cout << "Wrong strategy!" << endl;
	}

	if (bs.compare("sat") == 0)
	{
		sol = new SatSolver(alg, inst, corr, wd.append("/sat"), sd, GetFilename(af));
		sol->name = "sat";
	}

	if (bs.compare("asp") == 0)
	{
		sol = new AspSolver(alg, inst, corr, wd.append("/asp"), sd, GetFilename(af));
		sol->name = "asp";
	}

	if (bs.compare("asp-teg") == 0)
	{
		sol = new AspSolver(alg, inst, corr, wd.append("/asp"), sd, GetFilename(af));
		sol->name = "asp-teg";
	}
}

Strategy::~Strategy()
{
	delete inst;
	delete sol;
	delete corr;
}

string Strategy::GetFilename(string s)
{
	size_t found = s.find_last_of('/');
	if (found != string::npos)
		return s.substr(found + 1);
	else
		return s;
}

int Strategy::RunTests()
{
	int number_of_agents_to_compute = 0;
	size_t LB = 0;
	size_t bonus_makespan = 0;
	int result = 0;
	int p_expand = 1;

	if (B)
		corr->computed_map = inst->map;

	while (result != 1) // 1 = timeout
	{ 
		if (result == 0) // ok result -> add new agents
		{
			bonus_makespan = 0;
			number_of_agents_to_compute += 5;
			p_expand = 1;
			sol->ResetStat(timeout);
			LB = inst->GetLB(number_of_agents_to_compute);

			if (number_of_agents_to_compute > inst->agents.size())
				break;

			if (M)
			{
				corr->ResetComputedMap();
				corr->PathsToMap(number_of_agents_to_compute);
				corr->ExpandMap(1, number_of_agents_to_compute, LB + bonus_makespan);
			}
			if (P || C)
			{
				corr->ResetComputedMap();
				corr->PathsToMap(number_of_agents_to_compute);
			}
		}

		if (result == -1) // -1 = no solution
		{
			if (B || M)
				bonus_makespan++;
			if (P)
			{
				bool res = corr->ExpandMap(p_expand, number_of_agents_to_compute, LB + bonus_makespan);
				p_expand = p_expand * 2;

				if (!res) //if not expanded then add makespan and make max pruning
				{
					bonus_makespan++;
					corr->ResetComputedMap();
					corr->PathsToMap(number_of_agents_to_compute);
					p_expand = 1;
				}
			}
			if (C)
			{
				bonus_makespan++;
				corr->ExpandMap(1, number_of_agents_to_compute, LB + bonus_makespan);
			}
		}

		/* DEBUG - print details about the solved instance */

		cout << endl << endl;
		cout << "Solving agents file: " << inst->agents_file << endl;
		cout << "Current number of agents: " << number_of_agents_to_compute << endl;
		cout << "Strategy being used: " << alg << endl;
		cout << "Using " << sol->name << " solver" << endl; 
		cout << "Makespan lower bound: " << LB << endl;
		cout << "Current makespan used: " << LB + bonus_makespan << endl << endl;

		inst->DebugPrint(corr->computed_map);

		/* END DEBUG */

		result = sol->Solve(number_of_agents_to_compute, LB + bonus_makespan); //if "OK" add agents, if "NO solution" execute strategy, if "Timed out" end
	}

	return 0;
}
