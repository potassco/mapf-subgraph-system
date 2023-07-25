#include "strategy.hpp"

using namespace std;

Strategy::Strategy(bool debug, bool print_path, bool no_solve, char c, string af, string bs, int to, string wd, string sd, string id, string md, string rd, string path)
{
	inst = new Instance(md, id, GetFilename(af), path);
	subg = new SubgraphMaker(inst);

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

	if (bs.compare("asp-mks") == 0)
	{
		sol = new AspSolver(debug, print_path, no_solve, alg, inst, subg, wd, sd, GetFilename(af), rd);
		sol->name = "asp-mks";
	}

	if (bs.compare("asp-soc") == 0)
	{
		sol = new AspSolver(debug, print_path, no_solve, alg, inst, subg, wd, sd, GetFilename(af), rd);
		sol->name = "asp-soc";
		subg->soc = true;
	}
}

Strategy::~Strategy()
{
	delete inst;
	delete sol;
	delete subg;
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
	size_t mks_LB = 0;
	size_t soc_LB = 0;
	size_t bonus_cost = 0;
	int result = 0;
	int p_expand = 1;

	if (B)
	{
		subg->computed_map = inst->map;
		subg->use_individual_maps = false;
	}

	while (result != 1) // 1 = timeout
	{ 
		if (result == 0) // ok result -> add new agents
		{
			bonus_cost = 0;
			number_of_agents_to_compute += 1;
			p_expand = 1;
			sol->ResetStat(timeout);

			if (number_of_agents_to_compute > inst->agents.size())
				break;

			mks_LB = inst->GetMksLB(number_of_agents_to_compute);
			soc_LB = inst->GetSocLB(number_of_agents_to_compute);

			if (M)
			{
				subg->ResetComputedMap(number_of_agents_to_compute);
				subg->PathsToMap(number_of_agents_to_compute);
				subg->ExpandMap(1, number_of_agents_to_compute, bonus_cost);
			}
			if (P || C)
			{
				subg->ResetComputedMap(number_of_agents_to_compute);
				subg->PathsToMap(number_of_agents_to_compute);
			}
		}

		if (result == -1) // -1 = no solution
		{
			if (B || M)
				bonus_cost++;
			if (P)
			{
				bool res = subg->ExpandMap(p_expand, number_of_agents_to_compute, bonus_cost);
				p_expand = p_expand * 2;

				if (!res) //if not expanded then add makespan and make max pruning
				{
					bonus_cost++;
					subg->ResetComputedMap(number_of_agents_to_compute);
					subg->PathsToMap(number_of_agents_to_compute);
					p_expand = 1;
				}
			}
			if (C)
			{
				bonus_cost++;
				subg->ExpandMap(1, number_of_agents_to_compute, bonus_cost);
			}
		}

		/* DEBUG - print details about the solved instance */

		cout << endl << endl;
		cout << "Solving agents file: " << inst->agents_file << endl;
		cout << "Current number of agents: " << number_of_agents_to_compute << endl;
		cout << "Strategy being used: " << alg << endl;
		cout << "Using " << sol->name << " solver" << endl; 
		cout << "Makespan lower bound: " << mks_LB << endl;
		cout << "Sum of costs lower bound: " << soc_LB << endl;
		cout << "Current delta used: " << bonus_cost << endl << endl;		

		inst->DebugPrint(subg->computed_map);

		/* END DEBUG */

		result = sol->Solve(number_of_agents_to_compute, bonus_cost); //if "OK" add agents, if "NO solution" execute strategy, if "Timed out" end
	}

	return 0;
}
