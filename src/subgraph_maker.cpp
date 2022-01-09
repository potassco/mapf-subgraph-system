#include "subgraph_maker.hpp"

using namespace std;

SubgraphMaker::SubgraphMaker(Instance* i)
{
	inst = i;
	has_numbering = false;
}

void SubgraphMaker::ResetComputedMap()
{
	computed_map = vector<vector<int> >(inst->height, vector<int>(inst->width, -1));
	has_numbering = false;
}

void SubgraphMaker::PathsToMap(int agents)
{
	for (size_t i = 0; i < agents; i++)
		for (size_t j = 0; j < inst->shortest_paths[i].size(); j++)
			computed_map[inst->shortest_paths[i][j].x][inst->shortest_paths[i][j].y] = 0;

	has_numbering = false;
}

bool SubgraphMaker::ExpandMap(int subg_width, int agents, int mks)
{
	bool expanded = false;
	int subgraph_distance = 1;

	if (has_numbering)
	{
		for (size_t i = 0; i < computed_map.size(); i++)
			for (size_t j = 0; j < computed_map[i].size(); j++)
				if (computed_map[i][j] != -1)
					computed_map[i][j] = 0;
		has_numbering = false;
	}

	for (int k = 0; k < subg_width; k++)
	{
		for (size_t i = 0; i < computed_map.size(); i++)
		{
			for (size_t j = 0; j < computed_map[i].size(); j++)
			{
				if (computed_map[i][j] > -1 || inst->map[i][j] == -1)
					continue;
				vertex_reachable = false;

				if (i > 0 && computed_map[i-1][j] != -1 && computed_map[i-1][j] < subgraph_distance && IsReachable(i, j, agents, mks))
				{
					computed_map[i][j] = subgraph_distance;
					expanded = true;
					continue;
				}
				if (i < inst->height - 1 && computed_map[i+1][j] != -1 && computed_map[i+1][j] < subgraph_distance && IsReachable(i, j, agents, mks))
				{
					computed_map[i][j] = subgraph_distance;
					expanded = true;
					continue;
				}
				if (j > 0 && computed_map[i][j-1] != -1 && computed_map[i][j-1] < subgraph_distance && IsReachable(i, j, agents, mks))
				{
					computed_map[i][j] = subgraph_distance;
					expanded = true;
					continue;
				}
				if (j < inst->width - 1 && computed_map[i][j+1] != -1 && computed_map[i][j+1] < subgraph_distance && IsReachable(i, j, agents, mks))
				{
					computed_map[i][j] = subgraph_distance;
					expanded = true;
					continue;
				}
			}
		}
		subgraph_distance++;
	}

	return expanded;
}

int SubgraphMaker::GiveNewNumbering()
{
	if (!has_numbering)
	{
		vertices = 0;
		for (size_t i = 0; i < computed_map.size(); i++)
			for (size_t j = 0; j < computed_map[i].size(); j++)
				if (computed_map[i][j] != -1)
				{
					computed_map[i][j] = vertices;
					vertices++;
				}

		has_numbering = true;
	}

	return vertices;
}

// time, agent, vertex
void SubgraphMaker::MakeTEG(int agents, int mks)
{
	time_expanded_graph = vector<vector<vector<int> > >(mks, vector<vector<int> >(agents));

	for (size_t t = 0; t < time_expanded_graph.size(); t++)
		for (size_t a = 0; a < time_expanded_graph[t].size(); a++)
			for (size_t x = 0; x < computed_map.size(); x++)
				for (size_t y = 0; y < computed_map[x].size(); y++)
					if (computed_map[x][y] != -1 && inst->length_from_start[a][inst->map[x][y]] <= t && inst->length_from_goal[a][inst->map[x][y]] <= mks - t)
						time_expanded_graph[t][a].push_back(computed_map[x][y]);
}

// vertex_x, vertex_y, agent, time
void SubgraphMaker::MakeTEG_XY(int agents, int mks)
{
	time_expanded_graph_xy = vector<vector<vector<vector<int> > > >(computed_map.size(), vector<vector<vector<int> > >(computed_map[0].size(), vector<vector<int> >(agents)));

	for (size_t x = 0; x < computed_map.size(); x++)
		for (size_t y = 0; y < computed_map[x].size(); y++)
			for (size_t a = 0; a < time_expanded_graph_xy[x][y].size(); a++)
				for (size_t t = 0; t <= mks; t++)
					if (computed_map[x][y] != -1 && inst->length_from_start[a][inst->map[x][y]] <= t && inst->length_from_goal[a][inst->map[x][y]] <= mks - t)
						time_expanded_graph_xy[x][y][a].push_back(t);
}

bool SubgraphMaker::IsReachable(int x, int y, int agents, int mks)
{
	if (vertex_reachable)
		return true;;

	for (size_t i = 0; i < agents; i++)
	{
		if (inst->length_from_start[i][inst->map[x][y]] + inst->length_from_goal[i][inst->map[x][y]] <= mks)
		{
			vertex_reachable = true;
			return true;
		}
	}

	return false;
}
