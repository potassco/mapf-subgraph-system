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
		ctl->load(io_file_name.append(work_dir + "/ASP-INC/encodings/solver.lp").c_str());

		// store the map as previous map
		// TODO

		first_time = false;
	}

	PrintInstance(agent_number, bonus_cost);

	if (no_solve) // do not call solver, just assume success
		return 0;

	bool solved = false;
	/*ctl->ground({{"base", {}}});
	for(auto& m : ctl->solve())
	{
		// found solution
		cout << "found solution" << endl;
		solved = true;
	}*/

	return ReadResults(agent_number, bonus_cost);
}

void AspIncSolver::PrintInstance(int agent_number, int bonus_cost)
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

    // reachability mks - TODO
    if (!name.compare("makespan-inc"))
    {
        for (size_t i = 0; i < subg->computed_map.size(); i++)
            for (size_t j = 0; j < subg->computed_map[i].size(); j++)
                if (subg->computed_map[i][j] != -1)
                {
                    for (int a = 0; a < agent_number; a++)
                    {
                        for (int t = 0; t < inst->GetMksLB(agent_number) + bonus_cost; t++)
                        {
                            auto xy_reach_func = Clingo::Function("", {Clingo::Number(i+1), Clingo::Number(j+1)});
                            auto atm = bck.add_atom(Clingo::Function("reach", {Clingo::Number(a), xy_reach_func, Clingo::Number(t)}));
                            bck.rule(false, {atm}, {});
                        }
                    }
                }
    }

    // reachability soc - TODO
    if (!name.compare("soc-inc"))
    {
        
    }

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
}

int AspIncSolver::ReadResults(int agent_number, int bonus_cost)
{
	// stats
	auto stat = ctl->statistics();
	/*total_calls_ += 1;
	total_time_ += ctl.statistics()["summary"]["times"]["total"];
	total_solve_time_ += ctl.statistics()["summary"]["times"]["solve"];
	total_choices_ += ctl.statistics()["solving"]["solvers"]["choices"];
	total_conflicts_ += ctl.statistics()["solving"]["solvers"]["conflicts"];*/

    cout << ctl->statistics()["summary"]["times"]["total"] << endl;
    cout << ctl->statistics()["summary"]["times"]["solve"] << endl;
    cout << ctl->statistics()["solving"]["solvers"]["choices"] << endl;
    cout << ctl->statistics()["solving"]["solvers"]["conflicts"] << endl;

	return 0;
}





/*

void MAPFPlanner::add_instance(Clingo::Backend& bck, unsigned int delta){
    
    std::unordered_set<Node*> vertices;
    std::unordered_map <unsigned int, std::unordered_set<Node*> > agent_vertices;
    std::unordered_map<Node*,std::unordered_set<Node*>> edges;

    for (int agent_id = 0; agent_id < env->num_of_agents; agent_id++){
        directional_pos start = get_start(agent_id);
        directional_pos goal = get_goal(agent_id);

        // add start, goal and agent facts
        auto start_xy = map.deserialize_2d(start.first);
        auto xy_func = Clingo::Function("", {Clingo::Number(start_xy.first), Clingo::Number(start_xy.second)});
        auto dir_start = Clingo::Function("", {xy_func, Clingo::Number(start.second)});

        auto start_atm = bck.add_atom(Clingo::Function("start", {Clingo::Number(agent_id), dir_start}));

        // goal
        auto goal_xy = map.deserialize_2d(start.first);
        auto gxy_func = Clingo::Function("", {Clingo::Number(goal_xy.first), Clingo::Number(goal_xy.second)});
        auto dir_goal = Clingo::Function("", {gxy_func, Clingo::Number(start.second)});

        auto goal_atm = bck.add_atom(Clingo::Function("goal", {Clingo::Number(agent_id), dir_goal}));

        // agent
        auto agent_atm = bck.add_atom(Clingo::Function("agent", {Clingo::Number(agent_id)}));

        // add facts
        bck.rule(false, {start_atm}, {});
        bck.rule(false, {goal_atm}, {});
        bck.rule(false, {agent_atm}, {});

        // Add reach atoms for each agent
        auto reach = map.reachable_nodes_within_distance(PLAN_DIST, get_horizon(agent_id)+delta, start, goal);
        for (int time = 0 ; time < reach.size() ; time++){
            for (Node* node : reach[time]){
                agent_vertices[agent_id].insert(node);

                auto vertex = map.deserialize_2d(node->name.first);
                auto xy_reach_func = Clingo::Function("", {Clingo::Number(vertex.first), Clingo::Number(vertex.second)});
                auto dir_reach = Clingo::Function("", {xy_reach_func, Clingo::Number(node->name.second)});
                
                // reach atom
                auto atm = bck.add_atom(Clingo::Function("reach", {Clingo::Number(agent_id), dir_reach, Clingo::Number(time)}));
                bck.rule(false, {atm}, {});

                vertices.insert(node);
                std::vector<Node*> neighbors = node->out;
                neighbors.push_back(node);
                for (Node* neighbor : neighbors){
                    // if we are not in the last step and the neighbor is reachable in the next step
                    if (time < reach.size() - 1 &&  reach[time+1].count(neighbor) == 1){
                        if (edges.count(node) == 0){
                            edges[node] = std::unordered_set<Node*>();
                        }
                        edges[node].insert(neighbor);
                    }
                }
            }
        }
    }
    // now we add atoms for node and edge
    for (Node* node : vertices){
        auto vertex = map.deserialize_2d(node->name.first);
        
        auto xy_func = Clingo::Function("", {Clingo::Number(vertex.first), Clingo::Number(vertex.second)});
        auto dir_vertex = Clingo::Function("", {xy_func, Clingo::Number(node->name.second)});


        auto atm = bck.add_atom(Clingo::Function("vertex", {dir_vertex}));
        bck.rule(false, {atm}, {});
    }

    for ( auto& [agent_id, vertices] : agent_vertices){
        for (Node* node : vertices){
            auto vertex = map.deserialize_2d(node->name.first);
            
            auto xy_func = Clingo::Function("", {Clingo::Number(vertex.first), Clingo::Number(vertex.second)});
            auto dir_vertex = Clingo::Function("", {xy_func, Clingo::Number(node->name.second)});

            unsigned int dist = map.distance_between(get_goal(agent_id), node->name);

            auto atm = bck.add_atom(Clingo::Function("dist", {Clingo::Number(agent_id),dir_vertex, Clingo::Number(dist)}));
            bck.rule(false, {atm}, {});
        }
    }
    
    for (auto& [nodefrom, nodeto_vec] : edges){
        auto vertex_from = map.deserialize_2d(nodefrom->name.first);
        auto xy_func_from = Clingo::Function("", {Clingo::Number(vertex_from.first), Clingo::Number(vertex_from.second)});
        auto dir_func_from = Clingo::Function("", {xy_func_from, Clingo::Number(nodefrom->name.second)});

        for ( auto& nodeto : nodeto_vec){
            auto vertex_to = map.deserialize_2d(nodeto->name.first);
            auto xy_func_to = Clingo::Function("", {Clingo::Number(vertex_to.first), Clingo::Number(vertex_to.second)});
            auto dir_func_to = Clingo::Function("", {xy_func_to, Clingo::Number(nodeto->name.second)});

            auto atm = bck.add_atom(Clingo::Function("edge", {dir_func_from, dir_func_to}));
            bck.rule(false, {atm}, {});
        }
    }
}

void MAPFPlanner::solve(){

    std::unordered_map< unsigned int, std::vector<directional_pos> > positions;

    for (int i = 0; i < PLAN_DIST+1; i++){
        positions[i] = std::vector<directional_pos>(env->curr_states.size(), std::pair(0,Orientation(0)));
    }

    bool model_found = false;
    unsigned int delta = STARTING_DELTA;

    while (!model_found){
        Clingo::Control ctl{{"--opt-strategy=usc"}};
        Clingo::Backend bck = ctl.backend();

        add_instance(bck, delta);

        // add the encoding
        ctl.load("encodings/paths-turns-soc-fixed.lp");

        ctl.ground({{"base", {}}});
        for(auto& m : ctl.solve()){
            std::cout << "model found! Final delta is: " << delta << endl;
            // set to true to get out of the loop
            model_found = true;

            for(auto& atom : m.symbols()){
                // std::cout << atom << " ";
                if ( atom.match("at", 3) ){
                    // could only get the rest if time == 1 (the first move)
                    int time = atom.arguments()[2].number();
                    int agent_id = atom.arguments()[0].number();
                    int x = atom.arguments()[1].arguments()[0].arguments()[0].number();
                    int y = atom.arguments()[1].arguments()[0].arguments()[1].number();
                    int dir = atom.arguments()[1].arguments()[1].number();

                    positions[time][agent_id] =  std::make_pair(map.serialize_2d(x,y), Orientation(dir));                    
                    
                }
            }
        }
        // std::cout << endl;
        delta += 1;

        // stats
        total_calls_ += 1;
        total_time_ += ctl.statistics()["summary"]["times"]["total"];
        total_solve_time_ += ctl.statistics()["summary"]["times"]["solve"];
        total_choices_ += ctl.statistics()["solving"]["solvers"]["choices"];
        total_conflicts_ += ctl.statistics()["solving"]["solvers"]["conflicts"];
    }

    // populate the moves vector
    for (int time = 0; time < PLAN_KEEP; time++){
        std::vector<Action> actions = std::vector<Action>(env->curr_states.size(), Action::W);
        for (int agent_id = 0; agent_id < env->curr_states.size(); agent_id++){
            actions[agent_id] = convert_action(positions[time][agent_id], positions[time+1][agent_id]);
        }
        computed_plans_.push_back(actions);
    }

    std::cout << "total calls: " << total_calls_ << std::endl;
    std::cout << "total time: " << total_time_ << std::endl;
    std::cout << "total solve time: " << total_solve_time_ << std::endl;
    std::cout << "total choices: " << total_choices_ << std::endl;
    std::cout << "total conflicts: " << total_conflicts_ << std::endl;

}

*/