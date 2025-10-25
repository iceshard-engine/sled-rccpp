#pragma once
#include <sled/runtime_types.hxx>
#include <Aurora/RuntimeObjectSystem/ISimpleSerializer.h>
#include <string_view>
#include <assert.h>

namespace sled
{

	class SlObjectSerializer final
	{
	public:
		template<typename T>
		void write_property(std::string_view name, T const& field) noexcept
		{
			// NOTE: we cast away the const, since that's the current implementations way of doing it.
			//  However we don't need to wory since this will only happed during serialization.
			assert(_impl->IsLoading() == false);
			_impl->SerializeProperty(name.data(), const_cast<T&>(field));
		}

		template<typename T>
		void read_property(std::string_view name, T& field) const noexcept
		{
			assert(_impl->IsLoading() == true);
			_impl->SerializeProperty(name.data(), field);
		}

	public:
		SlObjectSerializer(ISimpleSerializer* impl) noexcept
			: _impl{ impl }
		{ }

		SlObjectSerializer(SlObjectSerializer&&) noexcept = delete;
		auto operator=(SlObjectSerializer&&) noexcept -> SlObjectSerializer & = delete;
		SlObjectSerializer(SlObjectSerializer const&) noexcept = delete;
		auto operator=(SlObjectSerializer const&) noexcept -> SlObjectSerializer & = delete;

	private:
		ISimpleSerializer* const _impl;
	};

} // namespace sled
