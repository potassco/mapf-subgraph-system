CC = g++
CFLAGS = -std=c++11 -O3
S_DIR = src
B_DIR = models

_DEPS = strategy.hpp instance.hpp corridor_maker.hpp sat_solver.hpp asp_solver.hpp isolver.hpp
DEPS = $(patsubst %,$(S_DIR)/%,$(_DEPS))

_OBJ = main.o strategy.o instance.o corridor_maker.o sat_solver.o asp_solver.o
OBJ = $(patsubst %,$(S_DIR)/%,$(_OBJ))

corridor_framework: $(OBJ)
	$(CC) $(CFLAGS) -o $(B_DIR)/$@ $^

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(S_DIR)/*.o $(B_DIR)/corridor_framework
	rm -f models/ASP_model/instance.lp models/ASP_model/output_asp
	rm -f models/SAT_model/instance.pi models/SAT_model/output_sat

test: corridor_framework
	$(B_DIR)/corridor_framework -i resources/instances/empty-32-32-condensed-0.scen -s p -b asp -t 1

experiment: corridor_framework
	sh experiment.sh
