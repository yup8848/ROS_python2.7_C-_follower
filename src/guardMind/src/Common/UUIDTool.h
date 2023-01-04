
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string>

#ifdef _WIN32
#include <objbase.h>
#else
#include "uuid.h"
#endif

#define IRVMS_GUID_LEN 64

	namespace spaceUuid
	{
#ifdef _WIN32
		static std::string generate()
		{
			char buf[IRVMS_GUID_LEN] = { 0 };
			GUID guid;

			if (CoCreateGuid(&guid))
			{
				return std::move(std::string(""));
			}

			sprintf(buf,
				"%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2],
				guid.Data4[3], guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);

			return std::move(std::string(buf));
		}
#else
		static std::string generate()
		{
			char buf[IRVMS_GUID_LEN] = { 0 };

			uuid_t uu;
			uuid_generate(uu);

			int32_t index = 0;
			for (int32_t i = 0; i < 16; i++)
			{
				int32_t len = i < 15 ?
					sprintf(buf + index, "%02X-", uu[i]) :
					sprintf(buf + index, "%02X", uu[i]);
				if (len < 0)
					return std::move(std::string(""));
				index += len;
			}

			return std::move(std::string(buf));
		}
#endif
	}