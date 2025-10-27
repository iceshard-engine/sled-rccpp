#pragma once

#include <Aurora/RuntimeObjectSystem/IObject.h>
#include <Aurora/RuntimeObjectSystem/IObjectFactorySystem.h>
#include <Aurora/RuntimeObjectSystem/ObjectInterface.h>
#include <Aurora/RuntimeObjectSystem/RuntimeTracking.h>
#include <Aurora/RuntimeObjectSystem/ObjectInterfacePerModule.h>

struct IObjectFactorySystem;

template<typename T>
class TActual;

template<typename T>
class TObjectConstructorConcrete;
