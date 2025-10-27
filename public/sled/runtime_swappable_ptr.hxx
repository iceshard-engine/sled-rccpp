#pragma once
#include <sled/runtime_object.hxx>
#include <sled/runtime_object_owner.hxx>

namespace sled
{

	namespace concepts
	{

		template<typename Base, typename Derived>
		concept HasIntactPointerOwnership =
			// The Derived pointer is not runtime-compiled, so it can be hold by a Base that can be either.
			(sled::concepts::IsRuntimeCompileReady<Derived> == false)
			// ... or both pointers are runtime-compiled.
			|| sled::concepts::IsRuntimeCompileReady<Base>;

	} // namespace concepts

	template<typename T>
	struct SlSwappablePtr;

	template<typename T>
	struct SlSwappablePtr : public sled::ISlRuntimeObjectOwner<true>
	{
		friend struct sled::SlSwappablePtr;
	private:
		static auto from_value(T* value) noexcept -> sled::ISlRuntimeObject*
		{
			return sled::SlActualRuntimeObject<T>::from_value_pointer(value);
		}

		static void try_assing_owner(sled::SlSwappablePtr<T>* self) noexcept
		{
			if constexpr (sled::concepts::IsRuntimeCompileReady<T>)
			{
				if (self->_value != nullptr && self->_factory != nullptr)
				{
					from_value(self->_value)->assign_to(self);
				}
			}
		}

	public:
		T* _value;

		constexpr explicit SlSwappablePtr(T* value) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ nullptr }
			, _value{ value }
		{ }

		constexpr SlSwappablePtr(T* value, sled::ISlObjectFactorySystem* factory) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ factory }
			, _value{ value }
		{
			try_assing_owner(this);
		}

		template<sled::concepts::IsRuntimeCompileReady U> requires (std::is_same_v<T, U>)
		constexpr SlSwappablePtr(sled::SlRuntimeObject<U>* object, sled::ISlObjectFactorySystem* factory) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ factory }
			, _value{ object->value<T>() }
		{
			try_assing_owner(this);
		}

		constexpr SlSwappablePtr(std::nullptr_t = {}) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ nullptr }
			, _value{ nullptr }
		{ }

		constexpr auto operator=(std::nullptr_t) noexcept -> SlSwappablePtr&
		{
			_value = nullptr;
			_factory = nullptr;
			return *this;
		}

		constexpr SlSwappablePtr(SlSwappablePtr&& other) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ ice::exchange(other._factory, nullptr) }
			, _value{ ice::exchange(other._value, nullptr) }
		{
			try_assing_owner(this);
		}

		constexpr auto operator=(SlSwappablePtr&& other) noexcept -> SlSwappablePtr&
		{
			if (this != ice::addressof(other))
			{
				// Don't destroy anything here, this is the task of the actual pointer class.
				_factory = ice::exchange(other._factory, nullptr);
				_value = ice::exchange(other._value, nullptr);
				try_assing_owner(this);
			}
			return *this;
		}

		template<typename U> requires sled::concepts::HasIntactPointerOwnership<T, U>
		SlSwappablePtr(SlSwappablePtr<U>&& other) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ ice::exchange(other._factory, nullptr) }
			, _value{ std::exchange(other._value, nullptr) }
		{
			try_assing_owner(this);
		}

		template<typename U> requires sled::concepts::HasIntactPointerOwnership<T, U>
		auto operator=(SlSwappablePtr<U>&& other) noexcept -> sled::SlSwappablePtr<T>&
		{
			// Don't destroy anything here, this is the task of the actual pointer class.
			_factory = ice::exchange(other._factory, nullptr);
			_value = std::exchange(other._value, nullptr);
			try_assing_owner(this);
			return *this;
		}

		void notify(sled::SlObjectId object_id) noexcept override
		{
			if constexpr (sled::concepts::IsRuntimeCompileReady<T>)
			{
				assert(_value != nullptr && this->_factory != nullptr);
				if (_value != nullptr && this->_factory != nullptr)
				{
					IObject* const rccpp_object = this->_factory->GetObject(object_id);
					assert(rccpp_object != nullptr);
					sled::SlRuntimeObject<T>* const object = sled::SlRuntimeObject<T>::from_rccpp_object(rccpp_object);
					assert(object != nullptr);

					from_value(_value)->assign_to(nullptr);

					_value = object->value<T>();
					assert(_value != nullptr);
					object->assign_to(this);
				}
			}
		}

		constexpr operator T* () & noexcept { return _value; }
		constexpr operator T* () const & noexcept { return _value; }
		constexpr operator T* () && noexcept = delete; // ("We don't allow auto-casting from r-values!");
		constexpr operator T* () const && noexcept = delete; // ("We don't allow auto-casting from r-values!");
	};

} // namespace sled
