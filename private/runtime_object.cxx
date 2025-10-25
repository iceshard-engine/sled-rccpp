#include <sled/runtime_object.hxx>
#include <Aurora/RuntimeObjectSystem/IObjectFactorySystem.h>
#include <Aurora/RuntimeObjectSystem/IRuntimeObjectSystem.h>

namespace sled::rccpp
{

	auto object_factory() noexcept -> sled::ISlObjectFactorySystem*
	{
		return PerModuleInterface::g_pRuntimeObjectSystem->GetObjectFactorySystem();
	}

	auto find_constructor(sled::SlConstructorId ctor_id) noexcept -> sled::ISlObjectConstructor*
	{
		return object_factory()->GetConstructor(ctor_id);
	}

} // namespace sled::rccpp
