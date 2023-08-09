CC = g++
CFLAGS = -std=c++11 -O3
S_DIR = src
B_DIR = build/bin

_DEPS = strategy.hpp instance.hpp subgraph_maker.hpp asp_solver.hpp isolver.hpp
DEPS = $(patsubst %,$(S_DIR)/%,$(_DEPS))

_OBJ = main.o strategy.o instance.o subgraph_maker.o asp_solver.o
OBJ = $(patsubst %,$(abspath $(S_DIR))/%,$(_OBJ))

subgraph_framework: $(OBJ)
	mkdir -p build/bin
	$(CC) $(CFLAGS) -o $(B_DIR)/$@ $^ -lstdc++fs

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(S_DIR)/*.o $(B_DIR)/subgraph_framework

test: subgraph_framework
	$(B_DIR)/subgraph_framework -i resources/scenarios/test2.scen -s b -b soc-iter -d

experiment: subgraph_framework
	sh experiment.sh
