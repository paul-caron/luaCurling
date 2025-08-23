# --------------------------------------------------------------
#  Variables
# --------------------------------------------------------------
CXX        := g++                      # or clang++
CXXFLAGS   := -std=c++17 -Wall -Wextra -pedantic
LDFLAGS    := -lcurl

# Lua 5.4 – replace with whichever Lua version you use
LUACFLAGS  := $(shell pkg-config --cflags lua5.4 2>/dev/null)
LUALDFLAGS := $(shell pkg-config --libs lua5.4    2>/dev/null)

# Path to Sol2 (the folder that contains the `sol/` sub‑folder)
SOL2_INC   := ./

CXXFLAGS   += $(LUACFLAGS) -I$(SOL2_INC)
LDFLAGS    += $(LUALDFLAGS)

# --------------------------------------------------------------
#  Targets
# --------------------------------------------------------------
TARGET  := sample
SRCS    := main.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGET)
