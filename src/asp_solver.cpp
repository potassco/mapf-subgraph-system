#include "asp_solver.hpp"

using namespace std;

int AspSolver::Solve(int agent_number, int bonus_cost)
{
	if (timeout < 0)
		return 1;

	solver_call++;

	io_file_name.clear();
	if (debug)
		io_file_name.append(inst->agents_file + "_" + alg + "_" + name + "_" + to_string(agent_number) + "_" + to_string(solver_call));
	else
		io_file_name.append(inst->agents_file + "_" + alg + "_" + name);

	stat_file_name.clear();
	stat_file_name.append(inst->agents_file + "_" + alg + "_" + name);

	subg->GiveNewNumbering();

	PrintInstance(agent_number, bonus_cost);

	string objective;
	if (name.compare("asp-mks") == 0)
		objective = "mks";
	if (name.compare("asp-soc") == 0)
		objective = "soc";

	stringstream exec;
	exec << "OBJECTIVE=" << objective << " INSTANCE=\"" << run_dir << "/" << io_file_name << ".lp\" timeout " << (int)timeout + 1 << " " << work_dir << "/plan.sh > " << run_dir << "/" << io_file_name << ".out";

	if (no_solve) // do not call solver, just assume success
		return 0;

	system(exec.str().c_str());

	return ReadResults(agent_number, bonus_cost);
}

void AspSolver::PrintInstance(int agent_number, int bonus_cost)
{
	ofstream asp;
	string ofile = run_dir;
	asp.open(ofile.append("/" + io_file_name + ".lp"));
	if (asp.is_open())
	{
		asp << "makespan(" << inst->GetMksLB(agent_number) + bonus_cost << ")." << endl;

		if (name.compare("asp-soc") == 0)
		{
			for (int i = 0; i < agent_number; i++)
				asp << "makespan(" << i + 1 << "," << inst->SP_lengths[i] + bonus_cost << ")." << endl;
			
			asp << "delta(" << bonus_cost << ")." << endl;
		}

		for (size_t i = 0; i < subg->computed_map.size(); i++)
			for (int j = 0; j < subg->computed_map[i].size(); j++)
				if (subg->computed_map[i][j] != -1)
					asp << "node(" << i + 1 << "," << j + 1 << "). ";
		asp << endl;

		for (int i = 0; i < agent_number; i++)
			asp << "start(" << i + 1 << "," << inst->agents[i].start.x + 1 << "," << inst->agents[i].start.y + 1 << "). ";
		asp << endl;

		for (int i = 0; i < agent_number; i++)
			asp << "goal(" << i + 1 << "," << inst->agents[i].goal.x + 1 << "," << inst->agents[i].goal.y + 1 << "). ";
		asp << endl;

		if (name.compare("asp-mks") == 0)
			subg->MakeTEG_mks(agent_number, inst->GetMksLB(agent_number) + bonus_cost);

		if (name.compare("asp-soc") == 0)
			subg->MakeTEG_soc(agent_number, bonus_cost);

		for (size_t x = 0; x < subg->time_expanded_graph.size(); x++)
		{
			for (size_t y = 0; y < subg->time_expanded_graph[x].size(); y++)
			{
				for (size_t a = 0; a < subg->time_expanded_graph[x][y].size(); a++)
				{
					/*for (size_t t = 0; t < subg->time_expanded_graph_xy[x][y][a].size(); t++)
					{
						string agent_loc = string("poss_loc(" + to_string(a + 1) + ",(" + to_string(x + 1) + "," + to_string(y + 1) + "),");
						asp << agent_loc << subg->time_expanded_graph_xy[x][y][a][t] << "). ";
					}*/

					if (subg->time_expanded_graph[x][y][a].size() == 0)
						continue;

					sort(subg->time_expanded_graph[x][y][a].begin(), subg->time_expanded_graph[x][y][a].end());

					bool single = true;
					bool print = false;
					int last = subg->time_expanded_graph[x][y][a][0];
					string agent_loc = string("poss_loc(" + to_string(a + 1) + ",(" + to_string(x + 1) + "," + to_string(y + 1) + "),");

					asp << agent_loc << subg->time_expanded_graph[x][y][a][0];

					for (size_t t = 1; t < subg->time_expanded_graph[x][y][a].size(); t++)
					{
						if (print)
							asp << agent_loc << subg->time_expanded_graph[x][y][a][t];

						if (subg->time_expanded_graph[x][y][a][t] == last + 1)
						{
							single = false;
							print = false;
							last = subg->time_expanded_graph[x][y][a][t];
							if (t == subg->time_expanded_graph[x][y][a].size() - 1)
								asp << ".." << last << "). ";
							continue;
						}
						
						if (single)
						{
							asp << "). ";
							print = true;
						}
						else
						{
							asp << ".." << last << "). ";
							single = true;
							print = true;
						}
						last = subg->time_expanded_graph[x][y][a][t];
					}

					if (subg->time_expanded_graph[x][y][a].size() == 1)
						asp << "). ";
				}
			}
		}
		asp << endl;

		if (print_path) // print shortest paths for each agent
		{
			for (size_t a = 0; a < agent_number; a++)
			{
				int size = inst->shortest_paths[a].size() - 1;
				for (size_t t = 0; t < inst->shortest_paths[a].size() - 1; t++)
				{
					asp << "spath(" << a + 1 << ",("
						<< inst->shortest_paths[a][size - t].x + 1 << "," << inst->shortest_paths[a][size - t].y + 1 << "),("
						<< inst->shortest_paths[a][size - (t+1)].x + 1 << "," << inst->shortest_paths[a][size - (t+1)].y + 1 << "),"
						<< t + 1 << "). ";
				}
				asp << "spath_length(" << a + 1 << "," << size << ").";
				asp << endl;
			}
		}
	}
	else
	{
		cout << "Could not open asp input file!" << endl;
		return;
	}

	asp.close();

	return;
}

int AspSolver::ReadResults(int agent_number, int bonus_cost)
{
	string line;
	string ifile = run_dir;
	ifstream input(ifile.append("/" + io_file_name + ".out"));
	if (input.is_open())
	{
		float solver_time = 0;
		float total_time = 0;
		bool solution_found = false;
		bool timeouted = true;

		while (getline(input, line))
		{
			if (line.compare("SATISFIABLE") == 0)	// solution found
			{
				solution_found = true;
				timeouted = false;
				continue;
			}

			if (line.compare("UNSATISFIABLE") == 0)	// solution found
			{
				solution_found = false;
				timeouted = false;
				continue;
			}

			if (line.rfind("Time", 0) == 0)	// time of computation
			{
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				solver_time = stof(parsed_line[12]);
				total_time = stof(parsed_line[10]);
			}

			if (line.rfind("Choices", 0) == 0)
			{
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				choices_vc.push_back(stoi(parsed_line[7]));
				cout << parsed_line[7] << endl;
			}

			if (line.rfind("Conflicts", 0) == 0)
			{
				cout << line << endl;
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				conflicts_vc.push_back(stoi(parsed_line[5]));
				cout << parsed_line[5] << endl;
			}

			if (line.rfind("Variables", 0) == 0)
			{
				cout << line << endl;
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				variables_vc.push_back(stoi(parsed_line[5]));
				cout << parsed_line[5] << endl;
			}

			if (line.rfind("Constraints", 0) == 0)
			{
				cout << line << endl;
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				constraints_vc.push_back(stoi(parsed_line[3]));
				cout << parsed_line[3] << endl;
			}
		}

		input.close();

		ofstream output;
		string ofile = stat_dir;
		output.open(ofile.append("/" + stat_file_name + ".log"), ios::app);
		if (output.is_open())
		{
			string res;
			if (solution_found)
				res = "OK";
			else
				res = "NO SOLUTION";

			if (timeouted)
				res = "TIMEOUT";

			output << inst->agents_file << "\t"
				<< agent_number << "\t"
				<< subg->vertices << "\t"
				<< inst->GetMksLB(agent_number) << "\t"
				<< inst->GetSocLB(agent_number) << "\t"
				<< bonus_cost << "\t"
				<< solver_time << "\t"
				<< total_time << "\t"
				<< res << "\t"
				<< choices_vc.back() << "\t"
				<< conflicts_vc.back() << "\t"
				<< variables_vc.back() << "\t"
				<< constraints_vc.back() << endl;

			output.close();
		}
		else
			cout << "Could not open log file!" << endl;

		total_runtime += total_time;
		total_solvertime += solver_time;
		timeout -= total_time;

		if (solution_found)
		{
			ofile = stat_dir;
			output.open(ofile.append("/" + stat_file_name + ".res"), ios::app);
			if (output.is_open())
			{
				output << name << "\t"
					<< inst->agents_file << "\t"
					<< agent_number << "\t"
					<< inst->number_of_vertices << "\t"
					<< inst->GetMksLB(agent_number) << "\t"
					<< inst->GetSocLB(agent_number) << "\t"
					<< subg->vertices << "\t"
					<< bonus_cost << "\t"
					<< total_solvertime << "\t"
					<< total_runtime << "\t"
					// choices - sum, mean, g_mean, std
					<< accumulate(choices_vc.begin(), choices_vc.end(), 0) << "\t"
					<< accumulate(choices_vc.begin(), choices_vc.end(), 0.0) / choices_vc.size() << "\t"
					<< GeometricMean(choices_vc) << "\t"
					<< StDev(choices_vc) << "\t"
					// conflicts - sum, mean, g_mean, std
					<< accumulate(conflicts_vc.begin(), conflicts_vc.end(), 0) << "\t"
					<< accumulate(conflicts_vc.begin(), conflicts_vc.end(), 0.0) / conflicts_vc.size() << "\t"
					<< GeometricMean(conflicts_vc) << "\t"
					<< StDev(conflicts_vc) << "\t"
					// variables - sum, mean, g_mean, std
					<< accumulate(variables_vc.begin(), variables_vc.end(), 0) << "\t"
					<< accumulate(variables_vc.begin(), variables_vc.end(), 0.0) / variables_vc.size() << "\t"
					<< GeometricMean(variables_vc) << "\t"
					<< StDev(variables_vc) << "\t"
					// constraints - sum, mean, g_mean, std
					<< accumulate(constraints_vc.begin(), constraints_vc.end(), 0) << "\t"
					<< accumulate(constraints_vc.begin(), constraints_vc.end(), 0.0) / constraints_vc.size() << "\t"
					<< GeometricMean(constraints_vc) << "\t"
					<< StDev(constraints_vc) << endl;

				output.close();
			}
			else
				cout << "Could not open results file!" << endl;

			return 0;
		}

		if (timeouted)
			return 1;	// timeout
		else
			return -1;	// no solution
	}
	else
		cout << "Could not open solution file!" << endl;
	
	return 1;
}



double AspSolver::GeometricMean(std::vector<int>& vc)
{
	double product = 1;

	for (int i = 0; i < vc.size(); i++)
		product = product * vc[i];

	double gm = pow(product, (double)1 / vc.size());
	return gm;
}

double AspSolver::StDev(std::vector<int>& vc)
{
	double sum = std::accumulate(vc.begin(), vc.end(), 0.0);
	double mean = sum / vc.size();

	double sq_sum = std::inner_product(vc.begin(), vc.end(), vc.begin(), 0.0);
	double stdev = std::sqrt(sq_sum / vc.size() - mean * mean);

	return stdev;
}
