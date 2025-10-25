#pragma once
#include <sled/runtime_concepts.hxx>
#include <sled/runtime_interfaces.hxx>
#include <sled/runtime_object_serializer.hxx>
#include <sled/runtime_object_constructor.hxx>

namespace sled
{

	namespace rccpp
	{

		auto object_factory() noexcept -> sled::ISlObjectFactorySystem*;
		auto find_constructor(sled::SlConstructorId ctor_id) noexcept -> sled::ISlObjectConstructor*;

	} // namespace rccpp

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	class SlRuntimeObject : public IObject, public sled::ISlRuntimeObject
	{
	public:
		static constexpr sled::SlUniqueID UniqueID = Constant_RuntimeTypeUniqueID<T>;

		SlRuntimeObject() noexcept;

		constexpr auto unique_id() const noexcept -> sled::SlUniqueID override { return UniqueID; }

		auto constructor() const noexcept -> sled::SlRuntimeObjectConstructor<T>*;

	public: // Bridge with the original RCCPP API
		static auto from_rccpp_object(IObject* object) noexcept -> sled::SlRuntimeObject<T>*;

		template<typename... Args> requires (std::is_default_constructible_v<T>)
		static auto init_rccpp_object(
			sled::SlRuntimeObjectConstructor<T, std::tuple<Args...>>* ctor,
			IObject* object
		) noexcept -> SlRuntimeObject<T>*;

		template<typename... Args>
		static auto init_rccpp_object(
			sled::SlRuntimeObjectConstructor<T, std::tuple<Args...>>* ctor,
			IObject* object,
			std::remove_cvref_t<Args>&&... args
		) noexcept -> SlRuntimeObject<T>*;

		auto rccpp_object() noexcept -> IObject* { return this; }

	protected: // Implementation Of: IObject
		void GetInterface(InterfaceID iid, void** pReturn) override final;
		void Serialize(ISimpleSerializer* pSerializer) override final;

	protected:
		~SlRuntimeObject() noexcept override = default;
	};

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	inline auto SlRuntimeObject<T>::constructor() const noexcept -> sled::SlRuntimeObjectConstructor<T>*
	{
		return static_cast<sled::SlRuntimeObjectConstructor<T>*>(GetConstructor());
	}

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	inline auto SlRuntimeObject<T>::from_rccpp_object(IObject* object) noexcept -> SlRuntimeObject<T>*
	{
		SlRuntimeObject<T>* result = nullptr;
		return object->GetInterface(UniqueID.value, reinterpret_cast<void**>(std::addressof(result))), result;
	}

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	template<typename... Args> requires (std::is_default_constructible_v<T>)
	inline auto SlRuntimeObject<T>::init_rccpp_object(
		sled::SlRuntimeObjectConstructor<T, std::tuple<Args...>>* ctor,
		IObject* object
	) noexcept -> SlRuntimeObject<T>*
	{
		sled::SlRuntimeObject<T>* const result = from_rccpp_object(object);
		assert(result != nullptr);

		sled::SlRuntimeObjectConstructor<T>* const sled_ctor = static_cast<sled::SlRuntimeObjectConstructor<T>*>(ctor);
		sled_ctor->InitializeMemory(result->storage());
		return result;
	}

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	template<typename... Args>
	inline auto SlRuntimeObject<T>::init_rccpp_object(
		sled::SlRuntimeObjectConstructor<T, std::tuple<Args...>>* ctor,
		IObject* object,
		std::remove_cvref_t<Args>&&... args
	) noexcept -> SlRuntimeObject<T>*
	{
		sled::SlRuntimeObject<T>* const result = from_rccpp_object(object);
		assert(result != nullptr);

		sled::SlRuntimeObjectConstructor<T>* const sled_ctor = static_cast<sled::SlRuntimeObjectConstructor<T>*>(ctor);
		sled_ctor->InitializeMemory(result->storage(), std::forward<Args>(args)...);
		return result;
	}

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	inline SlRuntimeObject<T>::SlRuntimeObject() noexcept
		: IObject{ }
		, sled::ISlRuntimeObject{ }
	{
	}

	// Implementation Of: IObject
	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	inline void SlRuntimeObject<T>::GetInterface(InterfaceID iid, void** pReturn)
	{
		switch (iid)
		{
		case UniqueID.value:
			*pReturn = this;
			break;
		default:
			*pReturn = nullptr;
		}
	}

	template<typename T> requires sled::concepts::RuntimeCompileReadyType<T>
	inline void SlRuntimeObject<T>::Serialize(ISimpleSerializer* serializer_impl)
	{
		sled::SlObjectSerializer serializer{ serializer_impl };
		if constexpr (sled::concepts::SerializableType<T> || sled::concepts::ReconstructibleType<T>)
		{
			if (serializer_impl->IsLoading())
			{
				if constexpr (sled::concepts::HasDeserializeMethod<T>)
				{
					value()->on_deserialize(serializer);
				}
				else
				{
					T::on_reconstruct(storage(), serializer);
				}
			}
			else
			{
				value()->on_serialize(serializer);
			}
		}
		else if (serializer_impl->IsLoading())
		{
			if constexpr (sled::concepts::HasReconstructMethod<T>)
			{
				T::on_reconstruct(storage(), serializer);
			}
			else
			{
				static_assert(std::is_default_constructible_v<T>, "The type needs to be default constructible to define a reconstruct method");
				constructor()->InitializeMemory(this->storage());
			}
		}
	}

} // namespace sled
