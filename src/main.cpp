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
	char *svalue = NULL;
	char *ivalue = NULL;
	char *bvalue = NULL;
	char *tvalue = NULL;

	int timeout = 300;
        fs::path parent_path = fs::path(__FILE__).parent_path().parent_path();
	string work_dir = parent_path / "encodings";
	string statistics_dir = fs::current_path() / "statistics";
        fs::create_directory(statistics_dir);
	string input_dir = parent_path / "resources/instances/scenarios";
	string map_dir = parent_path / "resources/instances/maps";

	// parse arguments
	opterr = 0;
	char c;
	while ((c = getopt (argc, argv, "hi:s:b:t:")) != -1)
	{
		switch (c)
		{
			case 'h':
				hflag = true;
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
			case '?':
				if (optopt == 'i' || optopt == 's' || optopt == 'b')
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

	if (string(bvalue).compare("sat") != 0 && string(bvalue).compare("asp") != 0)
	{
		cout << "Unknown base algorithm \"" << bvalue << "\"!" << endl;
		printHelp(argv);
		return -1;
	}

	if (tvalue != NULL)
		timeout = atoi(tvalue);

	Strategy* strat;

	if (svalue[0] == 'b' || svalue[0] == 'm' || svalue[0] == 'p' || svalue[0] == 'c')
		strat = new Strategy(svalue[0], ivalue, bvalue, timeout, work_dir, statistics_dir, input_dir, map_dir);
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
	cout << argv[0] << " [-h] -b base_algorithm -i agents_file -s strategy [-t timeout]" << endl;
	cout << "	-h                  : prints help and exits" << endl;
	cout << "	-b base_algorithm   : base algorithm to be used. Available options are sat|asp" << endl;
	cout << "	-i agents_file      : path to an agents file" << endl;
	cout << "	-s strategy         : strategy to be used. Available options are b|m|p|c" << endl;
	cout << "	-t timeout          : timeout of the computation. Default value is 300s" << endl;
}
