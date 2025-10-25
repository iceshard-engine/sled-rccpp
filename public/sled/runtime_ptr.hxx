#pragma once
#include <sled/runtime_swappable_ptr.hxx>
#include <sled/runtime_object_constructor_concrete.hxx>
#include <memory>

namespace sled
{

	template<typename T>
	class SlPtr
	{
	public:
		template<typename... Args>
		static auto create(Args&&... args) noexcept -> sled::SlPtr<T>;

		template<typename Self>
		inline auto operator->(this Self& self) noexcept -> T* { return self._pointer; }

		SlPtr(std::nullptr_t = nullptr) noexcept;
		SlPtr(SlPtr&& other) noexcept
			: _pointer{ std::exchange(other._pointer, {nullptr}) }
		{ } 
		auto operator=(SlPtr&& other) noexcept -> SlPtr&
		{
			if (std::addressof(other) != this)
			{
				_pointer = std::exchange(other._pointer, { nullptr });
			}
			return *this;
		}
		~SlPtr() noexcept;

	protected:
		SlPtr(sled::SlSwappablePtr<T>&& ptr) noexcept;

	private:
		sled::SlSwappablePtr<T> _pointer;
	};

	template<typename T>
	inline SlPtr<T>::SlPtr(std::nullptr_t) noexcept
		: _pointer{ nullptr }
	{
	}

	template<typename T>
	inline SlPtr<T>::~SlPtr() noexcept
	{
	}

	template<typename T>
	inline SlPtr<T>::SlPtr(sled::SlSwappablePtr<T>&& ptr) noexcept
		: _pointer{ std::move(ptr) }
	{
	}

	template<typename T>
	template<typename... Args>
	inline auto SlPtr<T>::create(Args&&... args) noexcept -> sled::SlPtr<T>
	{
		static_assert(std::is_constructible_v<T, Args...>, "The type is not constructible with the given arguments!");

		sled::SlSwappablePtr<T> ptr{ sled::create_object<T>(std::forward<Args>(args)...), sled::rccpp::object_factory() };
		return SlPtr<T>{ std::move(ptr) };
	}

} // namespace sled

#if 0

namespace sled
{

	template<typename T, typename... Args>
	inline auto make_unique(Args&&... args) noexcept -> std::unique_ptr<T>
	{
		static_assert(std::is_constructible_v<T, Args...>, "The type is not constructible with the given arguments!");
		assert(PerModuleInterface::g_pRuntimeObjectSystem != nullptr);
		sled::ISlObjectFactorySystem* const factory = PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem();
		sled::SlObjectConstructorConcrete<T>* const ctor = sled::SlObjectConstructorConcrete<T>::latest(factory);
		sled::SlRuntimeObject<T>* result = sled::SlRuntimeObject<T>::init_rccpp_object(ctor, ctor->Construct(), std::forward<Args>(args)...);
		return std::unique_ptr<T>{ result->value<T>() };
	}

	template<typename T>
	struct SlSwappablePtr
	{
	public:
		T* _value;

		SlSwappablePtr(
			T* value = nullptr, sled::ISlObjectFactorySystem* = nullptr
		) noexcept
			: _value{ value }
		{ }

		inline void reset() noexcept
		{
			_value = nullptr;
		}

		constexpr auto operator=(std::nullptr_t) noexcept -> SlSwappablePtr&
		{
			_value = nullptr;
		}

		constexpr auto operator->() noexcept -> T* { return _value; }
		constexpr auto operator->() const noexcept -> T* { return _value; }

		constexpr operator T*() noexcept { return _value; }
		constexpr operator T*() const noexcept { return _value; }

		constexpr operator bool() const noexcept { return _value != nullptr; }
	};

	template<sled::concepts::RuntimeCompileReadyType T>
	struct SlSwappablePtr<T> : public sled::ISlRuntimeObjectOwner<true>
	{
	public:
		sled::ISlRuntimeObject* _value;

		SlSwappablePtr(T* value = nullptr) noexcept
			: SlSwappablePtr{
				::TActual<sled::SlRuntimeObject<T>>::from_value_pointer(value),
				PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem()
			}
		{ }

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
		auto operator=(SlSwappablePtr const& other) noexcept -> SlSwappablePtr& = delete;

		~SlSwappablePtr() noexcept override
		{
			//this->reset();
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

		constexpr auto operator->() noexcept -> T* { return _value->value<T>(); }
		constexpr auto operator->() const noexcept -> T* { return _value->value<T>(); }

		constexpr operator T*() noexcept{ return _value == nullptr ? nullptr : _value->value<T>(); }
		constexpr operator T*() const noexcept { return _value == nullptr ? nullptr : _value->value<T>(); }

		constexpr operator bool() const noexcept { return _value != nullptr; }

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

	template<typename T, typename... Args>
	inline auto make_unique(Args&&... args) noexcept -> std::unique_ptr<T>
	{
		static_assert(std::is_constructible_v<T, Args...>, "The type is not constructible with the given arguments!");
		assert(PerModuleInterface::g_pRuntimeObjectSystem != nullptr);
		sled::ISlObjectFactorySystem* const factory = PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem();
		sled::SlObjectConstructorConcrete<T>* const ctor = sled::SlObjectConstructorConcrete<T>::latest(factory);
		sled::SlRuntimeObject<T>* result = sled::SlRuntimeObject<T>::init_rccpp_object(ctor, ctor->Construct(), std::forward<Args>(args)...);
		return std::unique_ptr<T>{ result->value<T>() };
	}

	template<typename T>
	class SlPtr
	{
	public:
		template<typename... Args>
		static auto create(Args&&... args) noexcept -> sled::SlPtr<T>;

		template<typename Self>
		inline auto operator->(this Self& self) noexcept -> T* { return self._pointer; }

		SlPtr(std::nullptr_t = nullptr) noexcept;
		~SlPtr() noexcept;

	protected:
		SlPtr(
			sled::ISlObjectFactorySystem* factory,
			sled::ISlRuntimeObject* object
		) noexcept;

	private:
		sled::SlSwappablePtr<T> _pointer;
	};

	template<typename T>
	inline SlPtr<T>::SlPtr(std::nullptr_t) noexcept
		: _pointer{ nullptr }
	{ }

	template<typename T>
	inline SlPtr<T>::~SlPtr() noexcept
	{ }

	template<typename T>
	inline SlPtr<T>::SlPtr(
		sled::ISlObjectFactorySystem* factory,
		sled::ISlRuntimeObject* object
	) noexcept
		: _pointer{ object, factory }
	{ }

	template<typename T>
	template<typename... Args>
	inline auto SlPtr<T>::create(Args&&... args) noexcept -> sled::SlPtr<T>
	{
		static_assert(std::is_constructible_v<T, Args...>, "The type is not constructible with the given arguments!");
		assert(PerModuleInterface::g_pRuntimeObjectSystem != nullptr);
		sled::ISlObjectFactorySystem* const factory = PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem();
		sled::SlObjectConstructorConcrete<T>* const ctor = sled::SlObjectConstructorConcrete<T>::latest(factory);
		return SlPtr<T>{ factory, sled::SlRuntimeObject<T>::init_rccpp_object(ctor, ctor->Construct(), std::forward<Args>(args)...) };
	}

} // namespace sled

template <class _Ty1, sled::concepts::RuntimeCompileReadyType _Ty2>
class std::_Compressed_pair<_Ty1, _Ty2*, true> final : _Ty1 { // store a pair of values, not deriving from first
public:
	sled::SlSwappablePtr<_Ty2> _Myval2;

	template <class... _Other2>
	constexpr explicit _Compressed_pair(_Zero_then_variadic_args_t, _Other2&&... _Val2) noexcept(
		conjunction_v<is_nothrow_default_constructible<_Ty1>, is_nothrow_constructible<_Ty2*, _Other2...>>)
		: _Ty1(), _Myval2(_STD forward<_Other2>(_Val2)...) {
	}

	template <class _Other1, class... _Other2>
	constexpr _Compressed_pair(_One_then_variadic_args_t, _Other1&& _Val1, _Other2&&... _Val2) noexcept(
		conjunction_v<is_nothrow_constructible<_Ty1, _Other1>, is_nothrow_constructible<_Ty2*, _Other2...>>)
		: _Ty1(_STD forward<_Other1>(_Val1)), _Myval2(_STD forward<_Other2>(_Val2)...) {
	}

	constexpr _Ty1& _Get_first() noexcept {
		return *this;
	}

	constexpr const _Ty1& _Get_first() const noexcept {
		return *this;
	}
};

template <sled::concepts::RuntimeCompileReadyType T>
struct std::default_delete<T>
{
	constexpr void operator()(T* ptr) const
	{
		sled::SlRuntimeObject<T>* const rtobj = TActual<sled::SlRuntimeObject<T>>::from_value_pointer(ptr);
		rtobj->destroy();
		ptr = nullptr;
	}
};

void test_yeah(std::unique_ptr<SledTest> a)
{
	//a = 
}
#endif