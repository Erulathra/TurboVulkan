#include "Core/RHI/VulkanRHI.h"

#include "Core/CoreUtils.h"
#include "Core/Engine.h"
#include "Core/Window.h"
#include "Core/Math/Color.h"
#include "Core/RHI/SwapChain.h"
#include "Core/RHI/VulkanDevice.h"
#include "Core/RHI/VulkanHardwareDevice.h"
#include "Core/RHI/Utils/VulkanUtils.h"
#include "Core/RHI/Image.h"
#include "Core/RHI/Pipelines/ComputePipeline.h"
#include "Core/RHI/Pipelines/DescriptorAllocator.h"
#include "Core/RHI/Pipelines/DescriptorLayoutBuilder.h"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_sdl3.h"
#include "Core/CoreTimer.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Turbo
{
    FVulkanRHI::FVulkanRHI() = default;
    FVulkanRHI::~FVulkanRHI() = default;

    void FVulkanRHI::Init()
    {
        CreateVulkanInstance();

        gEngine->GetWindow()->CreateVulkanSurface(mVulkanInstance);
        AcquirePhysicalDevice();

        if (IsValid(mHardwareDevice))
        {
            mDevice = std::make_unique<FVulkanDevice>(*mHardwareDevice);
            mDevice->Init();
        }

        if (IsValid(mDevice))
        {
            mSwapChain = std::make_unique<FSwapChain>(*mDevice);
            mSwapChain->Init();
        }

        if (IsValid(mSwapChain))
        {
            InitFrameData();
        }

        InitDrawImage();
        InitDescriptors();
        InitImGui();
        InitScene();
    }

    void FVulkanRHI::InitWindow(FSDLWindow* window)
    {
        window->InitForVulkan();
    }

    void FVulkanRHI::Destroy()
    {
        if (mDevice)
        {
            CHECK_VULKAN_HPP(mDevice->Get().waitIdle());
        }

        DestroyImGui();

        for (auto& frameData : mFrameDatas)
        {
            frameData.Destroy();
        }
        mFrameDatas.clear();

        mMainDestroyQueue.Flush(mDevice.get());

        if (mDevice)
        {
            CHECK_VULKAN_HPP(mDevice->Get().waitIdle());
        }

        if (mSwapChain)
        {
            mSwapChain->Destroy();
            mSwapChain.reset();
        }

        if (mDevice)
        {
            mDevice->Destroy();
            mDevice.reset();
        }

        mHardwareDevice.reset();

        gEngine->GetWindow()->DestroyVulkanSurface(mVulkanInstance);
        DestroyVulkanInstance();
    }

    void FVulkanRHI::CreateVulkanInstance()
    {
        TURBO_LOG(LOG_RHI, LOG_INFO, "Initialize VOLK");

        VULKAN_HPP_DEFAULT_DISPATCHER.init();

        vk::Result vulkanResult;
        uint32 instanceVersion;
        std::tie(vulkanResult, instanceVersion)  = vk::enumerateInstanceVersion();
        TURBO_CHECK_MSG(vulkanResult == vk::Result::eSuccess && instanceVersion >= VULKAN_VERSION, "Your device doesn't support Vulkan 1.3");

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "Turbo Vulkan";
        appInfo.applicationVersion = TURBO_VERSION();
        appInfo.pEngineName = "Turbo Vulkan";
        appInfo.engineVersion = TURBO_VERSION();
        appInfo.apiVersion = VULKAN_VERSION;

        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char*> extensionNames = gEngine->GetWindow()->GetVulkanRequiredExtensions();

#if WITH_VALIDATION_LAYERS
        if (CheckValidationLayersSupport())
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers supported.")

            createInfo.enabledLayerCount = kVulkanValidationLayers.size();
            createInfo.ppEnabledLayerNames = kVulkanValidationLayers.data();

            extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        else
#endif // else !WITH_VALIDATION_LAYERS
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Validation layers are unsupported or disabled.")

            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        createInfo.enabledExtensionCount = extensionNames.size();
        createInfo.ppEnabledExtensionNames = extensionNames.data();

        TURBO_LOG(LOG_RHI, LOG_INFO, "Creating VKInstance.")
        CHECK_VULKAN_HPP(vk::createInstance(&createInfo, nullptr, &mVulkanInstance));
        VULKAN_HPP_DEFAULT_DISPATCHER.init(mVulkanInstance);

#if WITH_VALIDATION_LAYERS
        SetupValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

        std::tie(vulkanResult, mExtensionProperties) = vk::enumerateInstanceExtensionProperties();
        CHECK_VULKAN_HPP(vulkanResult);

        std::stringstream extensionsStream;
        for (const vk::ExtensionProperties& extension : mExtensionProperties)
        {
            extensionsStream << "\t" << extension.extensionName << "\n";
        }
        TURBO_LOG(LOG_RHI, LOG_DISPLAY, "Supported Extensions: \n {}", extensionsStream.str());
    }

    void FVulkanRHI::DestroyVulkanInstance()
    {
        if (mVulkanInstance)
        {
#if WITH_VALIDATION_LAYERS
            DestroyValidationLayersCallbacks();
#endif // WITH_VALIDATION_LAYERS

            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying VKInstance.")
            mVulkanInstance.destroy();
            mVulkanInstance = nullptr;
        }
    }

    void FVulkanRHI::InitFrameData()
    {
        TURBO_CHECK(mDevice->IsValid());
        TURBO_LOG(LOG_RHI, LOG_INFO, "Initializing frame datas");

        for (uint32 frameId = 0; frameId < mSwapChain->GetNumBufferedFrames(); ++frameId)
        {
            FFrameData& newFrameData = mFrameDatas.emplace_back(*mDevice);
            newFrameData.Init();
        }
    }

    void FVulkanRHI::InitDrawImage()
    {
        TURBO_CHECK(mDevice);

        mDrawImage = std::make_unique<FImage>(*mDevice);
        mDrawImage->SetFormat(vk::Format::eR16G16B16A16Sfloat);
        mDrawImage->SetSize(gEngine->GetWindow()->GetFrameBufferSize());
        mDrawImage->SetUsage(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eColorAttachment);

        mDrawImage->InitResource();
        mDrawImage->RequestDestroy(mMainDestroyQueue);
    }

    void FVulkanRHI::InitDescriptors()
    {
        mMainDescriptorAllocator = std::make_unique<FDescriptorAllocator>(mDevice.get());
        std::vector<FDescriptorAllocator::FPoolSizeRatio> ratios = { {vk::DescriptorType::eStorageImage, 1}};
        mMainDescriptorAllocator->Init(10, ratios);

        mMainDestroyQueue.OnDestroy().AddLambda([this]()
        {
            mMainDescriptorAllocator->Destroy();
        });
    }

    void FVulkanRHI::InitScene()
    {
        mComputePipeline = std::make_unique<FComputePipeline>(mDevice.get());

        FDescriptorLayoutBuilder descriptorLayoutBuilder;
        descriptorLayoutBuilder.AddBinding(0, vk::DescriptorType::eStorageImage);

        const vk::DescriptorSetLayout layout = descriptorLayoutBuilder.Build(mDevice.get(), vk::ShaderStageFlagBits::eCompute);
        const vk::DescriptorSet set = mMainDescriptorAllocator->Allocate(layout);

        mComputePipeline->SetDescriptors(layout, set);

        vk::DescriptorImageInfo imgInfo {};
        imgInfo.setImageLayout(vk::ImageLayout::eGeneral);
        imgInfo.setImageView(mDrawImage->GetImageView());

        vk::WriteDescriptorSet drawImageWrite = {};
        drawImageWrite.setDstBinding(0);
        drawImageWrite.setDstSet(set);
        drawImageWrite.descriptorCount = 1;
        drawImageWrite.descriptorType = vk::DescriptorType::eStorageImage;
        drawImageWrite.setImageInfo(imgInfo);

        mDevice->Get().updateDescriptorSets({drawImageWrite}, 0);

        mComputePipeline = std::make_unique<FComputePipeline>(mDevice.get());
        mComputePipeline->SetDescriptors(layout, set);

        std::vector<uint32> shaderData = FCoreUtils::ReadWholeFile<uint32>("Shaders/Shader.spv");
        vk::ShaderModule shaderModule = VulkanUtils::CreateShaderModule(mDevice.get(), shaderData);
        mComputePipeline->Init(shaderModule);
        mDevice->Get().destroyShaderModule(shaderModule);

        mMainDestroyQueue.OnDestroy().AddLambda([this]()
        {
            // TODO: Create proper pipeline destructor
            mComputePipeline->Destroy();
        });
    }

    void FVulkanRHI::RenderSync()
    {
        const FFrameData& fd = GetCurrentFrame();

        // Wait until gpu finish its work.
        CHECK_VULKAN_HPP(mDevice->Get().waitForFences(1, &fd.mRenderFence, true, kDefaultVulkanTimeout));
        CHECK_VULKAN_HPP(mDevice->Get().resetFences(1, &fd.mRenderFence));
    }

    void FVulkanRHI::Tick()
    {
        RenderSync();
        GetFrameDeletionQueue().Flush(mDevice.get());

        AcquireSwapChainImage();
        BeginImGuiFrame();
        DrawFrame();
        PresentImage();
    }

    void FVulkanRHI::AcquireSwapChainImage()
    {
        const FFrameData& fd = GetCurrentFrame();

        // Request image from a swap chain
        vk::Result result;
        std::tie(result, mSwapChainImageIndex) = mDevice->Get().acquireNextImageKHR(mSwapChain->GetVulkanSwapChain(), kDefaultVulkanTimeout, fd.mSwapChainSemaphore, nullptr);
        CHECK_VULKAN_HPP_MSG(result, "Cannot obtain image from a swap chain.");
    }

    void FVulkanRHI::DrawFrame()
    {
        const FFrameData& fd = GetCurrentFrame();

        CHECK_VULKAN_HPP(fd.mCMD.reset());

        const vk::CommandBufferBeginInfo bufferBeginInfo = VulkanInitializers::BufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        CHECK_VULKAN_HPP(fd.mCMD.begin(bufferBeginInfo));

        VulkanUtils::TransitionImage(fd.mCMD, mDrawImage->GetImage(), vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
        DrawScene(fd.mCMD);

        BlitDrawImageToSwapchainImage(fd.mCMD);

        const vk::Image& currentImage = mSwapChain->GetImage(mSwapChainImageIndex);
        const vk::ImageView& currentImageView = mSwapChain->GetImageView(mSwapChainImageIndex);

        VulkanUtils::TransitionImage(fd.mCMD, currentImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eAttachmentOptimal);
        DrawImGuiFrame(fd.mCMD, currentImageView);
        VulkanUtils::TransitionImage(fd.mCMD, currentImage, vk::ImageLayout::eAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);

        CHECK_VULKAN_HPP(fd.mCMD.end());

        // Submit queue

        const vk::CommandBufferSubmitInfo cmdBufferInfo = VulkanInitializers::CommandBufferSubmitInfo(fd.mCMD);

        const vk::SemaphoreSubmitInfo waitSemaphoreInfo = VulkanInitializers::SemaphoreSubmitInfo(fd.mSwapChainSemaphore, vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        const vk::SemaphoreSubmitInfo signalSemaphoreInfo = VulkanInitializers::SemaphoreSubmitInfo(fd.mRenderSemaphore, vk::PipelineStageFlagBits2::eAllGraphics);

        const vk::SubmitInfo2 submitInfo = VulkanInitializers::SubmitInfo(cmdBufferInfo, &signalSemaphoreInfo, &waitSemaphoreInfo);

        CHECK_VULKAN_HPP(mDevice->GetQueues().GraphicsQueue.submit2(1, &submitInfo, fd.mRenderFence));
    }

    void FVulkanRHI::BlitDrawImageToSwapchainImage(const vk::CommandBuffer& cmd)
    {
        const vk::Image& currentImage = mSwapChain->GetImage(mSwapChainImageIndex);

        VulkanUtils::TransitionImage(cmd, mDrawImage->GetImage(), vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
        VulkanUtils::TransitionImage(cmd, currentImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        VulkanUtils::BlitImage(cmd, mDrawImage->GetImage(), mDrawImage->GetSize(), currentImage, mSwapChain->GetImageSize());
    }

    void FVulkanRHI::DrawScene(const vk::CommandBuffer& cmd)
    {
        constexpr glm::vec4 clearColor = ELinearColor::kBlack;
        const vk::ClearColorValue clearColorValue { clearColor.r, clearColor.g, clearColor.b, 1.f};
        const vk::ImageSubresourceRange clearSubresourceRange = VulkanInitializers::ImageSubresourceRange();

        cmd.clearColorImage(mDrawImage->GetImage(), vk::ImageLayout::eGeneral, clearColorValue, clearSubresourceRange);

        glm::ivec3 dispatchSize = glm::ivec3( glm::ceil(glm::vec2(mDrawImage->GetSize()) / 8.f), 1);
        mComputePipeline->Dispatch(cmd, dispatchSize);

        ImGui::Begin("Turbo VULKAN!");
        ImGui::Text("FrameTime: %2f ms, FPS: %1f", FCoreTimer::DeltaTime(), 1.f / FCoreTimer::DeltaTime());
        float test;
        ImGui::SliderFloat("Testowy Float", &test, 0.f, 1.f);

        ImGui::End();
    }

    void FVulkanRHI::PresentImage()
    {
        const FFrameData& fd = GetCurrentFrame();

        vk::PresentInfoKHR presentInfo = VulkanInitializers::PresentInfo(mSwapChain->GetVulkanSwapChain(), fd.mRenderSemaphore, mSwapChainImageIndex);
        CHECK_VULKAN_HPP_MSG(mDevice->GetQueues().PresentQueue.presentKHR(presentInfo), "Cannot present swap chain image.");

        mFrameNumber++;
    }

    uint32 FVulkanRHI::GetFrameDataIndex() const
    {
#if 1
        const uint32 numBufferedFrames = mSwapChain->GetNumBufferedFrames();
        return numBufferedFrames > 0 ? mFrameNumber % numBufferedFrames : 0;
#else
        return mSwapChainImageIndex;
#endif
    }

#if WITH_VALIDATION_LAYERS

    bool FVulkanRHI::CheckValidationLayersSupport()
    {
        bool bSuccess = true;

        vk::ResultValue<std::vector<vk::LayerProperties>> layerProperties = vk::enumerateInstanceLayerProperties();
        CHECK_VULKAN_HPP(layerProperties.result);

        for (const char* requestedValidationLayer : kVulkanValidationLayers)
        {
            bool bLayerFound = false;
            for (const vk::LayerProperties& layerProperty : layerProperties.value)
            {
                if (std::strcmp(layerProperty.layerName.data(), requestedValidationLayer) == 0)
                {
                    bLayerFound = true;
                    break;
                }
            }

            if (!bLayerFound)
            {
                TURBO_LOG(LOG_RHI, LOG_CRITICAL, "Missing Validation Layer: {}", requestedValidationLayer);
                bSuccess = false;
            }
        }

        if (bSuccess)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "All requested validation layers are supported.");
        }

        return bSuccess;
    }

    void FVulkanRHI::SetupValidationLayersCallbacks()
    {
        TURBO_LOG(LOG_RHI, LOG_INFO, "Assigning validation layers callback.");

        vk::DebugUtilsMessengerCreateInfoEXT createInfo{};

        using msgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        using msgType = vk::DebugUtilsMessageTypeFlagBitsEXT;

        createInfo.messageSeverity = msgSeverity::eVerbose | msgSeverity::eInfo | msgSeverity::eWarning | msgSeverity::eError;
        createInfo.messageType = msgType::eGeneral | msgType::eValidation | msgType::ePerformance;
        createInfo.setPfnUserCallback(&FVulkanRHI::HandleValidationLayerCallback);
        createInfo.pUserData = nullptr;

        CHECK_VULKAN_HPP(mVulkanInstance.createDebugUtilsMessengerEXT(&createInfo, nullptr, &mDebugMessengerHandle));
    }

    void FVulkanRHI::DestroyValidationLayersCallbacks()
    {
        if (mDebugMessengerHandle)
        {
            TURBO_LOG(LOG_RHI, LOG_INFO, "Destroying validation layers callback.");
            mVulkanInstance.destroyDebugUtilsMessengerEXT(mDebugMessengerHandle);
        }
    }

    vk::Bool32 FVulkanRHI::HandleValidationLayerCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData)
    {
        using msgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
        if (messageSeverity & msgSeverity::eVerbose)
        {
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", callbackData->pMessage)
        }
        else if (messageSeverity & msgSeverity::eInfo)
        {
            // Messages with info severity are very verbose, so I reduced its verbosity to display.
            TURBO_LOG(LOG_RHI, LOG_DISPLAY, "{}", callbackData->pMessage)
        }
        else if (messageSeverity & msgSeverity::eWarning)
        {
            TURBO_LOG(LOG_RHI, LOG_WARN, "{}", callbackData->pMessage)
        }
        else if (messageSeverity & msgSeverity::eError)
        {
            TURBO_LOG(LOG_RHI, LOG_ERROR, "{}", callbackData->pMessage)
            TURBO_DEBUG_BREAK();
        }

        return VK_FALSE;
    }
#endif // WITH_VALIDATION_LAYERS

    void FVulkanRHI::InitImGui()
    {
        std::vector<FDescriptorAllocator::FPoolSizeRatio> ratios = {
            {vk::DescriptorType::eSampler, 1},
            {vk::DescriptorType::eCombinedImageSampler, 1},
            {vk::DescriptorType::eSampledImage, 1},
            {vk::DescriptorType::eStorageImage, 1},
            {vk::DescriptorType::eUniformTexelBuffer, 1},
            {vk::DescriptorType::eStorageTexelBuffer, 1},
            {vk::DescriptorType::eUniformBuffer, 1},
            {vk::DescriptorType::eStorageBuffer, 1},
            {vk::DescriptorType::eUniformBufferDynamic, 1},
            {vk::DescriptorType::eStorageBufferDynamic, 1},
            {vk::DescriptorType::eInputAttachment, 1},
        };

        mImGuiAllocator = std::make_unique<FDescriptorAllocator>(mDevice.get());
        mImGuiAllocator->SetFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
        mImGuiAllocator->Init(1000, ratios);

        mMainDestroyQueue.OnDestroy().AddLambda([this]()
        {
            mImGuiAllocator->Destroy();
        });

        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui_ImplSDL3_InitForVulkan(gEngine->GetWindow()->GetWindow());
        ImGui_ImplVulkan_InitInfo initInfo {};

        initInfo.ApiVersion = VULKAN_VERSION;
        initInfo.Instance = mVulkanInstance;
        initInfo.PhysicalDevice = mHardwareDevice->Get();
        initInfo.Device = mDevice->Get();
        initInfo.Queue = mDevice->GetQueues().GraphicsQueue;
        initInfo.DescriptorPool = mImGuiAllocator->Get();
        initInfo.MinImageCount = mSwapChain->GetNumBufferedFrames();
        initInfo.ImageCount = mSwapChain->GetNumBufferedFrames();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.CheckVkResultFn = [](VkResult result) {CHECK_VULKAN_MSG(result, "ImGUI RHI Error.")};
        initInfo.UseDynamicRendering = true;

        // Dynamic rendering info
        vk::PipelineRenderingCreateInfo imGuiPipelineCI {};
        imGuiPipelineCI.setColorAttachmentCount(1);
        const vk::Format format = mSwapChain->GetImageFormat();
        imGuiPipelineCI.setPColorAttachmentFormats(&format);

        initInfo.PipelineRenderingCreateInfo = imGuiPipelineCI;

        ImGui_ImplVulkan_LoadFunctions(VULKAN_VERSION, [](const char* functionName, void* vulkan_instance)
        {
            return static_cast<vk::Instance*>(vulkan_instance)->getProcAddr(functionName);
        }, &mVulkanInstance);
        ImGui_ImplVulkan_Init(&initInfo);

        ImGui::StyleColorsDark();

        // TODO: Move to config
        ImFont* firaCodeImFont = io.Fonts->AddFontFromFileTTF("Content/Fonts/FiraCode/FiraCode-Regular.ttf", 18);
        io.FontGlobalScale = 0.75f;

        ImGui::PushFont(firaCodeImFont);
    }

    void FVulkanRHI::BeginImGuiFrame()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();

        ImGui::NewFrame();
    }

    void FVulkanRHI::DrawImGuiFrame(const vk::CommandBuffer& cmd, const vk::ImageView& targetImageView)
    {
        ImGui::Render();

        const vk::RenderingAttachmentInfo colorAttachment = VulkanInitializers::AttachmentInfo(targetImageView);
        const vk::RenderingInfo renderInfo = VulkanInitializers::RenderingInfo(mSwapChain->GetImageSize(), &colorAttachment);

        cmd.beginRendering(renderInfo);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        cmd.endRendering();
    }

    void FVulkanRHI::DestroyImGui()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
    }


    void FVulkanRHI::AcquirePhysicalDevice()
    {
        vk::Result vulkanResult;
        std::vector<vk::PhysicalDevice> physicalDevices;
        std::tie(vulkanResult, physicalDevices) = mVulkanInstance.enumeratePhysicalDevices();

        if (!physicalDevices.empty())
        {
            std::vector<FVulkanHardwareDevice*> hardwareDevices;
            for (VkPhysicalDevice physicalDevice : physicalDevices)
            {
                FVulkanHardwareDevice* hardwareDevice = new FVulkanHardwareDevice(physicalDevice);
                if (hardwareDevice->IsValid() && hardwareDevice->IsDeviceCapable())
                {
                    hardwareDevices.push_back(hardwareDevice);
                }
            }

            std::ranges::sort(
                hardwareDevices, std::ranges::greater{},
                [this](const FVulkanHardwareDevice* device)
                {
                    return device->CalculateDeviceScore();
                });

            mHardwareDevice = !hardwareDevices.empty() ? std::unique_ptr<FVulkanHardwareDevice>(hardwareDevices[0]) : nullptr;

            for (int DeviceId = 1; DeviceId < hardwareDevices.size(); ++DeviceId)
            {
                delete hardwareDevices[DeviceId];
                hardwareDevices[DeviceId] = nullptr;
            }
        }

        TURBO_CHECK_MSG(IsValid(mHardwareDevice), "There is no suitable GPU device.");

        vk::PhysicalDeviceProperties deviceProperties = mHardwareDevice->Get().getProperties();
        std::string_view deviceName{deviceProperties.deviceName};
        TURBO_LOG(LOG_RHI, LOG_INFO, "Using \"{}\" as primary physical device. (Score: {})", deviceName, mHardwareDevice->CalculateDeviceScore());
    }
} // Turbo
