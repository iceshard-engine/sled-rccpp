#pragma once
#include <sled/runtime_object_actual.hxx>
#include <sled/runtime_object_constructor.hxx>

namespace sled
{

	namespace rccpp
	{

		template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
		auto find_constructor() noexcept -> sled::SlRuntimeObjectConstructor<T>*;

	} // namespace rccpp

	template<typename T, typename... Args> requires sled::concepts::RuntimeCompileReadyType<T>
	auto create_object(Args&&... args) noexcept -> sled::SlRuntimeObject<T>*;

} // namespace sled

template<typename T>
class TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>> final : public sled::SlRuntimeObjectConstructor<T>
{
public:
	using ObjectType = sled::SlActualRuntimeObject<T>;

	TObjectConstructorConcrete(
		sled::SlString filename,
		sled::ISlRuntimeTrackingInfo const* tracking_info
	) noexcept;

	static auto GetTypeNameStatic() noexcept -> char const* { return sled::SlActualRuntimeObject<T>::GetTypeNameStatic(); }
	auto GetName() noexcept -> char const* override;
	auto GetFileName() noexcept -> const char* override;
	auto GetCompiledPath() noexcept -> const char* override;

	void SetConstructorId(sled::SlConstructorId id) noexcept override;
	auto GetConstructorId() const noexcept -> sled::SlConstructorId override;

	void SetProjectId(uint16_t project_id) noexcept override;
	auto GetProjectId() const noexcept -> uint16_t override;

	bool GetIsSingleton() const noexcept override { return false; }
	bool GetIsAutoConstructSingleton() const noexcept override { return false; }

	void SetAllocator(sled::ISlObjectAllocator* allocator) noexcept override;
	auto GetAllocator() const noexcept -> sled::ISlObjectAllocator* override;

	//void InitializeMemory(void* memory) const noexcept;

	auto ConstructNextIndex() noexcept -> sled::SlObjectIndex;
	auto Construct() noexcept -> IObject* override;
	void ConstructNull() noexcept override;
	void Destroy(IObject* object) noexcept override;
	auto GetConstructedObject(sled::SlObjectIndex idx) const noexcept -> IObject* override;
	auto GetNumberConstructedObjects() const noexcept -> size_t override;
	void ClearIfAllDeleted() noexcept override;

	auto GetMaxNumTrackingInfo() const noexcept -> size_t override;
	auto GetTrackingInfo(size_t num) const noexcept -> sled::SlRuntimeTrackingInfo override;

	// Special

	void DeRegister(sled::SlObjectIndex index) noexcept
	{
		//remove from constructed objects.
		//use swap with last one
		if ((_objects.size() - 1) == index)
		{
			//it's the last one, just remove it.
			_objects.pop_back();
		}
		else
		{
			_free_ids.push(index);
			_objects[index] = nullptr;
		}
	}

private:
	sled::ISlObjectAllocator* _allocator;
	sled::ISlRuntimeTrackingInfo const* _trackinginfo;
	sled::SlHeapString _filename;
	sled::SlConstructorId _id_constructor;
	sled::SlProjectId _id_project;

	sled::SlArray<ObjectType*> _objects;
	sled::SlQueue<sled::SlObjectIndex> _free_ids;
};

template<typename T>
inline TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::TObjectConstructorConcrete(
	sled::SlString filename,
	sled::ISlRuntimeTrackingInfo const* tracking_info
) noexcept
	: _allocator{ nullptr }
	, _trackinginfo{ tracking_info }
	// TODO: Ensure the _filename is a absolute path
	, _filename{ sled::SlModuleInterface::GetOSCanonicalCleanPath(filename.data()) }
	, _id_constructor{ InvalidId }
	, _id_project{ sled::SlProjectId{} }
	, _objects{ }
	, _free_ids{ }
{
	_objects.reserve(8);
	sled::SlModuleInterface::GetInstance()->AddConstructor(this);
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetName() noexcept -> char const*
{
	return sled::SlActualRuntimeObject<T>::name().data();
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetFileName() noexcept -> const char*
{
	// TODO: Ensure the _filename is a absolute path
	return _filename.data();
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetCompiledPath() noexcept -> const char*
{
	// TODO: Is it still necessary to keep this value?
	return "";
}

template<typename T>
inline void TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::SetConstructorId(sled::SlConstructorId id) noexcept
{
	_id_constructor = id;
}

template<typename T>
inline auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetConstructorId() const noexcept -> sled::SlConstructorId
{
	return _id_constructor;
}

template<typename T>
inline void TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::SetProjectId(sled::SlProjectId id) noexcept
{
	_id_project = id;
}

template<typename T>
inline auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetProjectId() const noexcept -> sled::SlProjectId
{
	return _id_project;
}

template<typename T>
void TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::SetAllocator(sled::ISlObjectAllocator* allocator) noexcept
{
	_allocator = allocator;
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetAllocator() const noexcept -> sled::ISlObjectAllocator*
{
	return _allocator;
}

template<typename T>
inline auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::ConstructNextIndex() noexcept -> sled::SlObjectIndex
{
	sled::SlObjectIndex result;
	if (_free_ids.empty())
	{
		result = _objects.size();
		ConstructNull();
	}
	else
	{
		result = _free_ids.front();
		_free_ids.pop();
	}
	return result;
}

//template<typename T>
//void TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::InitializeMemory(void* memory) const noexcept
//{
//	assert(false); // TODO: Only for default constructible?
//}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::Construct() noexcept -> IObject*
{
	sled::SlObjectIndex const index = ConstructNextIndex();
	sled::SlAllocationInfo constexpr allocinfo = ObjectType::allocation_info();

	void* const object_memory = _allocator->Allocate(allocinfo.size, allocinfo.alignment);
	ObjectType* const object = new (object_memory) ObjectType{ index };
	_objects[index] = object;
	return object->rccpp_object();
}

template<typename T>
void TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::ConstructNull() noexcept
{
	_objects.push_back(nullptr);
}

template<typename T>
void TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::Destroy(IObject* rccpp_object) noexcept
{
	assert(_allocator != nullptr);
	{
		ObjectType* const object = static_cast<ObjectType*>(rccpp_object);
		this->ResetMemory(object->storage());
		this->DeRegister(object->object_index());

		object->~ObjectType();
	}
	_allocator->Free(rccpp_object);
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetConstructedObject(sled::SlObjectIndex index) const noexcept -> IObject*
{
	if (_objects.size() > index)
	{
		return _objects[index];
	}
	return 0;
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetNumberConstructedObjects() const noexcept -> size_t
{
	return _objects.size();
}

template<typename T>
void TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::ClearIfAllDeleted() noexcept
{
	_free_ids = {};
	_objects.clear();
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetMaxNumTrackingInfo() const noexcept -> size_t
{
	if (_trackinginfo)
	{
		return _trackinginfo->MaxNum;
	}
	return 0;
}

template<typename T>
auto TObjectConstructorConcrete<sled::SlActualRuntimeObject<T>>::GetTrackingInfo(size_t num) const noexcept -> sled::SlRuntimeTrackingInfo
{
	if (_trackinginfo)
	{
		return _trackinginfo->GetTrackingInfo(num);
	}
	return sled::SlRuntimeTrackingInfo::GetNULL();
}

template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
auto sled::rccpp::find_constructor() noexcept -> sled::SlRuntimeObjectConstructor<T>*
{
	sled::SlObjectConstructorConcrete<T> const& type_ctor = sled::SlActualRuntimeObject<T>::constructor();
	sled::SlConstructorId const ctor_id = type_ctor.GetConstructorId();
	return static_cast<sled::SlRuntimeObjectConstructor<T>*>(
		sled::rccpp::find_constructor(ctor_id)
		);
}

template<typename T, typename... Args> requires sled::concepts::RuntimeCompileReadyType<T>
auto sled::create_object(Args&&... args) noexcept -> sled::SlRuntimeObject<T>*
{
	static_assert(std::is_constructible_v<T, Args...>, "The type is not constructible with the given arguments!");
	sled::SlRuntimeObjectConstructor<T>* const ctor = sled::rccpp::find_constructor<T>();
	return sled::SlRuntimeObject<T>::init_rccpp_object(ctor, ctor->Construct(), std::forward<Args>(args)...);
}

#define SLED_TYPE( T )	\
	static RuntimeTracking< __COUNTER__ > g_runtimeTrackingList_##T; \
	template<> TObjectConstructorConcrete< TActual< sled::SlRuntimeObject< T > > > TActual< sled::SlRuntimeObject< T > >::TypeConstructor( __FILE__, &g_runtimeTrackingList_##T );\
	template<> auto TActual< sled::SlRuntimeObject< T > >::name() noexcept -> sled::SlString { return #T; } \
	template class TActual< sled::SlRuntimeObject< T > >

#define SLED_TYPE_WITH_CTOR_ARGS( T, ... )	\
	static RuntimeTracking< __COUNTER__ > g_runtimeTrackingList_##T; \
	template<> TObjectConstructorConcrete< TActual< sled::SlRuntimeObject< T > > > TActual< sled::SlRuntimeObject< T > >::TypeConstructor( __FILE__, &g_runtimeTrackingList_##T );\
	template<> auto TActual< sled::SlRuntimeObject< T > >::name() noexcept -> sled::SlString { return #T; } \
	template class TActual< sled::SlRuntimeObject< T > >
