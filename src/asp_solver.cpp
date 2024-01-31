#include "asp_solver.hpp"

using namespace std;

int AspSolver::Solve(int agent_number, int bonus_cost)
{
	if (timeout < 0)
		return 1;

	stat_file_name = "statistics/asp_results.res";
	solver_call++;

	io_file_name.clear();
	if (debug)
		io_file_name.append(inst->agents_file + "_" + alg + "_" + inst->path_type + "_" + name + "_" + to_string(agent_number) + "_" + to_string(solver_call));
	else
		io_file_name.append(inst->agents_file + "_" + alg + "_" + inst->path_type + "_" + name);

	subg->GiveNewNumbering();

	PrintInstance(agent_number, bonus_cost);

	stringstream exec;
	exec << "python " << work_dir << "/ASP/mapf.py"
							<< " -i " << run_dir << "/" << io_file_name << ".lp"
							<< " -m " << name 
							<< " -d " << bonus_cost
							<< " -t " << (int)timeout + 1 
								<< " > " << run_dir << "/" << io_file_name << ".out";

	if (no_solve) // do not call solver, just assume success
		return 0;

	cout << exec.str() << endl;

	int ret = system(exec.str().c_str());

	if (ret != 0)
		return 1;

	return ReadResults(agent_number, bonus_cost);
}

void AspSolver::PrintInstance(int agent_number, int bonus_cost)
{
	ofstream asp;
	string ofile = run_dir;
	asp.open(ofile.append("/" + io_file_name + ".lp"));
	if (asp.is_open())
	{
		for (size_t i = 0; i < subg->computed_map.size(); i++)
			for (int j = 0; j < subg->computed_map[i].size(); j++)
				if (subg->computed_map[i][j] != -1)
					asp << "vertex((" << i + 1 << "," << j + 1 << ")). ";
		asp << "\n";

		for (size_t i = 0; i < subg->computed_map.size(); i++)
		{
			for (size_t j = 0; j < subg->computed_map[i].size(); j++)
			{
				if (subg->computed_map[i][j] == -1)
					continue;
				if (i > 0 && subg->computed_map[i-1][j] != -1)
					asp << "edge((" << i + 1 << "," << j + 1 << "),(" << i << "," << j + 1 << ")). ";
				if (i < subg->computed_map.size() - 1 && subg->computed_map[i+1][j] != -1)
					asp << "edge((" << i + 1 << "," << j + 1 << "),(" << i + 2 << "," << j + 1 << ")). ";
				if (j > 0 && subg->computed_map[i][j-1] != -1)
					asp << "edge((" << i + 1 << "," << j + 1 << "),(" << i + 1 << "," << j << ")). ";
				if (j < subg->computed_map[i].size() - 1 && subg->computed_map[i][j+1] != -1)
					asp << "edge((" << i + 1 << "," << j + 1 << "),(" << i + 1<< "," << j + 2 << ")). ";
			}
		}
		asp << "\n";

		for (int i = 0; i < agent_number; i++)
			asp << "start(" << i + 1 << ",(" << inst->agents[i].start.x + 1 << "," << inst->agents[i].start.y + 1 << ")). ";
		asp << "\n";

		for (int i = 0; i < agent_number; i++)
			asp << "goal(" << i + 1 << ",(" << inst->agents[i].goal.x + 1 << "," << inst->agents[i].goal.y + 1 << ")). ";
		asp << "\n";

		for (int i = 0; i < agent_number; i++)
			asp << "agent(" << i + 1 << ").";
		asp << "\n";
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

			if (line.rfind("time", 0) == 0)	// total time
			{
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				total_time = stof(parsed_line[1]);
			}

			if (line.rfind("Solving", 0) == 0)	// solver time
			{
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				solver_time = stof(parsed_line[1]);
			}

			if (line.rfind("vars", 0) == 0)
			{
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				variables_vc.push_back(stoi(parsed_line[1]));
			}

			if (line.rfind("constraints", 0) == 0)
			{
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				constraints_vc.push_back(stoi(parsed_line[1]));
			}
		}
		input.close();

		total_runtime += total_time;
		total_solvertime += solver_time;
		timeout -= total_time;

		if (solution_found)
		{
			ofstream output;
			output.open(stat_file_name, ios::app);
			if (output.is_open())
			{
				output << "asp" << "\t"
					<< inst->agents_file << "\t"
					<< inst->path_type << "\t"
					<< name << "\t"
					<< alg << "\t"
					<< agent_number << "\t"
					<< inst->number_of_vertices << "\t"
					<< inst->GetMksLB(agent_number) << "\t"
					<< inst->GetSocLB(agent_number) << "\t"
					<< subg->vertices << "\t"
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

		if (timeouted)
			return 1;	// timeout
		else
			return -1;	// no solution
	}
	else
		cout << "Could not open solution file!" << endl;
	
	return 1;
}
