# mapf-corridors-sat
Solving MAPF under map-to-corridor transformation with SAT


Usage of the framework

./corridor_framework [-h] -b base_algorithm -i agents_file -s strategy [-t timeout]
        -h                  : prints help and exits
        -b base_algorithm   : base algorithm to be used. Available options are sat|asp|asp-teg
        -i agents_file      : path to an agents file
        -s strategy         : strategy to be used. Available options are b|m|p|c
        -t timeout          : timeout of the computation. Default value is 300s
