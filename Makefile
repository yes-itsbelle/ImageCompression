# --- Makefile for Image Compression Project ---

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -Wl,--subsystem,console

# Source and executable
SRCS = main.cpp
EXE = image_compressor.exe

# Phony targets
.PHONY: all clean

# The 'all' target is the default. It builds the executable.
all: $(EXE)

# Rule to build the executable from the source file.
# The `LDFLAGS` explicitly tells the linker to create a console application.
$(EXE): $(SRCS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(SRCS) -o $@

# Clean up the build files.
clean:
	rm -f $(EXE)
