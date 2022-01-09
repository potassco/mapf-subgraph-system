#pragma once

#include "instance.hpp"

class SubgraphMaker
{
public:
	SubgraphMaker(Instance*);

	void ResetComputedMap();
	void PathsToMap(int);
	bool ExpandMap(int, int, int);
	void MakeTEG(int,int);
	void MakeTEG_XY(int,int);
	int GiveNewNumbering();

	std::vector<std::vector<int> > computed_map;
	std::vector<std::vector<std::vector<int> > > time_expanded_graph;
	std::vector<std::vector<std::vector<std::vector<int> > > > time_expanded_graph_xy;

	int vertices;
private:
	Instance* inst;
	
	bool has_numbering;
	bool vertex_reachable;

	bool IsReachable(int, int, int, int);
};
