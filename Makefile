# Quick and (*really*) dirty build for examples.
SRC_DIR := simple_examples
BIN_DIR := bin

EXAMPLES := $(patsubst $(SRC_DIR)/%,%,$(wildcard $(SRC_DIR)/*))
EXAMPLES_BIN := $(addprefix $(BIN_DIR)/,$(EXAMPLES))

CFLAGS := -Wall -Wextra -Wno-unused-parameter -O0 -ggdb3
LDFLAGS := -lbladeRF -lm

all: $(EXAMPLES_BIN)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/% : $(BIN_DIR)
	@$(MAKE) -C $(SRC_DIR)/$(notdir $@) \
		BIN_DIR="$(abspath $(dir $@))" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)"

clean:
	rm -rf $(BIN_DIR)

.PHONY: clean
