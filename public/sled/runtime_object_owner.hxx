#pragma once
#include <sled/runtime_types.hxx>

namespace sled
{

	template<bool IsRuntimeCompiled = true>
	class ISlRuntimeObjectOwner
	{
	public:
		ISlRuntimeObjectOwner(sled::ISlObjectFactorySystem* factory) noexcept
			: _factory{ factory }
		{ }

		virtual ~ISlRuntimeObjectOwner() noexcept = default;

		virtual void notify(sled::SlObjectId object_id) noexcept = 0;

	protected:
		sled::ISlObjectFactorySystem* _factory;
	};

	template<>
	class ISlRuntimeObjectOwner<false>
	{
	};

} // namespace sled
