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
