CC = g++
CFLAGS = -std=c++17 -O3
S_DIR = src
B_DIR = build
O_DIR = .objects

PROJECT_NAME = subgraph_framework

_DEPS = strategy.hpp instance.hpp subgraph_maker.hpp asp_solver.hpp isolver.hpp sp_finder.hpp
DEPS = $(patsubst %,$(S_DIR)/%,$(_DEPS))

_OBJ = main.o strategy.o instance.o subgraph_maker.o asp_solver.o sp_finder.o
OBJ = $(patsubst %,$(O_DIR)/%,$(_OBJ))

$(PROJECT_NAME): $(OBJ)
	mkdir -p $(B_DIR)
	$(CC) $(CFLAGS) -o $(B_DIR)/$@ $^ -lstdc++fs

$(O_DIR)/%.o: $(S_DIR)/%.cpp $(DEPS) | $(O_DIR)_exists
	$(CC) $(CFLAGS) -c -o $@ $<

$(O_DIR)_exists:
	mkdir -p $(O_DIR)

clean:
	rm -rf $(O_DIR) 
	rm -rf $(B_DIR)
	rm -f run/*

test: $(PROJECT_NAME)
	$(B_DIR)/$(PROJECT_NAME) -i resources/scenarios/empty08-1.scen -s c -b mks -a 5 -k 0 -p random

valgrind: $(PROJECT_NAME)
	valgrind --leak-check=full \
	--show-leak-kinds=all \
	--track-origins=yes \
	--verbose \
	--log-file=valgrind-out.txt \
	$(B_DIR)/$(PROJECT_NAME) -i resources/scenarios/empty08-1.scen -s c -b mks -a 5 -k 0 -p random

experiment: subgraph_framework
	sh experiment.sh
