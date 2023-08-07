#include <iostream>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <experimental/filesystem>

#include "strategy.hpp"

using namespace std;
namespace fs = std::experimental::filesystem;

void printHelp(char**);

int main(int argc, char** argv) 
{
	bool hflag = false;
	bool dflag = false;
	bool Pflag = false;
	bool nflag = false;
	char *svalue = NULL;
	char *ivalue = NULL;
	char *bvalue = NULL;
	char *tvalue = NULL;
	char *pvalue = NULL;

	srand('a');

	int timeout = 300;
		fs::path parent_path = fs::path(__FILE__).parent_path().parent_path();
	string work_dir = parent_path / "encodings";
	string statistics_dir = fs::current_path() / "statistics";
		fs::create_directory(statistics_dir);
	string input_dir = parent_path / "resources/scenarios";
	string map_dir = parent_path / "resources/maps";
	string run_dir = fs::current_path() / "run";
		fs::create_directory(run_dir);

	// parse arguments
	opterr = 0;
	char c;
	while ((c = getopt (argc, argv, "hdPni:s:b:t:p:")) != -1)
	{
		switch (c)
		{
			case 'h':
				hflag = true;
				break;
			case 'd':
				dflag = true;
				break;
			case 'P':
				Pflag = true;
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
			case '?':
				if (optopt == 'i' || optopt == 's' || optopt == 'b' || optopt == 't' || optopt == 'p')
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

	if (string(bvalue).compare("asp-mks") != 0 && string(bvalue).compare("asp-soc") != 0)
	{
		cout << "Unknown base algorithm \"" << bvalue << "\"!" << endl;
		printHelp(argv);
		return -1;
	}

	if (tvalue != NULL)
		timeout = atoi(tvalue);

	if (pvalue == NULL)
		pvalue = "single";

	if (Pflag && string(pvalue).compare("single") != 0)
	{
		cout << "-P can be used only with -p single. Ignoring -P" << endl;
		Pflag = false;
	}

	Strategy* strat;

	if (svalue[0] == 'b' || svalue[0] == 'm' || svalue[0] == 'p' || svalue[0] == 'c')
		strat = new Strategy(dflag, Pflag, nflag, svalue[0], ivalue, bvalue, timeout, work_dir, statistics_dir, input_dir, map_dir, run_dir, pvalue);
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
	cout << argv[0] << " [-h] [-d] -b base_algorithm -i agents_file -s strategy [-t timeout] [-p shortest_path]" << endl;
	cout << "	-h                  : prints help and exits" << endl;
	cout << "	-d                  : debug print - keep all of the used instance and output files" << endl;
	cout << "	-n                  : do not call solver, only print instance in given format" << endl;
	cout << "	-b base_algorithm   : base algorithm to be used. Available options are asp-mks|asp-soc" << endl;
	cout << "	-i agents_file      : path to an agents file" << endl;
	cout << "	-s strategy         : strategy to be used. Available options are b|m|p|c" << endl;
	cout << "	-t timeout          : timeout of the computation. Default value is 300s" << endl;
	cout << "	-p shortest_path    : what shortest path to use to create the pruned graph. Available options are single|all|random|diverse. Default is single." << endl;
	cout << "	-P                  : print the shortest path used to build the pruned graph. Works only with -p single." << endl;
}
