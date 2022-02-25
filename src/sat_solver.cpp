#include "sat_solver.hpp"

using namespace std;

int SatSolver::Solve(int agent_number, int mks)
{
	if (timeout < 0)
		return 1;

	mks = mks + 1; // different numbering of makespan -- number of states rather than number of actions

	solver_call++;

	io_file_name.clear();
	if (debug)
		io_file_name.append(inst->agents_file + "_" + alg + "_" + name + "_" + path + "_" + to_string(agent_number) + "_" + to_string(solver_call));
	else
		io_file_name.append(inst->agents_file + "_" + alg + "_" + name + "_" + path);

	stat_file_name.clear();
	stat_file_name.append(inst->agents_file + "_" + alg + "_" + name + "_" + path);

	subg->GiveNewNumbering();
	subg->MakeTEG(agent_number, mks);

	PrintInstance(agent_number, mks);

	stringstream exec;
	exec << "timeout " << (int)timeout + 1 << " " << work_dir << "/picat " << work_dir << "/mks_al.pi " << run_dir << "/" << io_file_name << ".pi" << " > " << run_dir << "/" << io_file_name << ".out";

	system(exec.str().c_str());

	return ReadResults(agent_number, mks);
}

void SatSolver::PrintInstance(int agent_number, int mks)
{
	ofstream picat;
	string ofile = run_dir;
	picat.open(ofile.append("/" + io_file_name + ".pi"));
	if (picat.is_open())
	{
		picat << "ins(Graph, As, B) =>" << " ";
		picat << "Graph = [" << " ";

		bool first = true;

		for (size_t i = 0; i < inst->height; i++)
		{
			for (size_t j = 0; j < inst->width; j++)
			{
				if (subg->computed_map[i][j] != -1)
				{
					if (!first)
						picat << ", ";
					first = false;

					picat << "$neibs(" << subg->computed_map[i][j] + 1 << ",[" << subg->computed_map[i][j] + 1;

					if (i > 0 && subg->computed_map[i - 1][j] != -1)
						picat << "," << subg->computed_map[i - 1][j] + 1;
					if (i < inst->height - 1 && subg->computed_map[i + 1][j] != -1)
						picat << "," << subg->computed_map[i + 1][j] + 1;
					if (j > 0 && subg->computed_map[i][j - 1] != -1)
						picat << "," << subg->computed_map[i][j - 1] + 1;
					if (j < inst->width - 1 && subg->computed_map[i][j + 1] != -1)
						picat << "," << subg->computed_map[i][j + 1] + 1;

					picat << "])";
				}
			}
		}

		picat << "], As = [";

		for (int i = 0; i < agent_number; i++)
		{
			if (i != 0)
				picat << ",";
			picat << "(" << subg->computed_map[inst->agents[i].start.x][inst->agents[i].start.y] + 1 << "," << subg->computed_map[inst->agents[i].goal.x][inst->agents[i].goal.y] + 1 << ")";
		}

		picat << "]," << " ";
		picat << "B = new_array(" << mks << "," << agent_number << ")";

		for (size_t t = 0; t < subg->time_expanded_graph.size(); t++)
		{
			for (size_t a = 0; a < subg->time_expanded_graph[t].size(); a++)
			{
				picat << ", B[" << t + 1 << "," << a + 1 << "] = flatten([";
				bool single = true;
				int last = subg->time_expanded_graph[t][a][0];

				picat << subg->time_expanded_graph[t][a][0] + 1;

				//picat << subg->time_expanded_graph[t][a].front() + 1 << ".." << subg->time_expanded_graph[t][a].back() + 1;
				//picat << "1.." << subg->vertices;

				for (size_t v = 1; v < subg->time_expanded_graph[t][a].size(); v++)
				{
					if (subg->time_expanded_graph[t][a][v] == last + 1)
					{
						single = false;
						last = subg->time_expanded_graph[t][a][v];
						if (v == subg->time_expanded_graph[t][a].size() - 1)
							picat << ".." << last + 1;
						continue;
					}
					
					if (single)
					{
						picat << "," << subg->time_expanded_graph[t][a][v] + 1;
					}
					else
					{
						picat << ".." << last + 1 << "," << subg->time_expanded_graph[t][a][v] + 1;
						single = true;
					}
					last = subg->time_expanded_graph[t][a][v];
				}
				picat << "])";
			}
		}

		picat << ".";
	}
	else
	{
		cout << "Could not open picat input file!" << endl;
		return;
	}

	picat.close();

	return;
}

int SatSolver::ReadResults(int agent_number, int mks)
{
	mks = mks - 1; //change numbering back to stay consistent with ASP solver

	string line;
	string ifile = run_dir;
	ifstream input(ifile.append("/" + io_file_name + ".out"));
	if (input.is_open())
	{
		float solver_time = 0;
		float total_time = 0;
		bool look_for_solver = false;
		bool solution_found = false;
		bool timeouted = true;

		while (getline(input, line))
		{
			if (line.compare("done") == 0)	// solution found
			{
				solution_found = true;
				continue;
			}

			if (line.rfind("solving start", 0) == 0)	// SAT started
			{
				look_for_solver = true;
				continue;
			}

			if (line.rfind("CPU", 0) == 0)	// time of computation
			{
				timeouted = false;
				stringstream ssline(line);
				string part;
				vector<string> parsed_line;
				while (getline(ssline, part, ' '))
					parsed_line.push_back(part);

				if (look_for_solver)
				{
					solver_time = stof(parsed_line[2]);
					look_for_solver = false;
				}
				else
					total_time = stof(parsed_line[2]);
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
			output.open(ofile.append("/" + stat_file_name + ".res"), ios::app);
			if (output.is_open())
			{
				output << inst->agents_file << "\t"
					<< agent_number << "\t"
					<< inst->number_of_vertices << "\t"
					<< inst->GetLB(agent_number) << "\t"
					<< subg->vertices << "\t"
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
