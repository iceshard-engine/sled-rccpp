#pragma once
#include <concepts>
#include <sled/runtime_uniqueid.hxx>

namespace sled
{

	class SlObjectSerializer;

	namespace concepts
	{

		template<typename T>
		concept RuntimeCompileCustomConstructor = RuntimeCompileReadyType<T>
			and requires (const typename T::SledConstructor& args)
		{
			{ args } -> std::convertible_to<const typename T::SledConstructor&>;
		};

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
		concept SerializableType = HasSerializeMethod<T> && HasDeserializeMethod<T>;

		template<typename T>
		concept ReconstructibleType = HasSerializeMethod<T> && HasReconstructMethod<T>;

	} // namespace concepts

} // namespace sled
