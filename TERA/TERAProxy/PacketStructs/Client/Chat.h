/*/
/*
/*		TODO: Expand the packet structures.
/*
/*/

#ifndef __PACKET_SEND_CHAT__H_
#define __PACKET_SEND_CHAT__H_

#pragma pack(push)
#pragma pack(1)

#include "../../Opcodes.h"
#include "../../Container.h"

namespace Packets
{
	template<size_t N>
	struct CChatPacket : public CPacketData
	{
		IMPLEMENT_PACKET(CChatPacket, packet)
		{
			uint16_t size;
			uint16_t header;
			uint8_t type;
			uint32_t unk;
			uint8_t unk_0;
			wchar_t text[N];
		};

		SPacket& packet;
	};
}

#pragma pack(pop)

#endif
