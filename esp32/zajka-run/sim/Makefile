# Simulateur PC (meme code de jeu que le firmware) : make -C sim
SRC := $(wildcard ../src/game/*.cpp) sim.cpp
OUT := ../build/sim

$(OUT): $(SRC) $(wildcard ../src/game/*.h) ../src/game/gen/assets_data.inc
	@mkdir -p ../build
	$(CXX) -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter -o $@ $(SRC)

clean:
	rm -f $(OUT)
