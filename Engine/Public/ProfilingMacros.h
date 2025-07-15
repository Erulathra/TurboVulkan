#pragma once

#if WITH_PROFILER

#if defined ( __clang__ ) || defined ( __GNUC__ )
	#define TracyFunction __PRETTY_FUNCTION__
#elif defined ( _MSC_VER )
	#define TracyFunction __FUNCSIG__
#endif

#include <tracy/Tracy.hpp>

#define TRACE_ZONE() ZoneScoped
#define TRACE_MARK_FRAME() FrameMark

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

#else

#define TRACE_ZONE()
#define TRACE_MARK_FRAME()

#endif
