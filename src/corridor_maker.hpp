#pragma once

#include "instance.hpp"

class CorridorMaker
{
public:
	CorridorMaker(Instance*);

	void ResetComputedMap();
	void PathsToMap(int);
	bool ExpandMap(int);
	void MakeTEG(int,int);
	int GiveNewNumbering();

	std::vector<std::vector<int> > computed_map;
	std::vector<std::vector<std::vector<int> > > time_expanded_graph;

	int vertices;
private:
	Instance* inst;
	
	bool has_numbering;
};