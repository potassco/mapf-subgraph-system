#include "sat_solver.hpp"

using namespace std;

int SatSolver::Solve(int agent_number, int bonus_cost)
{
	if (timeout < 0)
		return 1;

	stat_file_name = "statistics/sat_results.res";
	solver_call++;

	io_file_name.clear();
	if (debug)
		io_file_name.append(inst->agents_file + "_" + alg + "_" + inst->path_type + "_" + name + "_" + to_string(agent_number) + "_" + to_string(solver_call));
	else
		io_file_name.append(inst->agents_file + "_" + alg + "_" + inst->path_type + "_" + name);

	subg->GiveNewNumbering();

	PrintInstance(agent_number, bonus_cost);

	stringstream exec;
	exec << "./" << work_dir << "/SAT/MAPF" 
			<< " -e at_parallel_" << name << "_all"
			<< " -s " << run_dir << "/" << io_file_name << ".scen"
			<< " -m " << run_dir
			<< " -t " << timeout
			<< " -d " << bonus_cost
			<< " -l 1 -o -q > " << run_dir << "/" << io_file_name << ".out";

	if (no_solve) // do not call solver, just assume success
		return 0;

	system(exec.str().c_str());

	return ReadResults(agent_number, bonus_cost);
}

void SatSolver::PrintInstance(int agent_number, int bonus_cost)
{
	ofstream sat;
	string ofile = run_dir;
	sat.open(ofile.append("/" + io_file_name + ".map"));

	// print map file
	if (sat.is_open())
	{
		sat << "type octile\n"; 
		sat << "height " << inst->height << "\n";
		sat << "width " << inst->width << "\n";
		sat << "map\n";
		for (size_t i = 0; i < subg->computed_map.size(); i++)
		{
			for (int j = 0; j < subg->computed_map[i].size(); j++)
			{
				if (subg->computed_map[i][j] != -1)
					sat << ".";
				else
					sat << "@";
			}
			sat << "\n";
		}
	}
	else
	{
		cout << "Could not open sat input map file!" << endl;
		return;
	}
	sat.close();

	ofile = run_dir;
	sat.open(ofile.append("/" + io_file_name + ".scen"));

	// print scenario file
	if (sat.is_open())
	{
		sat << "version 1\n";
		for (size_t a = 0; a < agent_number; a++)
		{
			sat << "0" << "\t"
				<< io_file_name << ".map" << "\t"
				<< inst->height << "\t"
				<< inst->width << "\t"
				<< inst->agents[a].start.y << "\t"
				<< inst->agents[a].start.x << "\t"
				<< inst->agents[a].goal.y << "\t"
				<< inst->agents[a].goal.x << "\t"
				<< "0\n";
		}
	}
	else
	{
		cout << "Could not open sat input scen file!" << endl;
		return;
	}
	sat.close();

	return;
}

int SatSolver::ReadResults(int agent_number, int bonus_cost)
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

		getline(input, line);

		if (line.compare("") != 0)
		{
			stringstream ssline(line);
			string part;
			vector<string> parsed_line;
			while (getline(ssline, part, '\t'))
				parsed_line.push_back(part);

			if (parsed_line.size() == 14)
			{
				timeouted = false;
				solver_time = stoi(parsed_line[9]);
				total_time = stoi(parsed_line[8]) + stoi(parsed_line[9]);
				variables_vc.push_back(stoi(parsed_line[11]));
				constraints_vc.push_back(stoi(parsed_line[12]));
				if (parsed_line[13].compare("sat") == 0)
					solution_found = true;
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
				output << "sat" << "\t"
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
					<< total_solvertime << "\t"
					<< total_runtime << "\t"
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
