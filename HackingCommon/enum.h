#pragma once
#include <string>

using namespace std;

// Less dependencies for inclusion in Hooked*.cpp that cannot include certain headers

// Grant enums sensible powers that were taken away when C++ ignored C
// MS already defines a macro DEFINE_ENUM_FLAG_OPERATORS that goes part way,
// but only does the bitwise operators and returns the result as enum types (so
// a simple 'if (enum & mask)' will still fail). Really what I want is the
// namespacing of an enum class that behaves like an int as an enum does. Not
// sure why C++ had to try to solve two problems at once and in doing so
// created a brand new problem...
#define SENSIBLE_ENUM(ENUMTYPE) \
inline int operator | (ENUMTYPE a, ENUMTYPE b) { return (((int)a) | ((int)b)); } \
inline int operator & (ENUMTYPE a, ENUMTYPE b) { return (((int)a) & ((int)b)); } \
inline int operator ^ (ENUMTYPE a, ENUMTYPE b) { return (((int)a) ^ ((int)b)); } \
inline int operator ~ (ENUMTYPE a) { return (~((int)a)); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) |= ((int)b)); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) &= ((int)b)); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (ENUMTYPE &)(((int &)a) ^= ((int)b)); } \
inline bool operator || (ENUMTYPE a,  ENUMTYPE b) { return (((int)a) || ((int)b)); } \
inline bool operator || (    bool a,  ENUMTYPE b) { return (((int)a) || ((int)b)); } \
inline bool operator || (ENUMTYPE a,      bool b) { return (((int)a) || ((int)b)); } \
inline bool operator && (ENUMTYPE a,  ENUMTYPE b) { return (((int)a) && ((int)b)); } \
inline bool operator && (    bool a,  ENUMTYPE b) { return (((int)a) && ((int)b)); } \
inline bool operator && (ENUMTYPE a,      bool b) { return (((int)a) && ((int)b)); } \
inline bool operator ! (ENUMTYPE a) { return (!((int)a)); }

template <class T1, class T2>
struct EnumName_t {
	T1 name;
	T2 val;
};

static int _autoicmp(const wchar_t* s1, const wchar_t* s2)
{
	return _wcsicmp(s1, s2);
}
static int _autoicmp(const char* s1, const char* s2)
{
	return _stricmp(s1, s2);
}

// To use this function be sure to terminate an EnumName_t list with {NULL, 0}
// as it cannot use ArraySize on passed in arrays.
template <class T1, class T2>
static T2 lookup_enum_val(struct EnumName_t<T1, T2>* enum_names, T1 name, T2 default_, bool* found = NULL)
{
	for (; enum_names->name; enum_names++) {
		if (!_autoicmp(name, enum_names->name)) {
			if (found)
				*found = true;
			return enum_names->val;
		}
	}

	if (found)
		*found = false;

	return default_;
}
template <class T1, class T2>
static T2 lookup_enum_val(struct EnumName_t<T1, T2>* enum_names, T1 name, size_t len, T2 default_, bool* found = NULL)
{
	for (; enum_names->name; enum_names++) {
		if (!_wcsnicmp(name, enum_names->name, len)) {
			if (found)
				*found = true;
			return enum_names->val;
		}
	}

	if (found)
		*found = false;

	return default_;
}
template <class T1, class T2>
static T1 lookup_enum_name(struct EnumName_t<T1, T2>* enum_names, T2 val)
{
	for (; enum_names->name; enum_names++) {
		if (val == enum_names->val)
			return enum_names->name;
	}

	return NULL;
}

template <class T2>
static wstring lookup_enum_bit_names(struct EnumName_t<const wchar_t*, T2>* enum_names, T2 val)
{
	wstring ret;
	T2 remaining = val;

	for (; enum_names->name; enum_names++) {
		if ((T2)(val & enum_names->val) == enum_names->val) {
			if (!ret.empty())
				ret += L' ';
			ret += enum_names->name;
			remaining = (T2)(remaining & (T2)~enum_names->val);
		}
	}

	if (remaining != (T2)0) {
		wchar_t buf[20];
		wsprintf(buf, L"%x", remaining);
		if (!ret.empty())
			ret += L' ';
		ret += L"unknown:0x";
		ret += buf;
	}

	return ret;
}

// Parses an option string of names given by enum_names. The enum used with
// this function should have an INVALID entry, other flags declared as powers
// of two, and the SENSIBLE_ENUM macro used to enable the bitwise and logical
// operators. As above, the EnumName_t list must be terminated with {NULL, 0}
//
// If you wish to parse an option string that contains exactly one unrecognised
// argument, provide a pointer to a pointer in the 'unrecognised' field and the
// unrecognised option will be returned. Multiple unrecognised options are
// still considered errors.
template <class T1, class T2, class T3>
static T2 parse_enum_option_string(struct EnumName_t<T1, T2>* enum_names, T3 option_string, T1* unrecognised)
{
	T3 ptr = option_string, cur;
	T2 ret = (T2)0;
	T2 tmp = T2::INVALID;

	if (unrecognised)
		*unrecognised = NULL;

	while (*ptr) {
		// Skip over whitespace:
		for (; *ptr == L' '; ptr++) {}

		// Mark start of current entry:
		cur = ptr;

		// Scan until the next whitespace or end of string:
		for (; *ptr && *ptr != L' '; ptr++) {}

		if (*ptr) {
			// NULL terminate the current entry and advance pointer:
			*ptr = L'\0';
			ptr++;
		}

		// Lookup the value of the current entry:
		tmp = lookup_enum_val<T1, T2>(enum_names, cur, T2::INVALID);
		if (tmp != T2::INVALID) {
			ret |= tmp;
		}
		else {
			if (unrecognised && !(*unrecognised)) {
				*unrecognised = cur;
			}
			else {
				LogOverlayW(LOG_WARNING, L"WARNING: Unknown option: %s\n", cur);
				ret |= T2::INVALID;
			}
		}
	}
	return ret;
}

// Two template argument version is the typical case for now. We probably want
// to start adding the 'const' modifier in a bunch of places as we work towards
// migrating to C++ strings, since .c_str() always returns a const string.
// Since the parse_enum_option_string currently modified one of its inputs, it
// cannot use const, so the three argument template version above is to allow
// both const and non-const types passed in.
template <class T1, class T2>
static T2 parse_enum_option_string(struct EnumName_t<T1, T2>* enum_names, T1 option_string, T1* unrecognised)
{
	return parse_enum_option_string<T1, T2, T1>(enum_names, option_string, unrecognised);
}

// This is similar to the above, but stops parsing when it hits an unrecognised
// keyword and returns the position without throwing any errors. It also
// doesn't modify the option_string, allowing it to be used with C++ strings.
template <class T1, class T2>
static T2 parse_enum_option_string_prefix(struct EnumName_t<T1, T2>* enum_names, T1 option_string, T1* unrecognised)
{
	T1 ptr = option_string, cur;
	T2 ret = (T2)0;
	T2 tmp = T2::INVALID;
	size_t len;

	if (unrecognised)
		*unrecognised = NULL;

	while (*ptr) {
		// Skip over whitespace:
		for (; *ptr == L' '; ptr++) {}

		// Mark start of current entry:
		cur = ptr;

		// Scan until the next whitespace or end of string:
		for (; *ptr && *ptr != L' '; ptr++) {}

		// Note word length:
		len = ptr - cur;

		// Advance pointer if not at end of string:
		if (*ptr)
			ptr++;

		// Lookup the value of the current entry:
		tmp = lookup_enum_val<T1, T2>(enum_names, cur, len, T2::INVALID);
		if (tmp != T2::INVALID) {
			ret |= tmp;
		}
		else {
			if (unrecognised)
				*unrecognised = cur;
			return ret;
		}
	}
	return ret;
}
