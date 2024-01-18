#include <filesystem>

#include "instance.hpp"

using namespace std;

Instance::Instance(string map_dir, string agents_dir, string a_f, string path)
{
	agents_file = a_f;
	path_type = path;
	LoadAgents(agents_dir.append("/").append(agents_file), map_dir);
}

void Instance::LoadAgents(string agents_path, string map_dir)
{
	// Read input
	bool map_loaded = false;
	ifstream in;
	in.open(agents_path);
	if (!in.is_open())
	{
		cout << "Could not open scenario file " << agents_path << endl;
		return;
	}

	char c_dump;
	string line;
	getline(in, line); // first line - version

	while (getline(in, line))
	{
		stringstream ssline(line);
		string part;
		vector<string> parsed_line;
		while (getline(ssline, part, '\t'))
			parsed_line.push_back(part);

		if (!map_loaded)
		{
			LoadMap(map_dir.append("/").append(parsed_line[1]));
			map_loaded = true;
		}

		Agent new_agent;
		new_agent.start = {stoi(parsed_line[5]), stoi(parsed_line[4])};
		new_agent.goal = {stoi(parsed_line[7]), stoi(parsed_line[6])};

		agents.push_back(new_agent);
	}

	shortest_paths.resize(agents.size());
	length_from_start = vector<vector<int> >(agents.size(), vector<int>(number_of_vertices, -1));
	length_from_goal = vector<vector<int> >(agents.size(), vector<int>(number_of_vertices, -1));

	mks_LBs = vector<int>(agents.size() + 1, -1);
	soc_LBs = vector<int>(agents.size() + 1, -1);

	in.close();
}

void Instance::LoadMap(string map_path)
{
	ifstream in;
	in.open(map_path);
	if (!in.is_open())
	{
		cout << "Could not open map file " << map_path << endl;
		return;
	}

	char c_dump;
	string s_dump;
	getline(in, s_dump); // first line - type

	in >> s_dump >> height;
	in >> s_dump >> width;
	in >> s_dump; // map
	
	// graph
	map = vector<vector<int> >(height, vector<int>(width, -1));
	number_of_vertices = 0;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			in >> c_dump;
			if (c_dump == '.')
			{
				map[i][j] = number_of_vertices;
				number_of_vertices++;
			}
		}
	}

	in.close();
}

void Instance::ComputeShortestPaths(int number_of_agents_to_compute)
{
	path_compute_time = 0;

	auto time_start = chrono::high_resolution_clock::now();
	vector<pair<pair<size_t, size_t>, pair<size_t, size_t>>> agents_to_process;

	for (size_t i = 0; i < number_of_agents_to_compute; i++)
	{
		BFS(length_from_start[i], agents[i].start);
		BFS(length_from_goal[i], agents[i].goal);

		SP_lengths.push_back(length_from_start[i][map[agents[i].goal.x][agents[i].goal.y]]);

		agents_to_process.push_back(make_pair(make_pair(agents[i].start.x + 1, agents[i].start.y + 1),make_pair(agents[i].goal.x + 1, agents[i].goal.y + 1)));
	}

	PathFinderI* pathfinder = NULL;
	if (!path_type.compare("biased"))
		pathfinder = new Biased();
	if (!path_type.compare("random"))
		pathfinder = new TrullyRandom();
	if (!path_type.compare("cross"))
		pathfinder = new WithoutCrossing();
	if (!path_type.compare("time"))
		pathfinder = new WithoutCrossingAtSameTimes();

	if (pathfinder == NULL)	// default approach
	{	
		path_type = "biased";
		pathfinder = new Biased();
	}

	vector<vector<pair<size_t, size_t>>> output_paths(agents_to_process.size());

	vector<vector<size_t>> ref_map = vector<vector<size_t> >(height + 2, vector<size_t>(width + 2, 0));
	for (size_t i = 0; i < height; i++)
		for (size_t j = 0; j < width; j++)
			ref_map[i + 1][j + 1] = map[i][j] + 1;

	pathfinder->compute_shortest_paths(ref_map, output_paths, agents_to_process);

	for (size_t i = 0; i < output_paths.size(); i++)
	{
		vector<Vertex> vc; 
		for (size_t j = 0; j < output_paths[i].size(); j++)
		{
			vc.push_back({int(output_paths[i][j].first - 1), int(output_paths[i][j].second - 1)});
		}
		shortest_paths[i] = vc;
	}

	delete pathfinder;

	auto time_end = chrono::high_resolution_clock::now();

	path_compute_time = chrono::duration_cast<chrono::milliseconds>(time_end - time_start).count();
} 

void Instance::BFS(vector<int>& length_from, Vertex start)
{
	queue<Vertex> que;

	length_from[map[start.x][start.y]] = 0;
	que.push(start);

	while(!que.empty())
	{
		Vertex v = que.front();
		que.pop();

		if (v.x > 0 && map[v.x - 1][v.y] != -1 && length_from[map[v.x - 1][v.y]] == -1)
		{
			length_from[map[v.x - 1][v.y]] = length_from[map[v.x][v.y]] + 1;
			que.push({v.x - 1, v.y});
		}
		if (v.x < height - 1 && map[v.x + 1][v.y] != -1 && length_from[map[v.x + 1][v.y]] == -1)
		{
			length_from[map[v.x + 1][v.y]] = length_from[map[v.x][v.y]] + 1;
			que.push({v.x + 1, v.y});
		}
		if (v.y > 0 && map[v.x][v.y - 1] != -1 && length_from[map[v.x][v.y - 1]] == -1)
		{
			length_from[map[v.x][v.y - 1]] = length_from[map[v.x][v.y]] + 1;
			que.push({v.x, v.y - 1});
		}
		if (v.y < width - 1 && map[v.x][v.y + 1] != -1 && length_from[map[v.x][v.y + 1]] == -1)
		{
			length_from[map[v.x][v.y + 1]] = length_from[map[v.x][v.y]] + 1;
			que.push({v.x, v.y + 1});
		}
	}
}

int Instance::GetMksLB(int ags)
{
	if (mks_LBs[ags] >= 0)
		return mks_LBs[ags];
	int LB = 0;
	for (size_t i = 0; i < ags; i++)
		LB = max(LB, SP_lengths[i]);
	mks_LBs[ags] = LB;
	return LB;
}

int Instance::GetSocLB(int ags)
{
	if (soc_LBs[ags] >= 0)
		return soc_LBs[ags];
	int LB = 0;
	for (size_t i = 0; i < ags; i++)
		LB += SP_lengths[i];
	soc_LBs[ags] = LB;
	return LB;
}

/* DEBUG */

void Instance::DebugPrint(vector<vector<int> >& map_to_print)
{
	for (size_t i = 0; i < map_to_print.size(); i++)
	{
		for (size_t j = 0; j < map_to_print[i].size(); j++)
			if (map[i][j] == -1)
				cout << "@";
			else if (map_to_print[i][j] == -1)
				cout << "#";
			else
				cout << ".";

			//cout << map_to_print[i][j] << " ";
		cout << endl;
	}

	/*cout << endl << "Agents:" << endl;
	for (size_t i = 0; i < agents.size(); i++)
	{
		cout << "Agent " << i << " start: (" << agents[i].start.x << "," << agents[i].start.y << ") = vertice nr " << map_to_print[agents[i].start.x][agents[i].start.y] << endl;
		cout << "Agent " << i << " goal: (" << agents[i].goal.x << "," << agents[i].goal.y << ") = vertice nr " << map_to_print[agents[i].goal.x][agents[i].goal.y] << endl;
	}*/
}

void Instance::DebugPrint(vector<int>& vc_to_print)
{
	for (size_t i = 0; i < vc_to_print.size(); i++)
		cout << vc_to_print[i] << " ";
	cout << endl;
}

void Instance::DebugPrint(vector<Vertex>& vc_to_print)
{
	for (size_t i = 0; i < vc_to_print.size(); i++)
		cout << "(" << vc_to_print[i].x << ", " << vc_to_print[i].y << ") ";
	cout << endl;
}