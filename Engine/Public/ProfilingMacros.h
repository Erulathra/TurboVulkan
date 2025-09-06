#pragma once

#include "CommonMacros.h"

#if WITH_PROFILER

#if defined ( __clang__ ) || defined ( __GNUC__ )
	#define TracyFunction __PRETTY_FUNCTION__
#elif defined ( _MSC_VER )
	#define TracyFunction __FUNCSIG__
#endif

#include <tracy/Tracy.hpp>

#define TRACE_ZONE_SCOPED() ZoneScoped;
#define TRACE_ZONE_SCOPED_N(NAME) ZoneScopedN(#NAME);

#define TRACE_MARK_FRAME() FrameMark

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

#define TRACE_MARK_FRAME() {}

#endif // else WITH_PROFILER
