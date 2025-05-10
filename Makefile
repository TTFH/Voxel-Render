TARGET = vox_render

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -Wpedantic -O3 #-g -D_BLENDER
CXXFLAGS += -Wno-missing-field-initializers
CXXFLAGS += `pkg-config --cflags glfw3`
LIBS = `pkg-config --libs glfw3 --static`

SOURCES = main_sv.cpp glad/glad.c lib/tinyxml2.cpp
SOURCES += src/camera.cpp src/ebo.cpp src/light.cpp src/overlay.cpp
SOURCES += src/postprocessing.cpp src/render_boundary.cpp src/render_mesh.cpp
SOURCES += src/render_rope.cpp src/render_vox_greedy.cpp src/render_vox_hex.cpp
SOURCES += src/render_vox_rtx.cpp src/render_voxbox.cpp src/render_water.cpp
SOURCES += src/scene_loader.cpp src/shader.cpp src/shadow_volume.cpp src/skybox.cpp
SOURCES += src/render_interface.cpp src/utils.cpp src/vao.cpp src/vbo.cpp src/vox_loader.cpp
SOURCES += imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp
SOURCES += imgui/backends/imgui_impl_glfw.cpp imgui/backends/imgui_impl_opengl3.cpp

OBJDIR = obj
OBJS = $(SOURCES:.cpp=.o)
OBJS := $(OBJS:.c=.o)
OBJS := $(addprefix $(OBJDIR)/, $(notdir $(OBJS)))

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux)
	ECHO_MESSAGE = "Linux"
	CXXFLAGS += -Wno-unused-result
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lopengl32 -limm32 -static not_td.res
endif

ifeq ($(UNAME_S), Darwin)
	ECHO_MESSAGE = "MacOS"
endif

.PHONY: all clean rebuild

all: $(TARGET)
	@echo Build complete for $(ECHO_MESSAGE)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: src/%.cpp src/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: lib/%.cpp lib/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: glad/%.c glad/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: imgui/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: imgui/backend/%.cpp imgui/backend/%.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

rebuild: clean all

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o
