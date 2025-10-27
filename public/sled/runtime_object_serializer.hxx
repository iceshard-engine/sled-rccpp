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

	namespace concepts
	{

		template<typename T>
		concept HasSerializeMethod = requires(T const& t, sled::SlObjectSerializer& s) {
			{ t.on_serialize(s) } -> std::convertible_to<void>;
		};

		template<typename T>
		concept HasDeserializeMethod = requires(T& t, sled::SlObjectSerializer const& s) {
			{ t.on_deserialize(s) } -> std::convertible_to<void>;
		};

		template<typename T>
		concept HasReconstructMethod = requires(void* memory, sled::SlObjectSerializer const& s) {
			{ T::on_reconstruct(memory, s) } -> std::convertible_to<void>;
		};

		template<typename T>
		concept HasRuntimeConstructMethod = requires(void* memory, sled::SlObjectSerializer const& s) {
			{ T::on_construct(memory) } -> std::convertible_to<void>;
		};

		template<typename T>
		concept SerializableType = sled::concepts::IsRuntimeCompileReady<T>
			&& sled::concepts::HasSerializeMethod<T>
			&& sled::concepts::HasDeserializeMethod<T>;

		template<typename T>
		concept ReconstructibleType = sled::concepts::IsRuntimeCompileReady<T>
			&& sled::concepts::HasSerializeMethod<T>
			&& sled::concepts::HasReconstructMethod<T>;

	} // namespace concepts

} // namespace sled
