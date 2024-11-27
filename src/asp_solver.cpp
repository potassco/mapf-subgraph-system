#include "asp_solver.hpp"

using namespace std;

int AspSolver::Solve(int agent_number, int bonus_cost)
{
	if (timeout < 0)
		return 1;

	stat_file_name = "statistics/asp_results.res";
	solver_call++;

	ctl = new Clingo::Control{{"--opt-strategy=usc", "--warn=none"}};
	io_file_name.clear();
	ctl->load(io_file_name.append(work_dir + "/ASP/soc-bound.lp").c_str());
	io_file_name.clear();
	ctl->load(io_file_name.append(work_dir + "/ASP/solver.lp").c_str());

	PrintInstance(agent_number, bonus_cost);

	solved = false;
    bool ended = false;
	thread waiting_thread = thread(WaitForTerminate, timeout*1000, ctl, ref(ended));

	for(auto& m : ctl->solve())
	{
		// found solution
		cout << "found solution" << endl;
		solved = true;
        for(auto& a : m.symbols(Clingo::ShowType::Shown))
            cout << a.to_string() << endl;
	}
    ended = true;
    waiting_thread.join();

	return ReadResults(agent_number, bonus_cost);
}

void AspSolver::PrintInstance(int agent_number, int bonus_cost)
{
	Clingo::Backend bck = ctl->backend();

	// add all terms

    // agents, start, goal
    for (int a = 0; a < agent_number; a++)
    {
        // start
        auto sxy_func = Clingo::Function("", {Clingo::Number(inst->agents[a].start.x + 1), Clingo::Number(inst->agents[a].start.y + 1)});
        auto start_atm = bck.add_atom(Clingo::Function("start", {Clingo::Number(a), sxy_func}));
        bck.rule(false, {start_atm}, {});

        // goal
        auto gxy_func = Clingo::Function("", {Clingo::Number(inst->agents[a].goal.x + 1), Clingo::Number(inst->agents[a].goal.y + 1)});
        auto goal_atm = bck.add_atom(Clingo::Function("goal", {Clingo::Number(a), gxy_func}));
        bck.rule(false, {goal_atm}, {});

        // agent
        auto agent_atm = bck.add_atom(Clingo::Function("agent", {Clingo::Number(a)}));
        bck.rule(false, {agent_atm}, {});
    }

	// agent horizon
	if (!name.compare("mks"))  // mks
	{
		auto mks_atm = bck.add_atom(Clingo::Function("mks", {Clingo::Number(inst->GetMksLB(agent_number) + bonus_cost)}));
		bck.rule(false, {mks_atm}, {});
	}

	if (!name.compare("soc"))       // soc
	{
		for (int a = 0; a < agent_number; a++)
		{
			auto horizon_atm = bck.add_atom(Clingo::Function("sp_length", {Clingo::Number(a), Clingo::Number(inst->SP_lengths[a])}));
			bck.rule(false, {horizon_atm}, {});
		}
	}

	// delta
	auto delta_atm = bck.add_atom(Clingo::Function("delta", {Clingo::Number(bonus_cost)}));
    bck.rule(false, {delta_atm}, {});

    // vertices, edges
    for (size_t i = 0; i < subg->computed_map.size(); i++)
        for (size_t j = 0; j < subg->computed_map[i].size(); j++)
            if (subg->computed_map[i][j] != -1)
            {
                auto xy_func = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("vertex", {xy_func}));
                bck.rule(false, {atm}, {});
            }

    for (size_t i = 0; i < subg->computed_map.size(); i++)
    {
        for (size_t j = 0; j < subg->computed_map[i].size(); j++)
        {
            if (subg->computed_map[i][j] == -1)
                continue;
            auto xy_func_from = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
            
            /*{   // selfloop
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }*/
            if (i > 0 && subg->computed_map[i-1][j] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
            if (i < subg->computed_map.size() - 1 && subg->computed_map[i+1][j] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+2), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
            if (j > 0 && subg->computed_map[i][j-1] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
            if (j < subg->computed_map[i].size() - 1 && subg->computed_map[i][j+1] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+2)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
        }
    }

	// reach states
    for (size_t i = 0; i < subg->computed_map.size(); i++)
    {
        for (size_t j = 0; j < subg->computed_map[i].size(); j++)
        {
			Vertex v = {int(i),int(j)};
            // vertex is in the subgraph, add it as reachable
            if (subg->computed_map[i][j] != -1)
            {
                for (int a = 0; a < agent_number; a++)
                {
                    pair<int,int> reach_times;
                    if (!name.compare("mks"))
                        reach_times = subg->GetReachTimesMKS(a, agent_number, v, bonus_cost);
                    if (!name.compare("soc"))
						reach_times = subg->GetReachTimesSOC(a, agent_number, v, bonus_cost);

					for (int t = reach_times.first; t <= reach_times.second; t++)
					{
						auto xy_reach_func = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
						auto reach_atm = bck.add_atom(Clingo::Function("reach", {Clingo::Number(a), xy_reach_func, Clingo::Number(t)}));
						bck.rule(false, {reach_atm}, {});

						//cout << "add agent " << a << " in " << i << ", " << j << " at timestep " << t << endl;
					}
                }
            }
        }
    }

    if (!name.compare("soc"))
        ctl->ground({
						{"mapf", {}},
						{"optimize", {}},
                        {"sum_of_cost", {}},
                    });
	
	if (!name.compare("mks"))
        ctl->ground({
						{"mapf", {}},
						{"makespan", {}},
                    });
}

int AspSolver::ReadResults(int agent_number, int bonus_cost)
{
	// stats
	auto stat = ctl->statistics();

    float solver_time = stat["summary"]["times"]["solve"];
	float total_time = stat["summary"]["times"]["total"];
    variables_vc.push_back(stat["problem"]["generator"]["vars"]);
    constraints_vc.push_back(stat["problem"]["generator"]["constraints"]);

    total_runtime += total_time;
    total_solvertime += solver_time;
    timeout -= total_time;

    if (timeout < 0)
        return 1;

    if (!solved)
        return -1;

    ofstream output;
    output.open(stat_file_name, ios::app);
    if (output.is_open())
    {
        output << "asp-vanilla" << "\t"
            << inst->agents_file << "\t"
            << inst->path_type << "\t"
            << name << "\t"
            << alg << "\t"
            << agent_number << "\t"
            << inst->number_of_vertices << "\t"
            << inst->GetMksLB(agent_number) << "\t"
            << inst->GetSocLB(agent_number) << "\t"
            << subg->GiveNewNumbering() << "\t"
            << bonus_cost << "\t"
            << inst->path_compute_time << "\t"
            << int(1000*total_solvertime) << "\t"
            << int(1000*total_runtime) << "\t"
            << solver_call << "\t"
            // variables
            << accumulate(variables_vc.begin(), variables_vc.end(), 0) << "\t"
            // constraints
            << accumulate(constraints_vc.begin(), constraints_vc.end(), 0) << "\t"
            << endl;

        output.close();
    }
    else
        cout << "Could not open results file!" << endl;

	return 0;
}

void AspSolver::WaitForTerminate(int time_left_ms, Clingo::Control* ctl, bool& ended)
{
	while (time_left_ms > 0)
	{
		if (ended)
			return;

		this_thread::sleep_for(std::chrono::milliseconds(50));
		time_left_ms -= 50;
	}

	if (ended)
		return;
		
	ctl->interrupt();	// Trusting in clingo implementation
}