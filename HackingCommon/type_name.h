#pragma once

#include <Unknwn.h>
#include <typeinfo>
#include <string>

static const char* type_name(IUnknown* object)
{
	try {
		return typeid(*object).name();
	}
	catch (std::__non_rtti_object) {
		return "<NO_RTTI>";
	}
	catch (std::bad_typeid) {
		return "<NULL>";
	}
}

std::string NameFromIID(IID id);