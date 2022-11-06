EXE = vox_render
ODIR = obj
SOURCES = main.cpp glad/glad.c lib/tinyxml2.cpp
SOURCES += src/camera.cpp src/shader.cpp src/vao.cpp src/vbo.cpp src/ebo.cpp src/skybox.cpp src/greedy_mesh.cpp
SOURCES += src/xml_loader.cpp src/vox_loader.cpp src/voxel_render.cpp src/voxbox_render.cpp src/water_render.cpp rope_render.cpp

OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -Ilib -O3

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	CXXFLAGS += -Wno-unused-result
	LIBS = -lglfw
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	CXXFLAGS += -Wno-array-bounds
	LIBS = -lglfw3
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------
.PHONY: all clean

$(ODIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: src/%.cpp src/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: lib/%.cpp lib/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: glad/%.c glad/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
