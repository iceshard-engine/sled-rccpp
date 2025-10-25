#pragma once
#include <inttypes.h>
#include <queue>
#include <vector>
#include <string>
#include <string_view>
#include <Aurora/RuntimeObjectSystem/IObject.h>
#include <Aurora/RuntimeObjectSystem/IObjectFactorySystem.h>
#include <Aurora/RuntimeObjectSystem/ObjectInterface.h>
#include <Aurora/RuntimeObjectSystem/RuntimeTracking.h>
#include <Aurora/RuntimeObjectSystem/ObjectInterfacePerModule.h>
#include <sled/runtime_concepts.hxx>


struct IObjectFactorySystem;
template<typename T>
class TActual;
template<typename T>
class TObjectConstructorConcrete;

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

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
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
