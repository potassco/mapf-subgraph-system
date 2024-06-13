#include "asp_inc_solver.hpp"

using namespace std;

void AspIncSolver::ResetStat(int to)
{
	if (ctl != NULL)
		delete ctl;
	timeout = to;
	total_runtime = 0;
	total_solvertime = 0;

	solver_call = 0;
	first_time = true;

	variables_vc.clear();
	constraints_vc.clear();
}


int AspIncSolver::Solve(int agent_number, int bonus_cost)
{
	if (timeout < 0)
		return 1;

	stat_file_name = "statistics/asp_results.res";
	solver_call++;

	if (first_time)
	{
		// clingo init
		ctl = new Clingo::Control{{"--opt-strategy=usc"}};
		io_file_name.clear();
		//ctl->load(io_file_name.append(work_dir + "/ASP-INC/encodings/solver.lp").c_str());

		// store the map as previous map
		// TODO

		first_time = false;
	}

	PrintInstance(agent_number, bonus_cost);

	if (no_solve) // do not call solver, just assume success
		return 0;

	bool solved = false;
	//ctl->ground({{"base", {}}});
	//for(auto& m : ctl->solve())
	{
		// found solution
		cout << "found solution" << endl;
		solved = true;
	}

	return ReadResults(agent_number, bonus_cost);
}

void AspIncSolver::PrintInstance(int agent_number, int bonus_cost)
{
	Clingo::Backend bck = ctl->backend();

	// add terms
}

int AspIncSolver::ReadResults(int agent_number, int bonus_cost)
{
	// stats
	/*total_calls_ += 1;
	total_time_ += ctl.statistics()["summary"]["times"]["total"];
	total_solve_time_ += ctl.statistics()["summary"]["times"]["solve"];
	total_choices_ += ctl.statistics()["solving"]["solvers"]["choices"];
	total_conflicts_ += ctl.statistics()["solving"]["solvers"]["conflicts"];*/
	return 0;
}
