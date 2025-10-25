#pragma once
#include <sled/runtime_object.hxx>
#include <Aurora/RuntimeObjectSystem/ObjectInterfacePerModule.h>

namespace sled
{

	struct SlAllocationInfo
	{
		size_t size;
		size_t alignment;
	};

	constexpr auto quick_align(size_t size, size_t alignment) noexcept
	{
		return size + ((0 - size) & (alignment - 1));
	}

	template<typename T>
	constexpr auto allocation_info() noexcept -> sled::SlAllocationInfo
	{

		size_t const alignment = std::max(alignof(sled::SlActualRuntimeObject<T>), alignof(T));
		size_t const size = quick_align(sizeof(sled::SlActualRuntimeObject<T>), alignment) + sizeof(T);
		return sled::SlAllocationInfo{ .size = size, .alignment = alignment };
	}

} // namespace sled

template<typename T> // todo: concept
class TActual<sled::SlRuntimeObject<T>> final : public sled::SlRuntimeObject<T>
{
public:
	static auto name() noexcept -> sled::SlString;
	static inline auto constructor() noexcept -> sled::SlObjectConstructorConcrete<T> const& { return TypeConstructor; }
	static constexpr auto storage_offset() noexcept -> size_t;
	static constexpr auto allocation_info() noexcept -> sled::SlAllocationInfo;

	static auto from_value_pointer(T* value) noexcept -> sled::SlRuntimeObject<T>*;

	~TActual() noexcept override;
	TActual(sled::SlObjectIndex index) noexcept;

	void destroy() noexcept override;
	auto storage() noexcept -> void* override { return ((char*)this) + storage_offset(); }
	auto object_index() const noexcept -> sled::SlObjectIndex { return _index; }

protected: // Implementation Of: IObject
	auto GetObjectId() const -> sled::SlObjectId override final;
	auto GetPerTypeId() const -> PerTypeObjectId override final { return _index; }
	auto GetConstructor() const noexcept -> IObjectConstructor* override final { return &TypeConstructor; }
	auto GetTypeName() const noexcept -> char const* override { return name().data(); }

private:
	static sled::SlObjectConstructorConcrete<T> TypeConstructor;
	sled::SlObjectIndex _index;
};

template<typename T>
constexpr auto TActual<sled::SlRuntimeObject<T>>::storage_offset() noexcept -> size_t
{
	size_t constexpr alignment = std::max(alignof(sled::SlActualRuntimeObject<T>), alignof(T));
	size_t constexpr size = sled::quick_align(sizeof(sled::SlActualRuntimeObject<T>), alignment);
	return size;
}

template<typename T>
constexpr auto TActual<sled::SlRuntimeObject<T>>::allocation_info() noexcept -> sled::SlAllocationInfo
{
	size_t const alignment = std::max(alignof(sled::SlActualRuntimeObject<T>), alignof(T));
	return sled::SlAllocationInfo{
		.size = storage_offset() + sizeof(T),
		.alignment = alignment
	};
}

template<typename T>
inline auto TActual<sled::SlRuntimeObject<T>>::from_value_pointer(T* value) noexcept ->  sled::SlRuntimeObject<T>*
{
	if (value != nullptr)
	{
		void* const sled_object_ptr = (reinterpret_cast<char*>(value) - storage_offset());
		return reinterpret_cast<TActual<sled::SlRuntimeObject<T>>*>(sled_object_ptr);
	}
	return nullptr;
}

template<typename T>
inline TActual<sled::SlRuntimeObject<T>>::~TActual() noexcept
{
	if (this->IsRuntimeDelete())
	{
		sled::SlRuntimeObject<T>::notify_owner(this->_owner, GetObjectId());
		assert(this->_owner == nullptr);
	}
}

template<typename T>
inline TActual<sled::SlRuntimeObject<T>>::TActual(sled::SlObjectIndex index) noexcept
	: sled::SlRuntimeObject<T>{ }
	, _index{ index }
{ }

template<typename T>
inline void TActual<sled::SlRuntimeObject<T>>::destroy() noexcept
{
	GetConstructor()->Destroy(this);
}

template<typename T>
inline auto TActual<sled::SlRuntimeObject<T>>::GetObjectId() const -> sled::SlObjectId
{
	sled::SlObjectId result;
	result.m_ConstructorId = TypeConstructor.GetConstructorId();
	result.m_PerTypeId = _index;
	return result;
}
