CC = g++
CFLAGS = -std=c++11 -O3
S_DIR = src
B_DIR = build/bin

_DEPS = strategy.hpp instance.hpp corridor_maker.hpp sat_solver.hpp asp_solver.hpp isolver.hpp
DEPS = $(patsubst %,$(S_DIR)/%,$(_DEPS))

_OBJ = main.o strategy.o instance.o corridor_maker.o sat_solver.o asp_solver.o
OBJ = $(patsubst %,$(abspath $(S_DIR))/%,$(_OBJ))

corridor_framework: $(OBJ)
	mkdir -p build/bin
	$(CC) $(CFLAGS) -o $(B_DIR)/$@ $^ -lstdc++fs

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(S_DIR)/*.o $(B_DIR)/corridor_framework
	rm -f encodings/asp/instance.lp encodings/asp/output_asp
	rm -f encodings/sat/instance.pi encodings/sat/output_sat

test: corridor_framework
	$(B_DIR)/corridor_framework -i resources/instances/scenarios/empty-128-128-condensed-4.scen -s p -b sat -t 300

experiment: corridor_framework
	sh experiment.sh
