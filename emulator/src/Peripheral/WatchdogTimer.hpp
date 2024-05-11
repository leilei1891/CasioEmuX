#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class WatchdogTimer : public Peripheral
	{
		MMURegion region_WDTCON, region_WDTMOD;
        uint8_t data_WDTCON, data_WDTMOD;

        bool data_WDP;
        
        size_t WDT_counter;
        bool overflow_count;

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
		void Tick();
	};
}
