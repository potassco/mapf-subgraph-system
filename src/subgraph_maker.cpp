#include "subgraph_maker.hpp"

using namespace std;

SubgraphMaker::SubgraphMaker(Instance* i)
{
	inst = i;
	has_numbering = false;
	soc = false;
}

void SubgraphMaker::ResetComputedMap(int agents)
{
	computed_map = vector<vector<int> >(inst->height, vector<int>(inst->width, -1));
	individual_maps = vector<vector<vector<int> > >(agents, vector<vector<int> >(inst->height, vector<int>(inst->width, -1)));
	has_numbering = false;
}

void SubgraphMaker::PathsToMap(int agents)
{
	for (size_t i = 0; i < agents; i++)
		for (size_t j = 0; j < inst->shortest_paths[i].size(); j++)
			individual_maps[i][inst->shortest_paths[i][j].x][inst->shortest_paths[i][j].y] = 0;

	FlattenMaps();

	has_numbering = false;
}

bool SubgraphMaker::ExpandMap(int subg_width, int agents, int bonus_cost)
{
	int subgraph_distance = 1;
	int total_cost = inst->GetMksLB(agents) + bonus_cost; // only for makespan; for soc is calculated later

	if (has_numbering)
	{
		FlattenMaps();
		has_numbering = false;
	}

	for (int k = 0; k < subg_width; k++)
	{
		for (size_t a = 0; a < individual_maps.size(); a++)
		{
			if (soc)
				total_cost = inst->SP_lengths[a] + bonus_cost;

			for (size_t i = 0; i < individual_maps[a].size(); i++)
			{
				for (size_t j = 0; j < individual_maps[a][i].size(); j++)
				{
					if (individual_maps[a][i][j] > -1 || inst->map[i][j] == -1)
						continue;

					if (i > 0 && individual_maps[a][i-1][j] != -1 && individual_maps[a][i-1][j] < subgraph_distance && IsReachable(i, j, a, total_cost))
					{
						individual_maps[a][i][j] = subgraph_distance;
						continue;
					}
					if (i < inst->height - 1 && individual_maps[a][i+1][j] != -1 && individual_maps[a][i+1][j] < subgraph_distance && IsReachable(i, j, a, total_cost))
					{
						individual_maps[a][i][j] = subgraph_distance;
						continue;
					}
					if (j > 0 && individual_maps[a][i][j-1] != -1 && individual_maps[a][i][j-1] < subgraph_distance && IsReachable(i, j, a, total_cost))
					{
						individual_maps[a][i][j] = subgraph_distance;
						continue;
					}
					if (j < inst->width - 1 && individual_maps[a][i][j+1] != -1 && individual_maps[a][i][j+1] < subgraph_distance && IsReachable(i, j, a, total_cost))
					{
						individual_maps[a][i][j] = subgraph_distance;
						continue;
					}
				}
			}
			subgraph_distance++;
		}
		
	}

	return FlattenMaps();
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

// vertex_x, vertex_y, agent, time
void SubgraphMaker::MakeTEG_mks(int agents, int mks)
{
	time_expanded_graph = vector<vector<vector<vector<int> > > >(computed_map.size(), vector<vector<vector<int> > >(computed_map[0].size(), vector<vector<int> >(agents)));

	for (size_t x = 0; x < computed_map.size(); x++)
		for (size_t y = 0; y < computed_map[x].size(); y++)
			for (size_t a = 0; a < time_expanded_graph[x][y].size(); a++)
				for (size_t t = 0; t <= mks; t++)
					if (computed_map[x][y] != -1 && inst->length_from_start[a][inst->map[x][y]] <= t && inst->length_from_goal[a][inst->map[x][y]] <= mks - t)
						time_expanded_graph[x][y][a].push_back(t);
}

// vertex_x, vertex_y, agent, time
void SubgraphMaker::MakeTEG_soc(int agents, int bonus_cost)
{
	time_expanded_graph = vector<vector<vector<vector<int> > > >(computed_map.size(), vector<vector<vector<int> > >(computed_map[0].size(), vector<vector<int> >(agents)));

	int mks = inst->GetMksLB(agents) + bonus_cost;

	for (size_t x = 0; x < computed_map.size(); x++)
		for (size_t y = 0; y < computed_map[x].size(); y++)
			for (size_t a = 0; a < time_expanded_graph[x][y].size(); a++)
			{
				for (size_t t = 0; t <= inst->SP_lengths[a] + bonus_cost; t++)
					if (computed_map[x][y] != -1 && inst->length_from_start[a][inst->map[x][y]] <= t && inst->length_from_goal[a][inst->map[x][y]] <= inst->SP_lengths[a] + bonus_cost - t)
						time_expanded_graph[x][y][a].push_back(t);
				if (inst->agents[a].start.x == x && inst->agents[a].start.y == y)
					for (size_t t = inst->SP_lengths[a] + bonus_cost + 1; t <= mks; t++)
						time_expanded_graph[x][y][a].push_back(t);
			}
}

bool SubgraphMaker::IsReachable(int x, int y, int agent, int cost)
{
	if (inst->length_from_start[agent][inst->map[x][y]] + inst->length_from_goal[agent][inst->map[x][y]] <= cost)
		return true;
	return false;
}

bool SubgraphMaker::FlattenMaps()
{
	bool changed = false;
	for (size_t a = 0; a < individual_maps.size(); a++)
		for (size_t i = 0; i < individual_maps[a].size(); i++)
			for (size_t j = 0; j < individual_maps[a][i].size(); j++)
				if (individual_maps[a][i][j] != -1)
				{
					if (computed_map[i][j] == -1)
						changed = true;
					computed_map[i][j] = 0;
					individual_maps[a][i][j] = 0;
				}
	return changed;
}
