#include "window.h"

static void frameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto window_ptr = reinterpret_cast<rae::window*>(glfwGetWindowUserPointer(window));
	if (window_ptr)
	{
		window_ptr->setResized();
	}
	else
	{
		std::cerr << "Error: window_ptr is nullptr!" << std::endl;
	}
}

//MEMBER METHODS
void rae::window::create_vulkan()
{
	//The App info;
	VkApplicationInfo appInfo{};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Main Game Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "RAE";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	/*
	* GLfW nedds some extensions from the VULKAN to run
	* so we get the exensions from the lines below
	*/
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//Physical device extensions
	for (int i = 0; i < glfwExtensionCount; i++) {
		this->extensions.push_back(glfwExtensions[i]);
	}
	VkInstanceCreateInfo createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;
	
	// Validation layer

#ifdef NDEBUG
	enableValidationLayers = false; 
#else
	enableValidationLayers = true;


	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	bool found_layer_flag = false;
	for (const char* layerName : this->validationLayers) {

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				std::cout << "LAYER FOUND VALIDATION : " << layerName << std::endl;
				found_layer_flag = true;
				break;
			}
			
		}
		if (!found_layer_flag)
		{
			std::cout << "NO VALIDATION LAYER FOUND FOR: " << layerName << std::endl;
			std::cout << "Available layers :  " << std::endl;
			for (const auto& layerProperties : availableLayers) {
				std::cout << layerProperties.layerName << std::endl;
			}
			throw std::runtime_error("Validation layer missing ");
		}
	}
#endif
	if (enableValidationLayers) {

		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

	}
	else {
		createInfo.enabledLayerCount = 0;
	}
	//Validation done 

	VkResult result = vkCreateInstance(&createInfo, nullptr, &vulkan_instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vulkan instance!");
	}

	/*Notice How the debug messenger is hooked after a instance of vulkan is created 
	* i.e during creation it is not checked and so during deletion 
	* This problem is addressed in the documentation
	*/

	if (enableValidationLayers) {
		this->setup_debug_messenger();
	}

	//Calling physical device i.e the gpu to use ; 
	this->pick_physical_device();
}

void rae::window::setup_debug_messenger()
{
	//Setting up the debug messenger: 
	VkDebugUtilsMessengerCreateInfoEXT createDebugInfo{};

	createDebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createDebugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createDebugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createDebugInfo.pfnUserCallback = this->debugCallback; //The Actuall callback pointer passiing 
	createDebugInfo.pUserData = nullptr;

	if (CreateDebugUtilsMessengerEXT(vulkan_instance, &createDebugInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger! ");
	}
}

VkResult rae::window::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void rae::window::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL rae::window::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{

	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void rae::window::create_surface()
{
	if (glfwCreateWindowSurface(this->vulkan_instance, this->win, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}
}


void rae::window::pick_physical_device()
{
	uint32_t count = 0;
	vkEnumeratePhysicalDevices(vulkan_instance, &count, nullptr);
	if (count == 0) {
		throw std::runtime_error("No physical device found !");
	}


	
#ifdef NDEBUG
#else
	std::cout << "No of physical device found : " << count  <<  std::endl;
#endif
	std::vector<VkPhysicalDevice> devices(count);
	vkEnumeratePhysicalDevices(vulkan_instance, &count, devices.data());


	//Check the suitability of the device @TOLEAERN

	//End checks

	this->physicalDevice = devices[0];

	//-------------------------------------------
	//-------------------------------------------
	//HAVE TO HAVE A SURFACE BY NOW--------------

	this->create_surface();

	//HAVE TO HAVE A SURFACE BY NOW--------------
	//-------------------------------------------
	//-------------------------------------------

	//Check for Graphics family . [ NOTE : has_value() part of Optional lib ]
	if (this->findQueueFamily(this->physicalDevice).graphicsFamily.has_value() ) {

		//Checking for the physical device
		if (this->physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("NO PHYSICAL DEVICE FOUND !");
		}
		
		//Creating logical device with queueFamilies
		this->createLogicalDevice();
	}
	else {
		throw std::runtime_error("[ERROR] NO QUE FAMILIES ");
	}
}

void rae::window::createLogicalDevice() //MAIN CALLING FUNC.
{
	VkDeviceCreateInfo dCreateInfo{};
	//QUEUE FAMILIES ARE DEVICE SPECIFIC AND ARE HANDLED ABOVE CODE in FindQueueFamilies ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	float priority = 1.0f;				//@TODO : Might make this callable later

	//Multiple queues Pushbacks
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { this->queue_indices.graphicsFamily.value(), this->queue_indices.presentFamily.value() };
	
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &priority;
		queueCreateInfos.push_back(queueCreateInfo);
	}
	//end

	//Creating the Logical Device  {VkDeviceCreateInfo}
	dCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	dCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	dCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	dCreateInfo.pEnabledFeatures = &this->deviceFeatures;

	//Check for Swap-Chain support
	if (this->checkDeviceExtensionSupport(this->physicalDevice)) {
		dCreateInfo.enabledExtensionCount = static_cast<uint32_t>(this->deviceExtensions.size());
		dCreateInfo.ppEnabledExtensionNames = this->deviceExtensions.data();

		this->swapChainSupport = querySwapChainSupport(this->physicalDevice);
		if (!swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty()) {
			
		}
		else {
			std::cout << "[ERROR ]: SWAP CHAIN NOT ADEQUATE " << std::endl;
		}
	}
	else {
		dCreateInfo.enabledExtensionCount = 0; //DEVICE SPECIFIC EXTENSION NOTE {ENABLE RAY TRACING HERE} 
	}
		

	if (enableValidationLayers) {
		dCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		dCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		dCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(this->physicalDevice, &dCreateInfo, nullptr, &this->logicalDevice) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	else {
		this->createSwapChain();
		this->create_image_views();
	}

	
	//Just Saving to global here !
	vkGetDeviceQueue(this->logicalDevice, this->queue_indices.presentFamily.value(), 0, &presentQueue);
	vkGetDeviceQueue(this->logicalDevice, this->queue_indices.graphicsFamily.value(), 0, &graphicsQueue);

	this->create_render_pass();
	
}


rae::QueueFamilyIndices rae::window::findQueueFamily(VkPhysicalDevice device)
{
	VkBool32 presentSupport = false;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	if (queueFamilyCount > 0) {
		int i = 0;
		for (auto &family : queueFamilies) {
			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				this->queue_indices.graphicsFamily = i;
				
				//Checking for Surface Support here:
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
				if (presentSupport) {
					this->queue_indices.presentFamily = i;
				}
			}
			i++;
		}
	}
	return this->queue_indices;
}

void rae::window::createSwapChain()
{
	VkSwapchainCreateInfoKHR swapCreateInfo{};

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);


	uint32_t imageCount = swapChainSupport.capabilities.minImageCount+1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	//Creating the swapChain
	swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapCreateInfo.surface = surface;
	swapCreateInfo.minImageCount = imageCount;
	swapCreateInfo.imageFormat = surfaceFormat.format;
	swapCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapCreateInfo.imageExtent = extent;
	swapCreateInfo.imageArrayLayers = 1;
	swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { this->queue_indices.graphicsFamily.value(), this->queue_indices.presentFamily.value() };

	if (this->queue_indices.graphicsFamily != this->queue_indices.presentFamily) {
		swapCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapCreateInfo.queueFamilyIndexCount = 2;
		swapCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		swapCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapCreateInfo.queueFamilyIndexCount = 0;
		swapCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	swapCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapCreateInfo.presentMode = presentMode;
	swapCreateInfo.clipped = VK_TRUE;
	swapCreateInfo.oldSwapchain = VK_NULL_HANDLE;


	if (vkCreateSwapchainKHR(logicalDevice, &swapCreateInfo, nullptr, &this->swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	//Getting the swap change image Count :
	vkGetSwapchainImagesKHR(this->logicalDevice,this->swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(this->logicalDevice,this->swapChain, &imageCount, swapChainImages.data());

	//Saving in Global Var
	this->swapChainImageFormat = surfaceFormat.format;
	this->swapChainExtent = extent;
}

VkSurfaceFormatKHR rae::window::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR rae::window::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D rae::window::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(this->win, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

bool rae::window::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(this->deviceExtensions.begin(),this->deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

rae::SwapChainSupportDetails rae::window::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	//Filling in the structs 

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->physicalDevice, this->surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

//Image views 

void rae::window::create_image_views()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (uint32_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = this->create_image_view(swapChainImages[i], swapChainImageFormat);
	}
}

//render pass 

void rae::window::create_render_pass()
{
	/*specify how many color and depth buffers there will be,
	how many samples to use for each of them and 
	how their contents should be handled throughout the rendering operations */

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = this->swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //No multisampling yet
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//MOdify stencil data here : Currently set to dont care 
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	//
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//Subpass attachmentt
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	//CREATING THE RENDER PASS
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(this->logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

}

//Shader Modules 

VkShaderModule rae::window::createShaderModule(const std::vector<char>& module)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = module.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(module.data());
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(this->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

//Descriptor set 
void rae::window::create_descriptor_set_layout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(this->logicalDevice, &layoutInfo, nullptr, &this->descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

//Graphics Pipeline continuation
void rae::window::add_shader_to_pipeline()
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = this->vertexShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = this->fragmentShaderModule;
	fragShaderStageInfo.pName = "main";

	this->shaderStages[0] = vertShaderStageInfo;
	this->shaderStages[1] = fragShaderStageInfo;

	////Invoking the Vertex Buffer Layout
	//this->continue_pipeline_creation();
}

void rae::window::continue_pipeline_creation()
{
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	//
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	//
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	//Input Assemblies : Triangle 

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//ViewPorts Scissor section
	//The Scissors and viewport making Dynamic then can without Swapchain recreation 

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(this->dynamicStates.size());
	dynamicState.pDynamicStates = this->dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	//Rasterization stage :
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;



	if (WireframeMode) {
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	}
	else
	{
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	}
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	//MultiSampling a.k.a Antialiasing :Disabled 

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	//Depth Buffer (imp) ;

	VkPipelineDepthStencilStateCreateInfo depth{};

	//Color Blending 
		//First Structure:
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
					//optionals
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
					//optional end
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	//Second Structire:
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; 
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; 
	colorBlending.blendConstants[1] = 0.0f; 
	colorBlending.blendConstants[2] = 0.0f; 
	colorBlending.blendConstants[3] = 0.0f; 
		
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; 
	pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout; 
	pipelineLayoutInfo.pushConstantRangeCount = 0; 
	pipelineLayoutInfo.pPushConstantRanges = nullptr; 

	if (vkCreatePipelineLayout(this->logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	//Actual Creation of the GraphicsPipeline

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = this->shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; 
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = this->pipelineLayout;
	pipelineInfo.renderPass = this->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;   // Optional
	pipelineInfo.basePipelineIndex = -1;				// Optional

	if (vkCreateGraphicsPipelines(this->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void rae::window::createFramebuffer()
{
	this->swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(this->logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
//CommandPool and Buffer Sections

void rae::window::create_command_pool()
{
	QueueFamilyIndices queueFamilyIndices = this->findQueueFamily(this->physicalDevice);
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(this->logicalDevice, &poolInfo, nullptr, &this->command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void rae::window::create_command_buffers()
{
	command_buffers.resize(MAX_FRAMES_PROCESS);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->command_pool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) this->command_buffers.size();

	if (vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, this->command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

VkCommandBuffer rae::window::begin_single_time_commands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void rae::window::end_single_time_commands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(this->logicalDevice, this->command_pool, 1, &commandBuffer);
}

void rae::window::record_command_buffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = this->renderPass;

	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex]; //Error

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} }; //Clear Screen Color
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

    VkBuffer vertexBuffers[] = { this->vertex_buffer };
	VkDeviceSize offsets[] = { 0 }; 



	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);


	//Viewport and Scissors are dynamic thus we specify theme here
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(this->swapChainExtent.width);
	viewport.height = static_cast<float>(this->swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = this->swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &this->descriptorSets[currentFrame], 0, nullptr);
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(this->indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void rae::window::recreate_swap_chain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(this->win, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(this->win, &width, &height);
		glfwWaitEvents();
	}

	std::cout << "Swap Chain recreated " << std::endl;
	vkDeviceWaitIdle(this->logicalDevice);

	clean_up_swapchain();

	this->createSwapChain();
	this->create_image_views();

	createFramebuffer();
}


void rae::window::clean_up_swapchain()
{
	for (auto framebuffer : this->swapChainFramebuffers) {
		vkDestroyFramebuffer(this->logicalDevice, framebuffer, nullptr);
	}
	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(this->logicalDevice, imageView, nullptr);
	}
	vkDestroySwapchainKHR(this->logicalDevice, this->swapChain, nullptr);

}

//Vertex Buffer :: NEEDATTEN
void rae::window::create_vertex_buffers()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	//Using Staging Buffer 
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	
	void* data;
	vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), bufferSize);
	vkUnmapMemory(this->logicalDevice, stagingBufferMemory);

	create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertex_buffer, this->vertex_buffer_memory);

	copy_buffer(stagingBuffer, this->vertex_buffer, bufferSize);
	
	vkDestroyBuffer(this->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, stagingBufferMemory, nullptr);

}

//Index Buffers
void rae::window::create_index_buffers()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	//Using Staging Buffer 
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), bufferSize);
	vkUnmapMemory(this->logicalDevice, stagingBufferMemory);

	create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->index_buffer, this->vertex_buffer_memory);

	copy_buffer(stagingBuffer, this->index_buffer, bufferSize);

	vkDestroyBuffer(this->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, stagingBufferMemory, nullptr);

}

void rae::window::create_uniform_buffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_PROCESS);
	uniformBuffersMemory.resize(MAX_FRAMES_PROCESS);
	uniformBuffersMapped.resize(MAX_FRAMES_PROCESS);

	for (size_t i = 0; i < MAX_FRAMES_PROCESS; i++) {
		this->create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->uniformBuffers[i], this->uniformBuffersMemory[i]);

		vkMapMemory(this->logicalDevice, this->uniformBuffersMemory[i], 0, bufferSize, 0, &this->uniformBuffersMapped[i]);
	}
}

void rae::window::update_uniform_buffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void rae::window::create_descriptor_pool()
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_PROCESS);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_PROCESS);

	if (vkCreateDescriptorPool(this->logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void rae::window::create_descriptor_set()
{
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_PROCESS, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_PROCESS);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_PROCESS);
	if (vkAllocateDescriptorSets(this->logicalDevice, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_PROCESS; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);


		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr; // Optional
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(this->logicalDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

void rae::window::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(this->logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(this->logicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = this->find_memory_type(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}
	vkBindBufferMemory(this->logicalDevice, buffer, bufferMemory, 0);

}

void rae::window::copy_buffer(VkBuffer source_buffer, VkBuffer destination_buffer, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = this->begin_single_time_commands();

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; 
	copyRegion.dstOffset = 0; 
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, source_buffer, destination_buffer, 1, &copyRegion);
	
	//This contains the Submit and the free of the command buffers  
	this->end_single_time_commands(commandBuffer);

}

//Textureing Texture Texture2D
void rae::window::create_texture_image()
{
	Texture2D texture("Dep/Texture/img.jpg");

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	this->create_buffer(texture.get_device_size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(this->logicalDevice, stagingBufferMemory, 0, texture.get_device_size(), 0, &data);
	memcpy(data, texture.get_pixel_ptr(), static_cast<size_t>(texture.get_device_size()));
	vkUnmapMemory(this->logicalDevice, stagingBufferMemory);

	texture.free_pixels();

	this->create_image(texture, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT , VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		this->textureImage, this->textureImageMemory);

	this->transition_image_layout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	this->copy_buffer_to_image(stagingBuffer, textureImage, static_cast<uint32_t>(texture.get_width()), static_cast<uint32_t>(texture.get_height()));
	this->transition_image_layout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(this->logicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->logicalDevice, stagingBufferMemory, nullptr);

	texture.~Texture2D();
}

VkImageView rae::window::create_image_view(VkImage image, VkFormat format)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(this->logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
}

void rae::window::create_texture_image_views()
{
	this->textureImageView = this->create_image_view(textureImage, VK_FORMAT_R8G8B8A8_SRGB);

}

void rae::window::create_image(Texture2D& texture , VkImageTiling tiling_mode , VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& textureImage, VkDeviceMemory& textureImageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(texture.get_width());
	imageInfo.extent.height = static_cast<uint32_t>(texture.get_height());
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling = tiling_mode;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage =usage;

	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	if (vkCreateImage(this->logicalDevice, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(this->logicalDevice, textureImage, &memRequirements);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = this->find_memory_type(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(this->logicalDevice, textureImage, textureImageMemory, 0);
}

void rae::window::transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	VkCommandBuffer commandBuffer = this->begin_single_time_commands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	this->end_single_time_commands(commandBuffer);
}

void rae::window::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = this->begin_single_time_commands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	this->end_single_time_commands(commandBuffer);
}

//CONSTRUCTOR AND DESTRUCTOR SECTIONS
rae::window::window(int height, int width, const char* windowName) : win(nullptr), vulkan_instance(nullptr)
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->win = glfwCreateWindow(width, height, windowName, nullptr, nullptr);
	glfwSetFramebufferSizeCallback(this->win,frameBufferResizeCallback);
	if (this->win) {
		std::cout << "[Window created successfully with Code ] : "<< win << std::endl;

		//Ennumerating the max Version the System can support upto ( VULKAN VERSIONn )
		uint32_t version = 0;

		vkEnumerateInstanceVersion(&version);
		std::cout << "[System Can support Vulkan upto version : ] " << std::endl
			<< "MAJOR : " << VK_API_VERSION_MAJOR(version) << std::endl
			<< "MINOR : " << VK_API_VERSION_MINOR(version) << std::endl
			<< "PATCH : " << VK_API_VERSION_PATCH(version) << std::endl;
		
		//this->create_vulkan();
	}
	else {
		std::cout << "[Failed to Create a Window Exiting !]" << win << std::endl;
	}
}

void rae::window::run()
{
	while (!glfwWindowShouldClose(this->win)) {
		glfwPollEvents();
		draw_frame();
	}
}

void rae::window::create_sync_objs()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_PROCESS);
	renderFinishedSemaphores.resize(MAX_FRAMES_PROCESS);
	inFlightFences.resize(MAX_FRAMES_PROCESS);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	
	for (int i = 0; i < MAX_FRAMES_PROCESS; i++) {
		if (vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(this->logicalDevice, &fenceInfo, nullptr, &this->inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores!");
		}
	}
}

//The draw frame part
void rae::window::draw_frame()
{
	vkWaitForFences(this->logicalDevice, 1, &this->inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
	

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(this->logicalDevice, this->swapChain, UINT64_MAX, this->imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR|| result == VK_SUBOPTIMAL_KHR || isFrameBufferResized) {
		isFrameBufferResized = false;
		recreate_swap_chain();
		return;
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(this->logicalDevice, 1, &this->inFlightFences[currentFrame]);

	vkResetCommandBuffer(this->command_buffers[currentFrame], 0);
	record_command_buffer(this->command_buffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->command_buffers[currentFrame];

	VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	this->update_uniform_buffer(currentFrame);

	if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//Presentation

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(this->presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_PROCESS;
}

rae::window::~window()
{
	clean_up_swapchain();

	//deleting textures
	vkDestroyImageView(this->logicalDevice, this->textureImageView, nullptr);

	vkDestroyImage(this->logicalDevice, this->textureImage, nullptr);
	vkFreeMemory(this->logicalDevice, this->textureImageMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_PROCESS; i++) {
		vkDestroyBuffer(this->logicalDevice, this->uniformBuffers[i], nullptr);
		vkFreeMemory(this->logicalDevice, this->uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(this->logicalDevice, this->descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(this->logicalDevice, this->descriptorSetLayout, nullptr);
	//NEEDATTN
	vkDestroyBuffer(this->logicalDevice, this->index_buffer, nullptr);
	vkFreeMemory(this->logicalDevice, this->index_buffer_memory, nullptr);

	vkDestroyBuffer(this->logicalDevice, this->vertex_buffer, nullptr);
	vkFreeMemory(this->logicalDevice, this->vertex_buffer_memory, nullptr);

	for (int i = 0; i < MAX_FRAMES_PROCESS; i++) {
		vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(this->logicalDevice, this->inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(this->logicalDevice, this->command_pool, nullptr);

	/*for (auto framebuffer : this->swapChainFramebuffers) {
		vkDestroyFramebuffer(this->logicalDevice, framebuffer, nullptr);
	}*/

	vkDestroyPipeline(this->logicalDevice, this->graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(this->logicalDevice, this->pipelineLayout, nullptr);
	vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);
	vkDestroyShaderModule(this->logicalDevice, this->fragmentShaderModule, nullptr);
	vkDestroyShaderModule(this->logicalDevice, this->vertexShaderModule, nullptr);

	/*for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(this->logicalDevice, imageView, nullptr);
	}*/

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vulkan_instance, debugMessenger, nullptr);
	}
	//vkDestroySwapchainKHR(this->logicalDevice, swapChain, nullptr);

	vkDestroyDevice(this->logicalDevice, nullptr);
	vkDestroySurfaceKHR(this->vulkan_instance, surface, nullptr);
	vkDestroyInstance(this->vulkan_instance, nullptr);
	glfwDestroyWindow(this->win);
	glfwTerminate();
}

//Memory
uint32_t rae::window::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (type_filter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");

	return 0;
}
