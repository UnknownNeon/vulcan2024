//TODO note :Uptil chrono
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <optional>
#include <set>
#include <cstdint> 
#include <limits> 
#include <algorithm>
#include <array>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace rae {

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct Vertex {
		glm::vec2 positions;
		glm::vec3 color;

		static VkVertexInputBindingDescription getBindingDescription() {

			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions;
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, positions);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);
			return attributeDescriptions;

		}
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats{};
		std::vector<VkPresentModeKHR> presentModes{};
	};

	class window {

		GLFWwindow* win;
		
		bool enableValidationLayers = false;
		//Validation Layer :
		std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

		//Logical Device Extensions :
		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		//Physical Device Extensions :
		std::vector<const char*> extensions;

		//Vulkan and Validation
		VkInstance vulkan_instance;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT debugMessenger;

		//Surface Rendering 
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		VkDeviceQueueCreateInfo queueCreateInfo{};
		VkSurfaceKHR surface;

		//Device 
		VkDevice logicalDevice{};
		rae::QueueFamilyIndices queue_indices;
		VkPhysicalDeviceFeatures deviceFeatures{};


		VkQueue presentQueue;
		VkQueue graphicsQueue;
		

		//swap chain
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkSwapchainKHR swapChain;
		SwapChainSupportDetails swapChainSupport{};
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainExtent;

		
		void create_vulkan();		// Incomplete Validation During Creation and destruction
		void setup_debug_messenger();
		void pick_physical_device(); // Incomplete sutability of physical Device 
		
		//Device related:
		rae::QueueFamilyIndices findQueueFamily(VkPhysicalDevice device);
		void createLogicalDevice();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);

		//Surface and SWAP CHAIN 
		void createSwapChain();
		void create_surface(); //This can be Implemented natively . Sinc GLFW we dont need native code 
		void createImageViews();
		rae::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		//Validation Layer related:
		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);


		//Create Graphics Pipeline  ;
		void create_render_pass();
		VkShaderModule createShaderModule(const std::vector<char>& module); //Shader Module 
		void add_shader_to_pipeline();
		void continue_pipeline_creation();


		std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineShaderStageCreateInfo shaderStages[2];
		VkShaderModule fragmentShaderModule;
		VkShaderModule vertexShaderModule;

		//Pipeline Layout i.e Uniforms 
		VkDescriptorSetLayout descriptorSetLayout;
		VkPipelineLayout pipelineLayout{};
		//Render Pass
		VkRenderPass renderPass;
		//
		VkPipeline graphicsPipeline;

		//Frame buffer
		void createFramebuffer();
		std::vector<VkFramebuffer> swapChainFramebuffers;

		//Command Pools and Buffers
		VkCommandPool command_pool;
		void create_command_pool();

		std::vector<VkCommandBuffer> command_buffers;
		void create_command_buffers();

		//After the command pools have been created we can Write to the Command Buffers 

		void record_command_buffer(VkCommandBuffer command_buffer, uint32_t imageIndex);


		//Frame Drawing
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector <VkFence> inFlightFences;
		

		void create_sync_objs();

		void draw_frame();
		uint32_t currentFrame = 0;


		//Swap Chain Recreation Because a change in surface or Framebuffer may Happen

		void recreate_swap_chain();
		void clean_up_swapchain();

		bool isFrameBufferResized = false;

		//Memory Related 
		
		uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties);

		//Vertex Buffers 
		VkBuffer vertex_buffer;
		VkDeviceMemory vertex_buffer_memory;
		//Index Buffers 
		VkBuffer index_buffer;
		VkDeviceMemory index_buffer_memory;

		const std::vector<rae::Vertex> vertices = {
					//Pos , Color
					{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
					{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
					{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
					{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		const std::vector<uint16_t> indices = {
					0, 1, 2, 2, 3, 0
		};

		void create_index_buffers();
		void create_vertex_buffers();
		void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		void copy_buffer(VkBuffer source_buffer, VkBuffer destination_buffer, VkDeviceSize size);

		//Uniforms and Descriptor sets 
		void create_descriptor_set_layout();
		void create_uniform_buffers();
		void update_uniform_buffer(uint32_t currentImage);

		void create_descriptor_pool();
		void create_descriptor_set();

		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		//Own
		std::vector<VkBuffer> vertexBuffers;

	public:

		inline void setResized() { this->isFrameBufferResized = true; } 

		const int MAX_FRAMES_PROCESS = 2;

		//Exposing private members 
		void create_graphics_pipeling(const std::vector<char>& fragment_spv_in_stdvector , const std::vector<char>& vertex_spv_in_stdvector) {
			
			this->create_vulkan();
			this->fragmentShaderModule = createShaderModule(fragment_spv_in_stdvector);
			this->vertexShaderModule = createShaderModule(vertex_spv_in_stdvector);
			this->create_descriptor_set_layout();
			this->add_shader_to_pipeline();
			this->continue_pipeline_creation();
			this->createFramebuffer();
			this->create_command_pool();
			this->create_vertex_buffers();
			this->create_index_buffers();
			this->create_uniform_buffers();
			this->create_descriptor_pool();
			this->create_descriptor_set();
			this->create_command_buffers();
			this->create_sync_objs();

		}

		void run();

		bool WireframeMode = false;
		inline bool isWindowClosed() { return glfwWindowShouldClose(win); };

		window(int height, int width, const char* windowName);
		~window();

	};	

	

	
}
