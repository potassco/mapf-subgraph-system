#pragma once

#include <filesystem>
#include <chrono>

#include "instance.hpp"
#include "asp_solver.hpp"
#include "subgraph_maker.hpp"

class Strategy
{
public:
	Strategy(bool, bool, bool, char, std::string, std::string, int, std::string, std::string, std::string, std::string, std::string, std::string);
	~Strategy();

	int RunTests();

private:
	std::string GetFilename(std::string);

	Instance* inst;
	ISolver* sol;
	SubgraphMaker* subg;

	std::string alg;
	bool B,M,P,C;

	int timeout;
};
