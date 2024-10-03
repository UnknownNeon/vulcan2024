#include "application.h"

Application::Application()
{
	util.compile_shaders("Dep/Shader/simple_shader.vert", "Dep/Shader/simple_shader.frag");
	auto vert = util.read_from_file("Dep/Shader/vert.spv");
	auto frag = util.read_from_file("Dep/Shader/frag.spv");

	main_window.create_graphics_pipeling(frag, vert);
	main_window.run();
}