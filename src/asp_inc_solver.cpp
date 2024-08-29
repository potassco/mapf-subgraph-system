#include "asp_inc_solver.hpp"

using namespace std;

void AspIncSolver::ResetStat(int to)
{
    // specific for inc ASP
	if (ctl != NULL)
		delete ctl;
    added_timesteps.clear();

    // shared
	timeout = to;
	total_runtime = 0;
	total_solvertime = 0;

	solver_call = 0;

	variables_vc.clear();
	constraints_vc.clear();
}

int AspIncSolver::Solve(int agent_number, int bonus_cost)
{
	if (timeout < 0)
		return 1;

	stat_file_name = "statistics/asp_results.res";
	solver_call++;

	if (solver_call == 1)
	{
		// clingo init
		ctl = new Clingo::Control{{"--opt-strategy=usc", "--warn=none"}};
		io_file_name.clear();
		ctl->load(io_file_name.append(work_dir + "/ASP-INC/encodings/mapf-inc2.lp").c_str());
        io_file_name.clear();
        ctl->load(io_file_name.append(work_dir + "/ASP-INC/encodings/soc-bound.lp").c_str());
        CreateInitialInstance(agent_number);
        added_timesteps = vector<vector<AtomInfo> >(agent_number, vector<AtomInfo>(inst->number_of_vertices, {-1,-2}));
	}

	PrintInstance(agent_number, bonus_cost);

	if (no_solve) // do not call solver, just assume success
		return 0;

	solved = false;
	for(auto& m : ctl->solve((Clingo::SymbolicLiteralSpan)assumptions_vector))
	{
		// found solution
		//cout << "found solution" << endl;
		solved = true;
        //for(auto& a : m.symbols(Clingo::ShowType::Shown))
        //    cout << a.to_string() << endl;
	}

	return ReadResults(agent_number, bonus_cost);
}

void AspIncSolver::CreateInitialInstance(int agent_number)
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

        // agent horizon
        if (!name.compare("mks"))  // mks
        {
            auto horizon_atm = bck.add_atom(Clingo::Function("starting_horizon", {Clingo::Number(a), Clingo::Number(inst->GetMksLB(agent_number))}));
            bck.rule(false, {horizon_atm}, {});
        }
        if (!name.compare("soc"))       // soc
        {
            auto horizon_atm = bck.add_atom(Clingo::Function("starting_horizon", {Clingo::Number(a), Clingo::Number(inst->SP_lengths[a])}));
            bck.rule(false, {horizon_atm}, {});
        }
    }

    // vertices, edges
    for (size_t i = 0; i < inst->map.size(); i++)
        for (size_t j = 0; j < inst->map[i].size(); j++)
            if (inst->map[i][j] != -1)
            {
                auto xy_func = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("vertex", {xy_func}));
                bck.rule(false, {atm}, {});
            }

    for (size_t i = 0; i < inst->map.size(); i++)
    {
        for (size_t j = 0; j < inst->map[i].size(); j++)
        {
            if (inst->map[i][j] == -1)
                continue;
            auto xy_func_from = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
            
            /*{   // selfloop
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }*/
            if (i > 0 && inst->map[i-1][j] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
            if (i < inst->map.size() - 1 && inst->map[i+1][j] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+2), Clingo::Number(j+1)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
            if (j > 0 && inst->map[i][j-1] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
            if (j < inst->map[i].size() - 1 && inst->map[i][j+1] != -1)
            {
                auto xy_func_to = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+2)});
                auto atm = bck.add_atom(Clingo::Function("edge", {xy_func_from, xy_func_to}));
                bck.rule(false, {atm}, {});
            }
        }
    }

    ctl->ground({{"base", {}}});
}

void AspIncSolver::PrintInstance(int agent_number, int bonus_cost)
{
	Clingo::Backend bck = ctl->backend();
    assumptions_vector.clear();

    // vertex is not in the subgraph, check if it was reachable before
    for (size_t i = 0; i < subg->computed_map.size(); i++)
    {
        for (size_t j = 0; j < subg->computed_map[i].size(); j++)
        {
            if (subg->computed_map[i][j] == -1 && inst->map[i][j] != -1)
            {
                int v_ID = inst->map[i][j];
                for (int a = 0; a < agent_number; a++)
                {
                    for (int t = added_timesteps[a][v_ID].first_timestep; t <= added_timesteps[a][v_ID].last_timestep; t++)
                    {
                        auto xy_reach_func = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
                        Clingo::SymbolicLiteral lit(Clingo::Function("at", {Clingo::Number(a), xy_reach_func, Clingo::Number(t)}), false);
                        assumptions_vector.push_back(lit);

                        //cout << "remove agent " << a << " in " << i << ", " << j << " at timestep " << t << endl;
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < subg->computed_map.size(); i++)
    {
        for (size_t j = 0; j < subg->computed_map[i].size(); j++)
        {
            // vertex is in the subgraph, add it as reachable
            if (subg->computed_map[i][j] != -1)
            {
                for (int a = 0; a < agent_number; a++)
                {
                    int max_t = 0;
                    if (!name.compare("mks"))
                        max_t = inst->GetMksLB(agent_number) + bonus_cost;
                    if (!name.compare("soc"))
                        max_t = inst->SP_lengths[a] + bonus_cost;

                    vector<pair<int,int> > vc = GetTRange(agent_number, bonus_cost, a, {int(i), int(j)}, max_t, bonus_cost);

                    for (size_t part = 0; part < vc.size(); part++)
                    {
                        for (int t = vc[part].first; t <= vc[part].second; t++)
                        {
                            auto xy_reach_func = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
                            auto atm = bck.add_atom(Clingo::Function("reach", {Clingo::Number(a), xy_reach_func, Clingo::Number(t), Clingo::Number(solver_call-1)}));
                            bck.rule(false, {atm}, {});

                            //cout << "add agent " << a << " in " << i << ", " << j << " at timestep " << t << endl;
                        }
                    }
                }
            }
        }
    }

    auto delta_cnum = Clingo::Number(bonus_cost);
    auto k_cnum = Clingo::Number(solver_call-1);

	ctl->ground({
                    {"time", {delta_cnum, k_cnum}},
                    {"mapf", {k_cnum}},
                    {"check", {delta_cnum, k_cnum}}
                });
    
    if (!name.compare("soc"))
        ctl->ground({
                        {"optimize", {delta_cnum, k_cnum}},
                    });

}

int AspIncSolver::ReadResults(int agent_number, int bonus_cost)
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
        output << "asp-inc" << "\t"
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

vector<pair<int,int> > AspIncSolver::GetTRange(int agent_number, int bonus_cost, int a, Vertex v, int max_t, int k)
{
    int v_ID = inst->map[v.x][v.y];
    int earliest = inst->length_from_start[a][v_ID];
    int first_timestep_used = added_timesteps[a][v_ID].first_timestep;
    int latest = max_t - inst->length_from_goal[a][v_ID];
    int last_timestep_used = added_timesteps[a][v_ID].last_timestep;

    vector<pair<int,int> > return_vc;

    if (!name.compare("soc"))   // the vertex is someone's goal, it is not reachable after they reach it
    {
        bool is_goal = false;
        for (int other_ag = 0; other_ag < agent_number; other_ag++)
        {
            if (other_ag == a)
                continue;
            if (inst->agents[other_ag].goal == v)
                latest = min(inst->SP_lengths[other_ag] + bonus_cost - 1, latest);
        }
    }

    if (earliest > latest)  // the vertex is not reachable
    {
        return return_vc;
    }

    if (first_timestep_used == -1)   // this vertex is being added for the first time
    {
        added_timesteps[a][v_ID].first_timestep = earliest;
        added_timesteps[a][v_ID].last_timestep = latest;

        return_vc.push_back({earliest, latest});
        return return_vc;
    }

    if (first_timestep_used < earliest)
    {
        cout << "this should not happen" << endl;
        return return_vc;
    }

    if (last_timestep_used > latest)
    {
        cout << "this should not happen" << endl;
        return return_vc;
    }

    if (first_timestep_used > earliest) // should not happen with the current strategies
    {
        cout << "this should not happen with the current strategies" << endl;
        added_timesteps[a][v_ID].first_timestep = earliest;
        return_vc.push_back({earliest, first_timestep_used-1});
    }

    if (last_timestep_used < latest)
    {
        added_timesteps[a][v_ID].last_timestep = latest;
        return_vc.push_back({last_timestep_used + 1, latest});
    }

    return return_vc;
}
