SRC := $(wildcard *.c)
BIN_DIR ?= .
TARGET := $(BIN_DIR)/$(shell basename $(CURDIR))

all: $(TARGET)

$(TARGET) : $(SRC)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $(TARGET) -O3 -ffast-math -mfpmath=387 -lm -lbladeRF
