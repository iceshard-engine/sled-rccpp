#pragma once
#include <inttypes.h>
#include <queue>
#include <vector>
#include <string>
#include <string_view>
#include <sled/runtime_type_traits.hxx>
#include <sled/runtime_rccpp_includes.hxx>

namespace sled
{

#if ! defined(RCCPPOFF)
	static constexpr bool is_available = true;
#else
	static constexpr bool is_available = false;
#endif

	using ISlObjectAllocator = ::IObjectAllocator;
	using ISlObjectConstructor = ::IObjectConstructor;
	using ISlObjectFactorySystem = ::IObjectFactorySystem;
	using ISlRuntimeTrackingInfo = ::IRuntimeTracking;

	using SlString = std::string_view;
	using SlHeapString = std::string;

	using SlObjectId = ::ObjectId;
	using SlObjectIndex = ::PerTypeObjectId;
	using SlConstructorId = ::ConstructorId;
	using SlProjectId = uint16_t;

	using SlModuleInterface = ::PerModuleInterface;
	using SlRuntimeTrackingInfo = ::RuntimeTackingInfo;

	class SlObjectSerializer;

	template<typename T>
	class SlPtr;

	template<typename T> requires sled::concepts::IsRuntimeCompileReady<T>
	class SlRuntimeObject;

	template<typename T>
	using SlActualRuntimeObject = ::TActual<sled::SlRuntimeObject<T>>;

	template<typename T>
	using SlObjectConstructorConcrete = ::TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>;

	template<typename T>
	using SlArray = std::vector<T>;

	template<typename T>
	using SlQueue = std::queue<T>;

} // namespace sled
