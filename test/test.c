#include<stdio.h>
#include<vulkan/vulkan.h>

int main()
{
	VkInstance instance;
	VkApplicationInfo appInfo = {0};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Lazy Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(0,0,1);
	appInfo.pEngineName = "Lzy";
	appInfo.engineVersion = VK_MAKE_VERSION(0,0,1);	
	appInfo.apiVersion = VK_MAKE_VERSION(1,2,170);

	VkInstanceCreateInfo instanceCreateInfo = {0};

	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	vkCreateInstance(&instanceCreateInfo, NULL, &instance);
	printf("Hi");
}
