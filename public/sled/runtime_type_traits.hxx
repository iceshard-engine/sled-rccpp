#pragma once
#include <tuple>
#include <sled/runtime_uniqueid.hxx>

namespace sled
{

	template<typename T>
	struct RuntimeCompileTypeTraits
	{
		using Type = T;
		using ParentType = void;
		using ConstructorArgs = std::tuple<>;

		static constexpr sled::SlUniqueID TypeID = sled::SlUniqueID{ 0 };
	};

	namespace concepts
	{

		template<typename T>
		concept IsRuntimeCompileReady = sled::RuntimeCompileTypeTraits<T>::TypeID != sled::SlUniqueID_Invalid;

		template<typename T>
		concept HasRuntimeCompileCustomConstructor = sled::concepts::IsRuntimeCompileReady<T>
			&& (std::tuple_size_v<typename sled::RuntimeCompileTypeTraits<T>::ConstructorArgs>) > 0;

		template<typename T>
		concept HasRuntimeCompileParentType = sled::concepts::IsRuntimeCompileReady<T>
			&& sled::concepts::IsRuntimeCompileReady<typename sled::RuntimeCompileTypeTraits<T>::ParentType>
			&& std::is_base_of_v<typename sled::RuntimeCompileTypeTraits<T>::ParentType, T>;

	} // namespace concepts

	namespace detail
	{

		template<typename T>
		using ConsturctorArgsT = typename sled::RuntimeCompileTypeTraits<T>::ConstructorArgs;

		template<typename T>
		constexpr auto uniqueid() noexcept -> sled::SlUniqueID
		{
			return sled::RuntimeCompileTypeTraits<T>::TypeID;
		}

		template<typename T>
		constexpr bool uniqueid_implements(sled::SlUniqueID id) noexcept
		{
			bool result = false;
			if constexpr (sled::concepts::IsRuntimeCompileReady<T>)
			{
				constexpr sled::SlUniqueID const TypeUniqueID = sled::detail::uniqueid<T>();
				if (TypeUniqueID == id)
				{
					result = true;
				}
				else if constexpr (sled::concepts::HasRuntimeCompileParentType<T>)
				{
					using ParentType = typename sled::RuntimeCompileTypeTraits<T>::ParentType;
					result = uniqueid_implements<ParentType>(id);
				}
			}
			return result;
		}

	} // namespace detail

} // namespace sled
