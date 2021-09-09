#include "asp_solver.hpp"

using namespace std;

int AspSolver::Solve(int agent_number, int mks)
{
	if (timeout < 0)
		return 1;

	corr->GiveNewNumbering();

	PrintInstance(agent_number, mks);

	stringstream exec;
	exec << "INSTANCE=\"" << work_dir << "/instance.lp\" timeout " << (int)timeout + 1 << " " << work_dir << "/scripts/plan.sh > " << work_dir << "/output_asp";

	system(exec.str().c_str());

	return ReadResults(agent_number, mks);
}

void AspSolver::PrintInstance(int agent_number, int mks)
{
	ofstream asp;
	string ofile = work_dir;
	asp.open(ofile.append("/instance.lp"));
	if (asp.is_open())
	{
		for (size_t i = 0; i < corr->computed_map.size(); i++)
			for (int j = 0; j < corr->computed_map[i].size(); j++)
				if (corr->computed_map[i][j] != -1)
					asp << "node(" << i + 1 << "," << j + 1 << "). ";
		asp << endl;

		for (int i = 0; i < agent_number; i++)
			asp << "start(" << i + 1 << "," << inst->agents[i].start.x + 1 << "," << inst->agents[i].start.y + 1 << "). ";
		asp << endl;

		for (int i = 0; i < agent_number; i++)
			asp << "goal(" << i + 1 << "," << inst->agents[i].goal.x + 1 << "," << inst->agents[i].goal.y + 1 << "). ";
		asp << endl;

		asp << "makespan(" << mks << ")." << endl;
	}
	else
	{
		cout << "Could not open asp input file!" << endl;
		return;
	}

	asp.close();

	return;
}

int AspSolver::ReadResults(int agent_number, int mks)
{
	string line;
	string ifile = work_dir;
	ifstream input(ifile.append("/output_asp"));
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
		}

		input.close();

		ofstream output;
		string ofile = stat_dir;
		output.open(ofile.append("/").append(inst->agents_file).append("_").append(alg).append("_").append(name).append(".log"), ios::app);
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
				<< corr->vertices << "\t"
				<< mks << "\t"
				<< solver_time << "\t"
				<< total_time << "\t"
				<< res << endl;

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
			output.open(ofile.append("/").append(inst->agents_file).append("_").append(alg).append("_").append(name).append(".res"), ios::app);
			if (output.is_open())
			{
				output << inst->agents_file << "\t"
					<< agent_number << "\t"
					<< inst->number_of_vertices << "\t"
					<< corr->vertices << "\t"
					<< inst->GetLB(agent_number) << "\t"
					<< mks << "\t"
					<< total_solvertime << "\t"
					<< total_runtime << endl;

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
