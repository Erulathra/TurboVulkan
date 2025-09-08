#pragma once
#include "Core/Delegate.h"

namespace Turbo
{
	class FCommandBuffer;
	class FGPUDevice;

	enum class EEngineStage : int8
	{
		PreDefault = 0,
		Default = 1,
		PostDefault = 2,

		Num
	};

	class IService
	{
		/** Interface */
	public:
		virtual ~IService() = default;

		virtual void Start() = 0;
		virtual void Shutdown() = 0;

		virtual void Tick_GameThread(float deltaTime) {};
		virtual void RenderFrame_RenderThread(FGPUDevice* gpu, FCommandBuffer* cmd) {};

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