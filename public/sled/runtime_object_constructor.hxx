#pragma once
#include <sled/runtime_object_interface.hxx>
#include <type_traits>
#include <tuple>

namespace sled
{

	template<typename T, typename InitDef = sled::detail::ConsturctorArgsT<T>> // todo: concept
	class SlRuntimeObjectConstructor;

	template<typename T, typename... InitArgs>
	class SlRuntimeObjectConstructor<T, std::tuple<InitArgs...>> : public sled::ISlObjectConstructor
	{
	public:
		virtual void InitializeMemory(void* memory, InitArgs... args) const noexcept
		{
			static_assert(std::is_constructible_v<T, InitArgs...>);

			// Construct the object in the given memory block.
			T* const t = new (memory) T{ args... };
			assert(t == memory);
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
