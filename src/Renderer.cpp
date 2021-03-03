#include "headers/Renderer.hpp"
#include <stdexcept>
#include <iostream>
#include "headers/Log.hpp"
#include "headers/Common.hpp"
#include <vector>

namespace lzy
{

    void Renderer::createInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.apiVersion = VK_API_VERSION_1_2;
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pApplicationName = "None";
        appInfo.pEngineName = "LazyRenderer";

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

#ifdef DEBUG
        const char *layers[]{
            "VK_LAYER_KHRONOS_validation"};

        createInfo.ppEnabledLayerNames = layers;
        createInfo.enabledLayerCount = sizeof(layers) / sizeof(const char *);
#else
#endif

        std::vector<const char *> ext;
        uint32_t extCount;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);
        ext.reserve(extCount + ext.size());

        for (int i = 0; i < extCount; i++)
        {
            ext.push_back(glfwExtensions[i]);
        }

        createInfo.enabledExtensionCount = ext.size();
        createInfo.ppEnabledExtensionNames = ext.data();

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create instance.");
        }
    }
    void Renderer::pickPhysicalDevice()
    {
        std::vector<VkPhysicalDevice> devices;
        uint32_t count;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        devices.resize(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());
        VkPhysicalDevice retval = nullptr;

        for (VkPhysicalDevice d : devices)
        {
            VkPhysicalDeviceProperties prop;
            vkGetPhysicalDeviceProperties(d, &prop);

            if (prop.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                LOG("Picking discrete GPU: %s", prop.deviceName);
                physicalDevice = d;
                return;
            } else if(!retval)
                retval = d;
        }
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties(retval,&prop);
        physicalDevice = retval;
        LOG("Picking Fallback GPU: %s", prop.deviceName);
    }
    void Renderer::createDevice()
    {
        
    }

    void Renderer::Init()
    {
        try
        {
            createInstance();
            pickPhysicalDevice();
        }
        catch (const std::exception &e)
        {
            ERROR("%s\n", e.what());
        }
    }

    Renderer::~Renderer()
    {
        vkDestroyInstance(instance, nullptr);
    }

}