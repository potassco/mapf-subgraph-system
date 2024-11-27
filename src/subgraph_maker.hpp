#pragma once

#include "instance.hpp"

class SubgraphMaker
{
public:
	SubgraphMaker(Instance*);

	void ResetComputedMap(int);
	void PathsToMap(int);
	bool ExpandMap(int, int, int);
	int GiveNewNumbering();
	std::pair<int,int> GetReachTimesMKS(int, int, Vertex, int);
	std::pair<int,int> GetReachTimesSOC(int, int, Vertex, int);
	
	//void MakeTEG_mks(int,int);
	//void MakeTEG_soc(int,int);

	std::vector<std::vector<int> > computed_map;
	std::vector<std::vector<std::vector<int> > > individual_maps;

	// lengths based on the subgraph
	std::vector<std::vector<int> > length_from_start;
	std::vector<std::vector<int> > length_from_goal;
	// std::vector<std::vector<std::vector<std::vector<int> > > > time_expanded_graph;

	int vertices;
	bool soc;
	bool use_individual_maps;
	
private:
	Instance* inst;
	
	bool has_numbering;

	bool IsReachable(int, int, int, int);
	bool FlattenMaps();
	void BFS(std::vector<int>&, Vertex);
};
