#pragma once

#include "CommonMacros.h"

#if WITH_PROFILER

#if defined ( __clang__ ) || defined ( __GNUC__ )
	#define TracyFunction __PRETTY_FUNCTION__
#elif defined ( _MSC_VER )
	#define TracyFunction __FUNCSIG__
#endif

#include <tracy/Tracy.hpp>

#include "Graphics/GraphicsCore.h"
#include <tracy/TracyVulkan.hpp>

#define TRACE_ZONE_SCOPED() ZoneScoped;
#define TRACE_ZONE_SCOPED_N(NAME) ZoneScopedN(NAME);

using FTraceGPUCtx = TracyVkCtx;
#define TRACE_NULL_GPU_CTX() nullptr
#define TRACE_CREATE_GPU_CTX(INSTANCE, PHYSICAL_DEVICE, DEVICE, QUEUE, COMMAND_BUFFER, INSTANCE_PROC_ADDR, DEVICE_PROC_ADDR) \
	TracyVkContext(INSTANCE, PHYSICAL_DEVICE, DEVICE, QUEUE, (COMMAND_BUFFER)->GetVkCommandBuffer(), INSTANCE_PROC_ADDR, DEVICE_PROC_ADDR)

#define TRACE_DESTROY_GPU_CTX(CTX) TracyVkDestroy(CTX)
#define TRACE_GPU_COLLECT(CTX, COMMAND_BUFFER) TracyVkCollect(CTX, (COMMAND_BUFFER)->GetVkCommandBuffer())

#define TRACE_GPU_SCOPED(GPU, COMMAND_BUFFER, NAME) TracyVkZone((GPU)->GetTraceGpuCtx(), (COMMAND_BUFFER)->GetVkCommandBuffer(), NAME);

#define TRACE_MARK_FRAME() tracy::Profiler::SendFrameMark( nullptr )

#define TRACE_PLOT_VALUE(NAME, VALUE) TracyPlot(NAME, VALUE);

#if 0 // broken
inline void * operator new ( std :: size_t count )
{
	auto ptr = malloc ( count ) ;
	TracyAlloc ( ptr , count ) ;
	return ptr ;
}

inline void operator delete ( void * ptr ) noexcept
{
	TracyFree ( ptr ) ;
	free ( ptr ) ;
}
#endif

#else // WITH_PROFILER

#define TRACE_ZONE_SCOPED() {}
#define TRACE_ZONE_SCOPED_N(NAME) {}

using FTraceGPUCtx = void*;
#define TRACE_NULL_GPU_CTX() nullptr
#define TRACE_CREATE_GPU_CTX(PHYSICAL_DEVICE, DEVICE, QUEUE, COMMAND_BUFFER) nullptr
#define TRACE_DESTROY_GPU_CTX(CTX) {}

#define TRACE_GPU_SCOPED(GPU_CTX, COMMAND_BUFFER, NAME) {}

#define TRACE_MARK_FRAME() {}

#define TRACE_PLOT_VALUE(NAME, VALUE) {}

#endif // else WITH_PROFILER
