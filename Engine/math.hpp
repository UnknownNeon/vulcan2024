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

	struct mat4_f {
		float mat[4][4];
		mat4_f() {
			this->mat[0][0] = 0.0f;
			this->mat[0][1] = 0.0f;
			this->mat[0][2] = 0.0f;
			this->mat[0][3] = 0.0f;
			this->mat[1][0] = 0.0f;
			this->mat[1][1] = 0.0f;
			this->mat[1][2] = 0.0f;
			this->mat[1][3] = 0.0f;
			this->mat[2][0] = 0.0f;
			this->mat[2][1] = 0.0f;
			this->mat[2][2] = 0.0f;
			this->mat[2][3] = 0.0f;
			this->mat[3][0] = 0.0f;
			this->mat[3][1] = 0.0f;
			this->mat[3][2] = 0.0f;
			this->mat[3][3] = 0.0f;
		}
		mat4_f(float f) {
			this->mat[0][0] = f;
			this->mat[0][1] = 0.0f;
			this->mat[0][2] = 0.0f;
			this->mat[0][3] = 0.0f;
			this->mat[1][0] = 0.0f;
			this->mat[1][1] = f;
			this->mat[1][2] = 0.0f;
			this->mat[1][3] = 0.0f;
			this->mat[2][0] = 0.0f;
			this->mat[2][1] = 0.0f;
			this->mat[2][2] = f;
			this->mat[2][3] = 0.0f;
			this->mat[3][0] = 0.0f;
			this->mat[3][1] = 0.0f;
			this->mat[3][2] = 0.0f;
			this->mat[3][3] = f;
		}
	};

}