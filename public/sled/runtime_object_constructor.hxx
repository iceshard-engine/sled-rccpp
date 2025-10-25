#pragma once
#include <sled/runtime_interfaces.hxx>
#include <type_traits>
#include <tuple>

namespace sled
{

	template<typename T>
	struct SlResolveConstrcutorArgs
	{
		using Constructor = std::tuple<>;
	};

	template<sled::concepts::RuntimeCompileCustomConstructor T>
	struct SlResolveConstrcutorArgs<T>
	{
		using Constructor = typename T::SledConstructor;
	};

	template<typename T, typename InitDef = typename SlResolveConstrcutorArgs<T>::Constructor> // todo: concept
	class SlRuntimeObjectConstructor;

	template<typename T, typename... InitArgs>
	class SlRuntimeObjectConstructor<T, std::tuple<InitArgs...>> : public sled::ISlObjectConstructor
	{
	public:
		virtual void InitializeMemory(void* memory, std::remove_cvref_t<InitArgs>&&... args) const noexcept
		{
			static_assert(std::is_constructible_v<T, InitArgs...>);
			if constexpr (std::is_constructible_v<T>)
			{
				T* t = new (memory) T{ std::forward<InitArgs>(args)... };
				assert(t == memory);
			}
		}

		virtual void InitializeMemory(void* memory) const noexcept
		{
			assert(std::is_default_constructible_v<T>);
			if constexpr (std::is_default_constructible_v<T>)
			{
				T* t = new (memory) T{ };
				assert(t == memory);
			}
		}

		virtual void ResetMemory(void* memory) const noexcept
		{
			reinterpret_cast<T*>(memory)->~T();
		}
	};

	template<typename T>
	class SlRuntimeObjectConstructor<T, std::tuple<>> : public sled::ISlObjectConstructor
	{
	public:
		virtual void InitializeMemory(void* memory) const noexcept
		{
			assert(std::is_default_constructible_v<T>);
			if constexpr (std::is_default_constructible_v<T>)
			{
				T* t = new (memory) T{ };
				assert(t == memory);
			}
		}

		virtual void ResetMemory(void* memory) const noexcept
		{
			reinterpret_cast<T*>(memory)->~T();
		}
	};

} // namespace sled
