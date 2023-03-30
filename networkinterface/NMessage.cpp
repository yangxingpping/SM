
#include "NMessage.h"

#include <cassert>
namespace SMNetwork
{
	NMessage::NMessage(uint32_t no, shared_ptr<string> msg) : _len(msg->length()), _no(no), _platBody(msg)
	{

	}
	NMessage::NMessage(uint32_t no, string_view msg) : _len(msg.length()), _no(no), _platBody(std::make_shared<string>(msg.begin(), msg.end()))
	{

	}
	NMessage::NMessage(NMessage& copy)
	{
		assert(0);
	}

	uint32_t NMessage::Len()
	{
		return _len;
	}
	uint32_t NMessage::No()
	{
		return _no;
	}
	shared_ptr<string> NMessage::body()
	{
		return _platBody;
	}
	void NMessage::body(shared_ptr<string> body)
	{
		_platBody = body;
	}
}
