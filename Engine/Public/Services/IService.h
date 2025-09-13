#pragma once
#include "Core/Delegate.h"

namespace Turbo
{
	class FCommandBuffer;
	class FGPUDevice;

	enum class EEngineStage : int8
	{
		EarliestPossible = 0,

		PreDefault,
		Default,
		PostDefault,

		Num
	};

	class IService
	{
		/** Interface */
	public:
		virtual ~IService() = default;

		virtual void Start() = 0;
		virtual void Shutdown() = 0;

		virtual void BeginTick_GameThread(float deltaTime) {};
		virtual void EndTick_GameThread(float deltaTime) {};

		virtual void PostBeginFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd) {};
		virtual void BeginPresentingFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd) {};

		virtual EEngineStage GetStage() { return EEngineStage::Default; }
		virtual FName GetName() = 0;

		/** Interface end */
	};

	using IServicePtr = std::shared_ptr<IService>;

	class FServiceManager final
	{
	public:
		FServiceManager();

	public:
		static FServiceManager* Get();

	public:
		void AddService(const IServicePtr& servicePtr);
		void RemoveService(FName serviceName);

		IServicePtr GetService(FName serviceName);

		template <typename FunctionType>
		void ForEachService(FunctionType function);

		template <typename FunctionType>
		void ForEachServiceReverse(FunctionType function);

	private:
		std::vector<std::vector<IServicePtr>> mServices;
	};

	template <typename FunctionType>
	void FServiceManager::ForEachService(FunctionType function)
	{
		for (std::vector<IServicePtr>& servicesVector : mServices)
		{
			for (const IServicePtr& service : servicesVector)
			{
				function(service.get());
			}
		}
	}

	template <typename FunctionType>
	void FServiceManager::ForEachServiceReverse(FunctionType function)
	{
		for (auto GroupIt = mServices.rbegin(); GroupIt != mServices.rend(); ++GroupIt)
		{
			for (auto ServiceIt = GroupIt->rbegin(); ServiceIt != GroupIt->rend(); ++ServiceIt)
			{
				function(ServiceIt->get());
			}
		}
	}

	template<typename ServiceType>
	requires std::is_base_of_v<IService, ServiceType>
	class FServiceRegistration
	{
	public:
		FServiceRegistration()
		{
			FServiceManager* serviceManager = FServiceManager::Get();
			serviceManager->AddService(std::make_shared<ServiceType>());
		}
	};

#define REGISTER_SERVICE(SERVICE_TYPE) namespace {  FServiceRegistration<SERVICE_TYPE> g ##SERVICE_TYPE ##Registration; }

} // Turbo