#include "Services/IService.h"

namespace Turbo
{
	FServiceManager::FServiceManager()
	{
		mServices.resize(static_cast<uint32>(EEngineStage::Num));
	}

	FServiceManager* FServiceManager::Get()
	{
		static FServiceManager* serviceManagerInstance = new FServiceManager();
		return serviceManagerInstance;
	}

	void FServiceManager::AddService(const IServicePtr& servicePtr)
	{
		TURBO_CHECK(servicePtr)
		mServices[static_cast<uint32>(servicePtr->GetStage())].push_back(servicePtr);
	}

	void FServiceManager::RemoveService(FName serviceName)
	{
		for (std::vector<IServicePtr>& servicesVector : mServices)
		{
			std::erase_if( servicesVector,
				[serviceName](const IServicePtr& service)
				{
					return service->GetName() == serviceName;
				});
		}
	}

	IServicePtr FServiceManager::GetService(FName serviceName)
	{
		for (std::vector<IServicePtr>& servicesVector : mServices)
		{
			for (const IServicePtr& service : servicesVector)
			{
				if (service->GetName() == serviceName)
				{
					return service;
				}
			}
		}

		return nullptr;
	}
} // Turbo