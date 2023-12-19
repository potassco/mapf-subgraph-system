CC = g++
CFLAGS = -std=c++11 -O3
S_DIR = src
B_DIR = build
O_DIR = .objects

_DEPS = strategy.hpp instance.hpp subgraph_maker.hpp asp_solver.hpp isolver.hpp
DEPS = $(patsubst %,$(S_DIR)/%,$(_DEPS))

_OBJ = main.o strategy.o instance.o subgraph_maker.o asp_solver.o
OBJ = $(patsubst %,$(O_DIR)/%,$(_OBJ))

subgraph_framework: $(OBJ)
	mkdir -p $(B_DIR)
	$(CC) $(CFLAGS) -o $(B_DIR)/$@ $^ -lstdc++fs

$(O_DIR)/%.o: $(S_DIR)/%.cpp $(DEPS) | $(O_DIR)_exists
	$(CC) $(CFLAGS) -c -o $@ $<

$(O_DIR)_exists:
	mkdir -p $(O_DIR)

clean:
	rm -rf $(O_DIR) 
	rm -rf $(B_DIR)

test: subgraph_framework
	$(B_DIR)/subgraph_framework -i resources/scenarios/test2.scen -s b -b soc-iter -d

experiment: subgraph_framework
	sh experiment.sh
