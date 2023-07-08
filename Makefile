EXE = vox_render
ODIR = obj
IMGUI_DIR = imgui

SOURCES = main.cpp glad/glad.c lib/tinyxml2.cpp
SOURCES += src/camera.cpp src/shader.cpp src/vao.cpp src/vbo.cpp src/ebo.cpp src/skybox.cpp src/greedy_mesh.cpp
SOURCES += src/xml_loader.cpp src/vox_loader.cpp src/voxel_render.cpp src/voxbox_render.cpp src/water_render.cpp src/rope_render.cpp
SOURCES += src/texture.cpp src/mesh.cpp src/utils.cpp src/light.cpp src/shadowmap.cpp
#SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
#SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp

OBJS = $(addprefix obj/, $(addsuffix .o, $(basename $(notdir $(SOURCES)))))
UNAME_S := $(shell uname -s)

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -Ilib
CXXFLAGS += -Wno-missing-field-initializers
CXXFLAGS += -std=c++11 -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backend

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	CXXFLAGS += -Wno-unused-result
	LIBS = -lglfw `pkg-config --static --libs glfw3`
	CXXFLAGS += `pkg-config --cflags glfw3`
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	CXXFLAGS += -Wno-array-bounds
	LIBS = -lglfw3 -lgdi32 -lopengl32 -limm32
#	CXXFLAGS += `pkg-config --cflags glfw3`
	CXXFLAGS += -IC:/msys64/mingw64/include
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

$(ODIR)/%.o: $(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(IMGUI_DIR)/backend/%.cpp $(IMGUI_DIR)/backend/%.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)
