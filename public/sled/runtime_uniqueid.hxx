#pragma once
#include <concepts>
#include <type_traits>

namespace sled
{

	struct SlUniqueID
	{
		uint64_t value;
	};

	constexpr bool operator==(SlUniqueID left, SlUniqueID right) noexcept
	{
		return left.value == right.value;
	}

	template<typename T>
	static constexpr SlUniqueID Constant_RuntimeTypeUniqueID = { 0 };

	namespace concepts
	{

		template<typename T>
		concept RuntimeCompileReadyType = requires {
			std::is_default_constructible_v<T>;
		} and Constant_RuntimeTypeUniqueID<T> != sled::SlUniqueID{ 0 };

	} // namespace concepts

} // namespace sled
