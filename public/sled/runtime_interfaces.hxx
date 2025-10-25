#pragma once
#include <sled/runtime_types.hxx>
#include <sled/runtime_object_owner.hxx>

namespace sled
{

	class ISlRuntimeObject
	{
	public:
		virtual auto unique_id() const noexcept -> sled::SlUniqueID = 0;
		virtual auto storage() noexcept -> void* = 0;
		virtual void destroy() noexcept = 0;

		template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
		auto value() noexcept -> T*
		{
			assert(sled::Constant_RuntimeTypeUniqueID<T> == unique_id());
			return reinterpret_cast<T*>(storage());
		}

		auto assign_to(sled::ISlRuntimeObjectOwner<true>* owner) noexcept;

	protected:
		virtual ~ISlRuntimeObject() noexcept = 0 { }

	protected:
		static /*constexpr*/ inline void notify_owner(ISlRuntimeObjectOwner<false>*, sled::SlObjectId id) noexcept {}
		static /*constexpr*/ inline void notify_owner(ISlRuntimeObjectOwner<true>* owner, sled::SlObjectId id) noexcept
		{
			owner->notify(id);
		}

	protected:
		ISlRuntimeObjectOwner<>* _owner = nullptr;
	};

	inline auto ISlRuntimeObject::assign_to(sled::ISlRuntimeObjectOwner<true>* owner) noexcept
	{
		_owner = owner;
	}

} // namespace sled
