#ifndef  TEXTURE
#define TEXTURE
#include "stb_image.h"

class Texture2D {

	unsigned long long device_size;
	unsigned char* pixels;
	int t_height, t_width, t_channels;

	bool is_mem_freed = false;

public:
	Texture2D(const char* filename) {

		this->pixels = stbi_load(filename, &this->t_width, &this->t_height, &this->t_channels, STBI_rgb_alpha); //stbi_rgb_alpha is 4 bytes long 
		this->device_size = t_height * t_width * 4; //hence 4 here 

		if (!pixels) {
			std::cout << "CANNNOT OPEN IMAGE FILE "<< std::endl;
			throw std::runtime_error("Failed to load texture ");
		}

	}

	void free_pixels() { stbi_image_free(this->pixels); this->is_mem_freed = true; }


	~Texture2D() {
		if(!is_mem_freed)
			stbi_image_free(this->pixels);
	}

	inline unsigned long long get_device_size() { return this->device_size; };
	inline unsigned char* get_pixel_ptr() { return this->pixels; };
	inline int get_height() { return this->t_height; }
	inline int get_width() { return this->t_width; }
	inline int get_channels() { return this->t_channels; }
};


#endif // ! TEXTURE
