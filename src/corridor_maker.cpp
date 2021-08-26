#include "corridor_maker.hpp"

using namespace std;

CorridorMaker::CorridorMaker(Instance* i)
{
	inst = i;
	has_numbering = false;
}

void CorridorMaker::ResetComputedMap()
{
	computed_map = vector<vector<int> >(inst->height, vector<int>(inst->width, -1));
	has_numbering = false;
}

void CorridorMaker::PathsToMap(int agents)
{
	for (size_t i = 0; i < agents; i++)
		for (size_t j = 0; j < inst->shortest_paths[i].size(); j++)
			computed_map[inst->shortest_paths[i][j].x][inst->shortest_paths[i][j].y] = 0;

	has_numbering = false;
}

bool CorridorMaker::ExpandMap(int corr_width)
{
	// TODO - if expanded nodes are not reachable in given makespan, then adding them has no meaning

	bool expanded = false;
	int corridor_distance = 1;

	if (has_numbering)
	{
		for (size_t i = 0; i < computed_map.size(); i++)
			for (size_t j = 0; j < computed_map[i].size(); j++)
				if (computed_map[i][j] != -1)
					computed_map[i][j] = 0;
		has_numbering = false;
	}

	for (int k = 0; k < corr_width; k++)
	{
		for (size_t i = 0; i < computed_map.size(); i++)
		{
			for (size_t j = 0; j < computed_map[i].size(); j++)
			{
				if (computed_map[i][j] > -1 || inst->map[i][j] == -1)
					continue;

				if (i > 0 && computed_map[i-1][j] != -1 && computed_map[i-1][j] < corridor_distance)
				{
					computed_map[i][j] = corridor_distance;
					expanded = true;
				}
				if (i < inst->height - 1 && computed_map[i+1][j] != -1 && computed_map[i+1][j] < corridor_distance)
				{
					computed_map[i][j] = corridor_distance;
					expanded = true;
				}
				if (j > 0 && computed_map[i][j-1] != -1 && computed_map[i][j-1] < corridor_distance)
				{
					computed_map[i][j] = corridor_distance;
					expanded = true;
				}
				if (j < inst->width - 1 && computed_map[i][j+1] != -1 && computed_map[i][j+1] < corridor_distance)
				{
					computed_map[i][j] = corridor_distance;
					expanded = true;
				}
			}
		}
		corridor_distance++;
	}

	return expanded;
}

int CorridorMaker::GiveNewNumbering()
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

void CorridorMaker::MakeTEG(int agents, int mks)
{
	time_expanded_graph = vector<vector<vector<int> > >(mks, vector<vector<int> >(agents));

	for (size_t t = 0; t < time_expanded_graph.size(); t++)
		for (size_t a = 0; a < time_expanded_graph[t].size(); a++)
			for (size_t x = 0; x < computed_map.size(); x++)
				for (size_t y = 0; y < computed_map[x].size(); y++)
					if (computed_map[x][y] != -1 && inst->length_from_start[a][inst->map[x][y]] <= t && inst->length_from_goal[a][inst->map[x][y]] <= mks - t)
						time_expanded_graph[t][a].push_back(computed_map[x][y]);
}