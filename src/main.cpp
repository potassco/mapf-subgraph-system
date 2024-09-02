#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <experimental/filesystem>

#include "strategy.hpp"

using namespace std;
namespace fs = std::experimental::filesystem;

vector<string> solver_names = {"asp-mks","asp-soc","asp-inc-mks","asp-inc-soc","sat-mks","sat-soc"};

void printHelp(char**);

int main(int argc, char** argv) 
{
	bool hflag = false;
	bool dflag = false;
	bool oflag = false;
	bool nflag = false;
	char *svalue = NULL;
	char *ivalue = NULL;
	char *bvalue = NULL;
	char *tvalue = NULL;
	char *pvalue = NULL;
	char *avalue = NULL;
	char *kvalue = NULL;

	srand('a');

	int timeout = 300;
	int init_agents = 1;
	int increment_agents = 1;
	string path_type = "biased";
		fs::path parent_path = fs::path(__FILE__).parent_path().parent_path();
	string work_dir = parent_path / "solvers";
	string statistics_dir = fs::current_path() / "statistics";
		fs::create_directory(statistics_dir);
	string input_dir = parent_path / "resources/scenarios";
	string map_dir = parent_path / "resources/maps";
	string run_dir = fs::current_path() / "run";
		fs::create_directory(run_dir);

	// parse arguments
	opterr = 0;
	char c;
	while ((c = getopt (argc, argv, "hdoni:s:b:t:p:a:k:")) != -1)
	{
		switch (c)
		{
			case 'h':
				hflag = true;
				break;
			case 'd':
				dflag = true;
				break;
			case 'o':
				oflag = true;
				break;
			case 'n':
				nflag = true;
				break;
			case 'i':
				ivalue = optarg;
				break;
			case 's':
				svalue = optarg;
				break;
			case 'b':
				bvalue = optarg;
				break;
			case 't':
				tvalue = optarg;
				break;
			case 'p':
				pvalue = optarg;
				break;
			case 'a':
				avalue = optarg;
				break;
			case 'k':
				kvalue = optarg;
				break;
			case '?':
				if (optopt == 'i' || optopt == 's' || optopt == 'b' || optopt == 't' || optopt == 'p' || optopt == 'a' || optopt == 'k')
				{
					cout << "Option -" << (char)optopt << " requires an argument!" << endl;
					return -1;
				}
				// unknown option - ignore it
				break;
			default:
				return -1; // should not get here;
		}
	}

	if (hflag)
	{
		printHelp(argv);
		return 0;
	}

	if (ivalue == NULL || svalue == NULL || bvalue == NULL)
	{
		cout << "Missing a required argument!" << endl;
		printHelp(argv);
		return -1;
	}

	bool ok_name = false;
	for (size_t n = 0; n < solver_names.size(); n++)
	{
		if (string(bvalue).compare(solver_names[n]) == 0)
		{
			ok_name = true;
			break;
		}
	}
	if (!ok_name)
	{
		cout << "Unknown base algorithm \"" << bvalue << "\"!" << endl;
		printHelp(argv);
		return -1;
	}

	if (tvalue != NULL)
		timeout = atoi(tvalue);

	if (avalue != NULL)
		init_agents = atoi(avalue);

	if (kvalue != NULL)
		increment_agents = atoi(kvalue);

	if (pvalue != NULL)
		path_type = pvalue;

	Strategy* strat;

	if (svalue[0] == 'b' || svalue[0] == 'm' || svalue[0] == 'p' || svalue[0] == 'c')
		strat = new Strategy(dflag, nflag, oflag, svalue[0], ivalue, bvalue, timeout, init_agents, increment_agents, work_dir, statistics_dir, input_dir, map_dir, run_dir, path_type);
	else
	{
		cout << "Unknown strategy!" << endl;
		printHelp(argv);
		return -1;
	}

	strat->RunTests();

	delete strat;

	return 0;
}

void printHelp(char* argv[])
{
	cout << "Usage of this generator:" << endl;
	cout << argv[0] << " [-h] [-d] [-n] [-o] -b base_algorithm -i agents_file -s strategy [-t timeout] [-p shortest_path] [-P] [-a initial_agents] [-k agents_increment]" << endl;
	cout << "	-h                  : prints help and exits" << endl;
	cout << "	-d                  : debug print - keep all of the used instance and output files" << endl;
	cout << "	-n                  : do not call solver, only print instance in given format" << endl;
	cout << "	-o                  : oneshot solve, do not perform any subsequent calls" << endl;
	cout << "	-b base_algorithm   : base algorithm to be used. Available options are ";
		for (size_t n = 0; n < solver_names.size(); n++) {cout << solver_names[n] << " ";}
		cout << endl;
	cout << "	-i agents_file      : path to an agents file" << endl;
	cout << "	-s strategy         : strategy to be used. Available options are b|m|p|c" << endl;
	cout << "	-t timeout          : timeout of the computation in s. Default value is 300s" << endl;
	cout << "	-p shortest_path    : what shortest path to use to create the pruned graph. Available options are biased|random|cross|time. Default is biased" << endl;
	cout << "	-a initial_agents   : initial number of agents. Default is 1" << endl;
	cout << "	-k agents_increment : increment in number of agents after successful solve. Default is 1. If 0 is selected, do not increase." << endl;
}
