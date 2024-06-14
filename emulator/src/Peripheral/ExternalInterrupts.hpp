#pragma once
#include "../Config.hpp"

#include "Peripheral.hpp"
#include "../Chipset/MMURegion.hpp"

namespace casioemu
{
	class ExternalInterrupts : public Peripheral
	{
		MMURegion region_EXICON, region_F048, region_F049, region_F04A, region_F04B, region_F04C;
        uint8_t data_F048, data_F049, data_F04A, data_F04B, data_F04C;

        bool pin_levels[4];
        bool pin_levels_last[4];

        size_t EXIINTS[4] = {0, 1, 2, 3};

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Reset();
		void Tick();
	};
}
