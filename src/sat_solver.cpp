#include "sat_solver.hpp"

using namespace std;

int SatSolver::Solve(int agent_number, int mks)
{
	if (timeout < 0)
		return 1;

	mks = mks + 1; // different numbering of makespan -- number of states rather than number of actions

	corr->GiveNewNumbering();
	corr->MakeTEG(agent_number, mks);

	PrintInstance(agent_number, mks);

	stringstream exec;
	exec << "timeout " << (int)timeout + 1 << " " << work_dir << "/picat " << work_dir << "/mks_al.pi " << work_dir << "/instance.pi" << " > " << work_dir << "/output_sat";

	system(exec.str().c_str());

	return ReadResults(agent_number, mks);
}

void SatSolver::PrintInstance(int agent_number, int mks)
{
	ofstream picat;
	string ofile = work_dir;
	picat.open(ofile.append("/instance.pi"));
	if (picat.is_open())
	{
		picat << "ins(Graph, As, B) =>" << " ";
		picat << "Graph = [" << " ";

		bool first = true;

		for (size_t i = 0; i < inst->height; i++)
		{
			for (size_t j = 0; j < inst->width; j++)
			{
				if (corr->computed_map[i][j] != -1)
				{
					if (!first)
						picat << ", ";
					first = false;

					picat << "$neibs(" << corr->computed_map[i][j] + 1 << ",[" << corr->computed_map[i][j] + 1;

					if (i > 0 && corr->computed_map[i - 1][j] != -1)
						picat << "," << corr->computed_map[i - 1][j] + 1;
					if (i < inst->height - 1 && corr->computed_map[i + 1][j] != -1)
						picat << "," << corr->computed_map[i + 1][j] + 1;
					if (j > 0 && corr->computed_map[i][j - 1] != -1)
						picat << "," << corr->computed_map[i][j - 1] + 1;
					if (j < inst->width - 1 && corr->computed_map[i][j + 1] != -1)
						picat << "," << corr->computed_map[i][j + 1] + 1;

					picat << "])";
				}
			}
		}

		picat << "], As = [";

		for (int i = 0; i < agent_number; i++)
		{
			if (i != 0)
				picat << ",";
			picat << "(" << corr->computed_map[inst->agents[i].start.x][inst->agents[i].start.y] + 1 << "," << corr->computed_map[inst->agents[i].goal.x][inst->agents[i].goal.y] + 1 << ")";
		}

		picat << "]," << " ";
		picat << "B = new_array(" << mks << "," << agent_number << ")";

		for (size_t t = 0; t < corr->time_expanded_graph.size(); t++)
		{
			for (size_t a = 0; a < corr->time_expanded_graph[t].size(); a++)
			{
				picat << ", B[" << t + 1 << "," << a + 1 << "] = flatten([";
				bool single = true;
				int last = corr->time_expanded_graph[t][a][0];

				picat << corr->time_expanded_graph[t][a][0] + 1;

				//picat << corr->time_expanded_graph[t][a].front() + 1 << ".." << corr->time_expanded_graph[t][a].back() + 1;
				//picat << "1.." << corr->vertices;

				for (size_t v = 1; v < corr->time_expanded_graph[t][a].size(); v++)
				{
					if (corr->time_expanded_graph[t][a][v] == last + 1)
					{
						single = false;
						last = corr->time_expanded_graph[t][a][v];
						if (v == corr->time_expanded_graph[t][a].size() - 1)
							picat << ".." << last + 1;
						continue;
					}
					
					if (single)
					{
						picat << "," << corr->time_expanded_graph[t][a][v] + 1;
					}
					else
					{
						picat << ".." << last + 1 << "," << corr->time_expanded_graph[t][a][v] + 1;
						single = true;
					}
					last = corr->time_expanded_graph[t][a][v];
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
	string ifile = work_dir;
	ifstream input(ifile.append("/output_sat"));
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
					<< inst->GetLB(agent_number) << "\t"
					<< corr->vertices << "\t"
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
