CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Iinclude
BUILD    := build
TARGET   := fileenc
SRCS     := $(wildcard src/*.cpp)
OBJS     := $(SRCS:src/%.cpp=$(BUILD)/%.o)

.PHONY: all clean test run-example

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(TARGET)
	bash tests/run_tests.sh

clean:
	rm -rf $(BUILD) $(TARGET) logs

-include $(OBJS:.o=.d)
