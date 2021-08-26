CC = g++
CFLAGS = -std=c++11 -O3
S_DIR = src
B_DIR = bin

_DEPS = strategy.hpp instance.hpp corridor_maker.hpp sat_solver.hpp isolver.hpp
DEPS = $(patsubst %,$(S_DIR)/%,$(_DEPS))

_OBJ = main.o strategy.o instance.o corridor_maker.o sat_solver.o
OBJ = $(patsubst %,$(S_DIR)/%,$(_OBJ))

all: solver

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

solver: $(OBJ)
	$(CC) $(CFLAGS) -o $(B_DIR)/$@ $^

clean:
	rm -f $(S_DIR)/*.o $(B_DIR)/solver

run: solver
	$(B_DIR)/solver -i ../instances/test.scen -s p -b sat -t 120