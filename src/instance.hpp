#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>
#include <queue>
#include <algorithm>

//#include "Algorithms.hpp"

struct Vertex
{
	int x;
	int y;
};

struct Agent
{
	Vertex start;
	Vertex goal;
};

class Instance
{
public:
	Instance(std::string, std::string, std::string);
	int GetLB(int);

	void DebugPrint(std::vector<std::vector<int> >&);
	void DebugPrint(std::vector<int>&);
	void DebugPrint(std::vector<Vertex>&);

	std::vector<Agent> agents;
	std::vector<std::vector<int> > length_from_start;
	std::vector<std::vector<int> > length_from_goal;
	std::vector<int> SP_lengths;
	std::vector<std::vector<Vertex> > shortest_paths;

	std::vector<std::vector<int> > map;
	size_t height;
	size_t width;
	size_t number_of_vertices;

	std::string agents_file;

private:
	void LoadAgents(std::string, std::string);
	void LoadMap(std::string);
	void ComputeShortestPaths();
	void BFS(std::vector<int>&, Vertex);
};
