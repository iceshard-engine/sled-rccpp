#pragma once
#include <sled/runtime_object_owner.hxx>
#include <sled/runtime_object.hxx>

namespace sled
{

	template<typename T>
	struct SlSwappablePtr
	{
	public:
		T* _value;

		SlSwappablePtr(T* value, sled::ISlObjectFactorySystem* = nullptr) noexcept
			: _value{ value }
		{ }

		SlSwappablePtr(std::nullptr_t = {}) noexcept : _value{ nullptr } {}
		auto operator=(std::nullptr_t) noexcept -> SlSwappablePtr& { _value = nullptr; return *this; }

		SlSwappablePtr(SlSwappablePtr&& other) noexcept = default;
		auto operator=(SlSwappablePtr&& other) noexcept -> SlSwappablePtr& = default;

		constexpr operator T* () noexcept { return _value; }
		constexpr operator T* () const noexcept { return _value; }
	};

	template<sled::concepts::RuntimeCompileReadyType T>
	struct SlSwappablePtr<T> : public sled::ISlRuntimeObjectOwner<true>
	{
	public:
		sled::ISlRuntimeObject* _value;

		SlSwappablePtr(
			sled::ISlRuntimeObject* value = nullptr,
			sled::ISlObjectFactorySystem* factory = nullptr
		) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ factory }
			, _value{ value }
		{
			if (_value != nullptr)
			{
				assert(_factory != nullptr);
				_value->assign_to(this);
			}
		}

		SlSwappablePtr(SlSwappablePtr&& other) noexcept
			: sled::ISlRuntimeObjectOwner<true>{ std::exchange(other._factory, nullptr) }
			, _value{ std::exchange(other._value, nullptr) }
		{
			if (_value != nullptr)
			{
				_value->assign_to(this);
			}
		}

		auto operator=(SlSwappablePtr&& other) noexcept -> SlSwappablePtr&
		{
			if (this != std::addressof(other))
			{
				this->reset();
				_factory = std::exchange(other._factory, nullptr);
				_value = std::exchange(other._value, nullptr);
				if (_value != nullptr)
				{
					_value->assign_to(this);
				}
			}
			return *this;
		}

		SlSwappablePtr(SlSwappablePtr const& other) noexcept = delete;
		auto operator=(SlSwappablePtr const& other) noexcept -> SlSwappablePtr & = delete;

		~SlSwappablePtr() noexcept override
		{
			this->reset();
		}

		inline void reset() noexcept
		{
			if (_value != nullptr)
			{
				assert(this->_factory != nullptr);
				std::exchange(_value, nullptr)->destroy();
			}
		}

		constexpr auto operator=(std::nullptr_t) noexcept -> SlSwappablePtr&
		{
			this->reset();
			return *this;
		}

		constexpr operator T* () noexcept { return _value == nullptr ? nullptr : _value->value<T>(); }
		constexpr operator T* () const noexcept { return _value == nullptr ? nullptr : _value->value<T>(); }

	private:
		void notify(sled::SlObjectId object_id) noexcept override;
	};

	template<sled::concepts::RuntimeCompileReadyType T>
	inline void SlSwappablePtr<T>::notify(sled::SlObjectId object_id) noexcept
	{
		assert(_value && this->_factory);
		if (_value && this->_factory)
		{
			IObject* const object = this->_factory->GetObject(object_id);
			assert(object != nullptr);
			_value->assign_to(nullptr);
			_value = sled::SlRuntimeObject<T>::from_rccpp_object(object);
			assert(_value != nullptr);
			_value->assign_to(this);
		}
	}

} // namespace sled
