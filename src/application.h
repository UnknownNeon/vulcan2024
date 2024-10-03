#pragma once
#include  "window.h"
#include "util.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Application {

public:

	Application();
private:

	utility util;
	rae::window main_window{HEIGHT , WIDTH , "Window 1"};

};