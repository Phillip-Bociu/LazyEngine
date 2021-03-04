#include "headers/Renderer.hpp"
#include <stdexcept>
#include <iostream>
#include "headers/Log.hpp"
#include "headers/Common.hpp"
#include "headers/Window.hpp"
#include <vector>
#include <set>

namespace lzy
{

    const std::vector<const char *> deviceExtensions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    bool QueueFamilyIndices::isComplete()
    {
        return graphicsFamily != invalid && presentFamily != invalid;
    }

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

        LOG("Created Instance!");
    }

    QueueFamilyIndices Renderer::findQueueFamily(VkPhysicalDevice physicalDevice)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        std::vector<VkQueueFamilyProperties> qFams;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        qFams.resize(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, qFams.data());
        VkBool32 presentSupport = false;
        int i = 0;

        for (const auto &qFam : qFams)
        {
            if (qFam.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
                break;

            i++;
        }

        return indices;
    }

    bool extensionsSupported(VkPhysicalDevice device)
    {

        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> ext(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, ext.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &e : ext)
        {
            requiredExtensions.erase(e.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool Renderer::isDeviceSuitable(VkPhysicalDevice physicalDevice, QueueFamilyIndices *qFam)
    {
        *qFam = findQueueFamily(physicalDevice);

        return qFam->isComplete() && extensionsSupported(physicalDevice);
    }

    void Renderer::pickPhysicalDevice(QueueFamilyIndices *qFam)
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
                if (isDeviceSuitable(d, qFam))
                {
                    physicalDevice = d;
                    return;
                }
            }
            else if (!retval)
            {
                if (isDeviceSuitable(d, qFam))
                    retval = d;
            }
        }
        VkPhysicalDeviceProperties prop;
        vkGetPhysicalDeviceProperties(retval, &prop);
        physicalDevice = retval;
        LOG("Picking Fallback GPU: %s", prop.deviceName);
    }

    void Renderer::createDevice()
    {
        QueueFamilyIndices qFam;
        pickPhysicalDevice(&qFam);

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueFamilies{qFam.graphicsFamily, qFam.presentFamily};

        for (uint32_t fam : uniqueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = fam;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures pdFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &pdFeatures);

        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = queueCreateInfos.size();
        createInfo.pEnabledFeatures = &pdFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef DEBUG
        const char *layers[]{
            "VK_LAYER_KHRONOS_validation"};

        createInfo.ppEnabledLayerNames = layers;
        createInfo.enabledLayerCount = sizeof(layers) / sizeof(const char *);
#endif

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create Logical Device");
        }

        LOG("Created Device!");

        vkGetDeviceQueue(device, qFam.graphicsFamily, 0, &graphicsQueue);
        LOG("Created Graphics Queue!");
        vkGetDeviceQueue(device, qFam.presentFamily, 0, &presentQueue);
        LOG("Created Present Queue!")
    }

    void Renderer::createSurface()
    {
        if (glfwCreateWindowSurface(instance, Window::GetWindow(), nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create window surface!");
        }
    }

    void Renderer::Init()
    {
        try
        {
            createInstance();
            createSurface();
            createDevice();
        }
        catch (const std::exception &e)
        {
            ERROR("%s\n", e.what());
        }
    }

    void Renderer::Shutdown()
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }
}