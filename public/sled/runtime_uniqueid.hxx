#pragma once
#include <concepts>
#include <type_traits>

namespace sled
{

	struct SlUniqueID
	{
		uint32_t value;
	};

	constexpr bool operator==(sled::SlUniqueID left, sled::SlUniqueID right) noexcept
	{
		return left.value == right.value;
	}

	static constexpr sled::SlUniqueID SlUniqueID_Invalid{ 0 };

} // namespace sled
