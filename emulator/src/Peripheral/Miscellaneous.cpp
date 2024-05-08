#include "Miscellaneous.hpp"

#include "../Logger.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Chipset/CPU.hpp"

#include <sstream>
#include <iomanip>

namespace casioemu
{
	constexpr uint16_t Miscellaneous::addr [];

	void Miscellaneous::Initialise()
	{
		region_dsr.Setup(0xF000, 1, "Miscellaneous/DSR", this, [](MMURegion *region, size_t) {
			return (uint8_t)((Miscellaneous *)region->userdata)->emulator.chipset.cpu.impl_last_dsr;
		}, [](MMURegion *region, size_t, uint8_t data) {
			((Miscellaneous *)region->userdata)->emulator.chipset.cpu.impl_last_dsr = data;
		}, emulator);

		// * TODO: figure out what these are

		int n_byte;
		switch (emulator.hardware_id)
		{
		case HW_ES_PLUS:
			n_byte = 3;
			break;
		case HW_CLASSWIZ:
			n_byte = 9;
			break;
		case HW_CLASSWIZ_II:
			n_byte = 9;
			break;
		default:
			PANIC("Unknown hardware_id\n");
		}
		for (int i = 0; i < n_byte; ++ i)
		{
			std::ostringstream stream;
			stream << "Miscellaneous/Unknown/" << std::hex << std::uppercase << addr[i] << "*1";
			region[i].Setup(addr[i], 1, stream.str(), &data[i], MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);
		}
		region_F048.Setup(0xF048, 8, "Miscellaneous/Unknown/F048*8", &data_F048, MMURegion::DefaultRead<uint64_t>, MMURegion::DefaultWrite<uint64_t>, emulator);
		region_F220.Setup(0xF220, 4, "Miscellaneous/Unknown/F220*4", &data_F220, MMURegion::DefaultRead<uint32_t>, MMURegion::DefaultWrite<uint32_t>, emulator);

		region_INT.Setup(0xF018, 1, "Miscellaneous/Interrupts/F018", &data_INT, MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);
	}

	void Miscellaneous::Tick() {
		if((data_INT & 0xC0) == 0xC0)
			emulator.chipset.MaskableInterrupts[3].TryRaise();
		if((data_INT & 0x30) == 0x30)
			emulator.chipset.MaskableInterrupts[2].TryRaise();
		if((data_INT & 0x0C) == 0x0C)
			emulator.chipset.MaskableInterrupts[1].TryRaise();
	}

	void Miscellaneous::Reset() {
		data_INT = 0;
	}
}
