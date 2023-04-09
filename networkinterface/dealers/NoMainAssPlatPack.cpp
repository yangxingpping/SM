#include "NoMainAssPlatPack.h"
#include <assert.h>

namespace SMNetwork
{
	NoMainAssPlatPack::NoMainAssPlatPack(int mainc)
		: _main(mainc)
	{
		assert(sizeof(int) == sizeof(uint32_t));
	}

	NoMainAssPlatPack::NoMainAssPlatPack(NoMainAssPlatPack& ref)
		:_main(ref._main)
	{

	}

	NoMainAssPlatPack::~NoMainAssPlatPack()
	{

	}

	int NoMainAssPlatPack::getMain()
	{
		return static_cast<int>(_main);
	}



	bool NoMainAssPlatPack::pack(span<char> dst)
	{
		return true;
	}

	bool NoMainAssPlatPack::unpack(string_view src)
	{
		return true;
	}

	size_t NoMainAssPlatPack::len()
	{
		return 0;
	}

	PlatformPackInterface* NoMainAssPlatPack::clone()
	{
		return new NoMainAssPlatPack(*this);
	}
}
