#pragma once

#include "instance.hpp"

class SubgraphMaker
{
public:
	SubgraphMaker(Instance*);

	void ResetComputedMap(int);
	void PathsToMap(int);
	bool ExpandMap(int, int, int);
	void MakeTEG_mks(int,int);
	void MakeTEG_soc(int,int);
	int GiveNewNumbering();

	std::vector<std::vector<int> > computed_map;
	std::vector<std::vector<std::vector<int> > > individual_maps;
	std::vector<std::vector<std::vector<std::vector<int> > > > time_expanded_graph;

	int vertices;
	bool soc;
	bool use_individual_maps;
	
private:
	Instance* inst;
	
	bool has_numbering;

	bool IsReachable(int, int, int, int);
	bool FlattenMaps();
};
