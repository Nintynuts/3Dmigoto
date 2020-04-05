#pragma once

#include <Unknwn.h>
#include <typeinfo>

static const char* type_name(IUnknown* object)
{
	try {
		return typeid(*object).name();
	}
	catch (__non_rtti_object) {
		return "<NO_RTTI>";
	}
	catch (bad_typeid) {
		return "<NULL>";
	}
}