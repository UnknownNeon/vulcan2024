#pragma once 
#include <cstdint>

namespace math {

	struct vec2_i
	{
		uint32_t x;
		uint32_t y;

		vec2_i operator+(vec2_i& temp) {
			return vec2_i{ this->x + temp.x , this->y + temp.y };
		}
	};

	struct vec3_i {
		uint32_t x;
		uint32_t y;
		uint32_t z;

		vec3_i operator+(vec3_i& temp) {
			return vec3_i{ this->x + temp.x , this->y + temp.y , this->z + temp.z };
		}
	};
	struct vec2_f {
		float x; //is Float IEEE 754 format..
		float y;

		vec2_f operator+(vec2_f& temp) {
			return vec2_f{ this->x + temp.x , this->y + temp.y };
		}
	};

	struct vec3_f {
		float x;
		float y;
		float z;

		vec3_f operator+(vec3_f& temp) {
			return vec3_f{ this->x + temp.x , this->y + temp.y , this->z + temp.z };
		}
	};
}