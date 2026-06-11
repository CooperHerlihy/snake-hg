SRC_DIR := .
BUILD_DIR := $(SRC_DIR)/build

STD := -std=c++17 -MMD -MP
WARNINGS := -Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic

DEBUG_CONFIG := -g -O0 -Wno-cpp -fsanitize=undefined -fno-exceptions -fno-rtti
RELEASE_CONFIG := -O3 -DNDEBUG -fno-exceptions -fno-rtti
CONFIG := $(DEBUG_CONFIG)

INCLUDES := \
	-I$(SRC_DIR)/vendor/hurdygurdy/include

.PHONY: all debug release clean

all: $(BUILD_DIR)/snake

debug:
	$(MAKE) CONFIG="$(DEBUG_CONFIG)"

release:
	$(MAKE) CONFIG="$(RELEASE_CONFIG)"

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/libhurdygurdy.a: | $(BUILD_DIR)
	$(MAKE) -C $(SRC_DIR)/vendor/hurdygurdy CONFIG="$(CONFIG)"
	cp $(SRC_DIR)/vendor/hurdygurdy/build/libhurdygurdy.a $(BUILD_DIR)/libhurdygurdy.a

$(BUILD_DIR)/%.o: $(SRC_DIR)/src/%.cpp | $(BUILD_DIR)
	c++ $(STD) $(CONFIG) $(WARNINGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%: $(BUILD_DIR)/%.o $(BUILD_DIR)/libhurdygurdy.a | $(BUILD_DIR)
	c++ $(STD) $(CONFIG) $(WARNINGS) -o $@ $< -L$(BUILD_DIR) -lhurdygurdy -lSDL3

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C $(SRC_DIR)/vendor/hurdygurdy clean

-include $(BUILD_DIR)/*.d

