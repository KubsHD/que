#include "pch.h"

#include "uuid.h"


#if _WIN32

#include <Windows.h>

String ID::get_uuid()
{
	UUID tmp;

	RPC_CSTR str = NULL;

	UuidCreate(&tmp);
	UuidToStringA(&tmp, &str);

	String s = (char*)str;

	return s;
}

#elif APPLE


#include <CoreFoundation/CoreFoundation.h>

String ID::get_uuid()
{
	auto uuid = CFUUIDCreate(NULL);
	auto str = CFUUIDCreateString(NULL, uuid);

	auto mut = CFStringCreateMutableCopy(NULL, 0, str);

	CFStringLowercase(mut, CFLocaleCopyCurrent());

	return String(CFStringGetCStringPtr(mut, kCFStringEncodingUTF8));
}

#elif __ANDROID__

#include <lib/uuid/uuid_v4.h>

String ID::get_uuid()
{
	UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator;
	UUIDv4::UUID uuid = uuidGenerator.getUUID();

	std::string bytes = uuid.bytes();

	return bytes;
}

#endif
