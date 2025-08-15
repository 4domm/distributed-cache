BUILD_DIR = build
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=Release
SHARD ?= 0           
CFG   ?= config.json
all: $(BUILD_DIR)/Makefile
	cmake --build $(BUILD_DIR)

$(BUILD_DIR)/Makefile:
	cmake -B $(BUILD_DIR) -S . $(CMAKE_FLAGS)

run: all
	./$(BUILD_DIR)/timkv $(SHARD) $(CFG)

clean:
	rm -rf $(BUILD_DIR)

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --num-callers=20 ./$(BUILD_DIR)/timkv $(SHARD) $(CFG)
helgrind: all
	valgrind --tool=helgrind ./$(BUILD_DIR)/timkv $(SHARD) $(CFG)