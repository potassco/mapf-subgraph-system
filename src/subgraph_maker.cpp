#include "subgraph_maker.hpp"

using namespace std;

SubgraphMaker::SubgraphMaker(Instance* i)
{
	inst = i;
	has_numbering = false;
	soc = false;
	use_individual_maps = false;
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

	if (changed) // map has changed, recalculate the distances!
	{
		length_from_start = vector<vector<int> >(inst->agents.size(), vector<int>(inst->number_of_vertices, -1));
		length_from_goal = vector<vector<int> >(inst->agents.size(), vector<int>(inst->number_of_vertices, -1));

		for (size_t i = 0; i < individual_maps.size(); i++)
		{
			BFS(length_from_start[i], inst->agents[i].start);
			BFS(length_from_goal[i], inst->agents[i].goal);
		}
	}

	return changed;
}

pair<int, int> SubgraphMaker::GetReachTimesMKS(int a, int total_agents, Vertex v, int delta)
{
	int start_t = length_from_start[a][inst->map[v.x][v.y]];
	int end_t = inst->GetMksLB(total_agents) + delta - length_from_goal[a][inst->map[v.x][v.y]];

	return pair<int,int>(start_t, end_t);
}

pair<int, int> SubgraphMaker::GetReachTimesSOC(int a, int total_agents, Vertex v, int delta)
{
	int start_t = length_from_start[a][inst->map[v.x][v.y]];
	int end_t = 0;

	// the node is unreachable in the current subgraph
	if (start_t == -1)
		return pair<int,int>(start_t, -2);

	int at_goal = inst->SP_lengths[a] + delta;
	for (int other_a = 0; other_a < total_agents; other_a++)
		if (other_a != a && inst->agents[other_a].goal == v)
			at_goal = inst->SP_lengths[other_a] + delta;
	end_t = min(inst->SP_lengths[a] + delta - length_from_goal[a][inst->map[v.x][v.y]], at_goal);

	return pair<int,int>(start_t, end_t);
}

void SubgraphMaker::BFS(vector<int>& length_from, Vertex start)
{
	queue<Vertex> que;

	length_from[inst->map[start.x][start.y]] = 0;
	que.push(start);

	while(!que.empty())
	{
		Vertex v = que.front();
		que.pop();

		if (v.x > 0 && computed_map[v.x - 1][v.y] != -1 && length_from[inst->map[v.x - 1][v.y]] == -1)
		{
			length_from[inst->map[v.x - 1][v.y]] = length_from[inst->map[v.x][v.y]] + 1;
			que.push({v.x - 1, v.y});
		}
		if (v.x < inst->height - 1 && computed_map[v.x + 1][v.y] != -1 && length_from[inst->map[v.x + 1][v.y]] == -1)
		{
			length_from[inst->map[v.x + 1][v.y]] = length_from[inst->map[v.x][v.y]] + 1;
			que.push({v.x + 1, v.y});
		}
		if (v.y > 0 && computed_map[v.x][v.y - 1] != -1 && length_from[inst->map[v.x][v.y - 1]] == -1)
		{
			length_from[inst->map[v.x][v.y - 1]] = length_from[inst->map[v.x][v.y]] + 1;
			que.push({v.x, v.y - 1});
		}
		if (v.y < inst->width - 1 && computed_map[v.x][v.y + 1] != -1 && length_from[inst->map[v.x][v.y + 1]] == -1)
		{
			length_from[inst->map[v.x][v.y + 1]] = length_from[inst->map[v.x][v.y]] + 1;
			que.push({v.x, v.y + 1});
		}
	}
}

/**************** LEGACY *****************/
/*
// vertex_x, vertex_y, agent, time
void SubgraphMaker::MakeTEG_mks(int agents, int mks)
{
	time_expanded_graph = vector<vector<vector<vector<int> > > >(computed_map.size(), vector<vector<vector<int> > >(computed_map[0].size(), vector<vector<int> >(agents)));

	for (size_t x = 0; x < computed_map.size(); x++)
		for (size_t y = 0; y < computed_map[x].size(); y++)
		{
			if (computed_map[x][y] == -1)
				continue;
			for (size_t a = 0; a < time_expanded_graph[x][y].size(); a++)
			{
				if (use_individual_maps && individual_maps[a][x][y] == -1)
					continue;
				for (size_t t = 0; t <= mks; t++)
					if (inst->length_from_start[a][inst->map[x][y]] <= t && inst->length_from_goal[a][inst->map[x][y]] <= mks - t)
						time_expanded_graph[x][y][a].push_back(t);
			}
		}
}

// vertex_x, vertex_y, agent, time
void SubgraphMaker::MakeTEG_soc(int agents, int bonus_cost)
{
	time_expanded_graph = vector<vector<vector<vector<int> > > >(computed_map.size(), vector<vector<vector<int> > >(computed_map[0].size(), vector<vector<int> >(agents)));

	int mks = inst->GetMksLB(agents) + bonus_cost;

	for (size_t x = 0; x < computed_map.size(); x++)
		for (size_t y = 0; y < computed_map[x].size(); y++)
		{
			if (computed_map[x][y] == -1)
				continue;
			for (size_t a = 0; a < time_expanded_graph[x][y].size(); a++)
			{
				if (use_individual_maps && individual_maps[a][x][y] == -1)
					continue;
				for (size_t t = 0; t <= inst->SP_lengths[a] + bonus_cost; t++)
					if (inst->length_from_start[a][inst->map[x][y]] <= t && inst->length_from_goal[a][inst->map[x][y]] <= inst->SP_lengths[a] + bonus_cost - t)
						time_expanded_graph[x][y][a].push_back(t);
				if (inst->agents[a].goal.x == x && inst->agents[a].goal.y == y)
					for (size_t t = inst->SP_lengths[a] + bonus_cost + 1; t <= mks; t++)
						time_expanded_graph[x][y][a].push_back(t);
			}
		}
}

*/