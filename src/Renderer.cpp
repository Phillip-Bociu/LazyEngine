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

    VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for(const auto& f : formats)
        {
            if(f.format == VK_FORMAT_R8G8B8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return f;
        }

        return formats[0];
    }

    VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        for(const auto& mode : presentModes)
        {
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return VK_PRESENT_MODE_MAILBOX_KHR;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(Window::GetWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)};

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }


    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;

        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t modeCount;

        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, nullptr);

        if (modeCount)
        {
            details.presentModes.resize(modeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &modeCount, details.presentModes.data());
        }

        return details;
    }

    bool QueueFamilyIndices::isComplete() const
    {
        return graphicsFamily != invalid && presentFamily != invalid;
    }

    std::vector<const char *> getExtensions()
    {
        std::vector<const char *> ext;
        uint32_t extCount;
        const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&extCount);
        ext.reserve(extCount + ext.size());

        for (int i = 0; i < extCount; i++)
        {
            ext.push_back(glfwExtensions[i]);
        }

#ifdef DEBUG
        ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return ext;
    }

    void Renderer::createSwapchain(QueueFamilyIndices* qFam)
    {
        SwapChainSupportDetails details = querySwapChainSupport(physicalDevice,surface);
        VkSurfaceFormatKHR format = chooseSurfaceFormat(details.formats);
        VkPresentModeKHR presentMode = choosePresentMode(details.presentModes);
        VkExtent2D extent = chooseExtent(details.capabilities);

        uint32_t imageCount = details.capabilities.minImageCount + 1;


        if(details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
        {
            imageCount = details.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = format.format;
        createInfo.imageColorSpace = format.colorSpace;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


        if (qFam->graphicsFamily!= qFam->presentFamily)
        {

            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = &qFam->graphicsFamily;

        } else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not Create Swapchain");
        }

        LOG("Created Swapchain");

        

        vkGetSwapchainImagesKHR(device, swapchain,&imageCount, nullptr);
        swapchainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

        swapchainFormat = format.format;
        swapchainExtent = extent;

    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        switch(messageSeverity)
        {
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
                ERROR("validation layer: %s\n", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
                WARN("validation layer: %s\n", pCallbackData->pMessage);
                break;
            case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
                LOG("validation layer: %s\n", pCallbackData->pMessage);
                break;
            default:
            break;
        }

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT* createInfo)
    {
        createInfo->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo->pfnUserCallback = debugCallback;
    }

    void Renderer::setupDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(&createInfo);

        if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Could not create debug Messenger");
        }

    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
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

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

        populateDebugMessengerCreateInfo(&debugCreateInfo);

        createInfo.pNext = &debugCreateInfo;

#endif

        std::vector<const char*> ext = getExtensions();

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

        bool swapchainAdequate = false;
        if (extensionsSupported(physicalDevice))
        {
            SwapChainSupportDetails details = querySwapChainSupport(physicalDevice, surface);
            swapchainAdequate = !details.formats.empty() && !details.presentModes.empty();
        }

        return qFam->isComplete() && swapchainAdequate;
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

    void Renderer::createDevice(QueueFamilyIndices* qFam)
    {
        pickPhysicalDevice(qFam);

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueFamilies{qFam->graphicsFamily, qFam->presentFamily};

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

        vkGetDeviceQueue(device, qFam->graphicsFamily, 0, &graphicsQueue);
        LOG("Created Graphics Queue!");
        vkGetDeviceQueue(device, qFam->presentFamily, 0, &presentQueue);
        LOG("Created Present Queue!");
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
            ///TEMP DATA
            QueueFamilyIndices indices;
            ///
            createInstance();
            #ifdef DEBUG
            setupDebugMessenger();
            #endif
            createSurface();
            createDevice(&indices);
            createSwapchain(&indices);
        }
        catch (const std::exception &e)
        {
            ERROR("%s\n", e.what());
        }
    }

    void Renderer::Shutdown()
    {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        #ifdef DEBUG
        DestroyDebugUtilsMessengerEXT(instance,debugMessenger,nullptr);
        #endif
        vkDestroyInstance(instance, nullptr);
    }
}