#include "Timer.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

#include <cmath>

namespace casioemu
{
	void Timer::Initialise()
	{
		real_hardware = emulator.GetModelInfo("real_hardware");
		cycles_per_second = emulator.GetCyclesPerSecond();

		TimerFreqDiv = 1;
		clock_type = CLOCK_LSCLK;

		EmuStopped = false;
		emulator.chipset.EmuTimerSkipped = false;
		
		region_interval.Setup(0xF020, 2, "Timer/TM0D", &data_interval, MMURegion::DefaultRead<uint16_t>, [](MMURegion *region, size_t offset, uint8_t data) {
			uint16_t *value = (uint16_t *)(region->userdata);
			*value &= ~(((uint16_t)0xFF) << ((offset - region->base) * 8));
			*value |= ((uint16_t)data) << ((offset - region->base) * 8);
			// if (!*value)
			// 	*value = 1;
		}, emulator);

		region_counter.Setup(0xF022, 2, "Timer/TM0C", &data_counter, MMURegion::DefaultRead<uint16_t>, [](MMURegion *region, size_t, uint8_t) {
			*((uint16_t *)region->userdata) = 0;
		}, emulator);

		region_F024.Setup(0xF024, 1, "Timer/TM0CON0", this, [](MMURegion *region, size_t) {
			Timer *timer = (Timer *)region->userdata;
			return (uint8_t)(timer->data_F024 & 0x0F);
		}, [](MMURegion *region, size_t, uint8_t data) {
			Timer *timer = (Timer *)region->userdata;
			timer->data_F024 = data & 0x0F;
			timer->TimerFreqDiv = std::pow(2, data & 0x07);
			if(data & 0x08)
				timer->clock_type = CLOCK_HSCLK;
			else
				timer->clock_type = CLOCK_LSCLK;
		}, emulator);

		region_control.Setup(0xF025, 1, "Timer/TM0CON1", this, [](MMURegion *region, size_t) {
			Timer *timer = (Timer *)region->userdata;
			return (uint8_t)(timer->data_control & 0x01);
		}, [](MMURegion *region, size_t, uint8_t data) {
			Timer *timer = (Timer *)region->userdata;
			timer->data_control = data & 0x01;
			timer->raise_required = false;
		}, emulator);
	}

	void Timer::Reset()
	{
		ext_to_int_counter = 0;
		if(!real_hardware) {
			ext_to_int_next = 0;
			ext_to_int_int_done = 0;
			cycles_per_second = emulator.GetCyclesPerSecond();
			DivideTicks();
			raise_required = false;
			EmuStopped = false;
			emulator.chipset.EmuTimerSkipped = false;
		}

		data_interval = 0;
		data_counter = 0;
		data_control = 0;
		data_F024 = 0;
	}

	void Timer::Tick()
	{
		if(!real_hardware) {
			if (ext_to_int_counter == ext_to_int_next)
				DivideTicks();
			++ext_to_int_counter;

			if (raise_required)
				emulator.chipset.MaskableInterrupts[TM0INT].TryRaise();
		} else if(data_control) {
			if(++ext_to_int_counter >= TimerFreqDiv) {
				ext_to_int_counter = 0;
				if(!data_interval) {
					data_counter = 0;
				} else if(++data_counter >= data_interval) {
					data_counter = 0;
					emulator.chipset.MaskableInterrupts[TM0INT].TryRaise();
				}
			}
		}
	}

	void Timer::TickAfterInterrupts()
	{
		if(!real_hardware) {
			raise_required = false;

			if(EmuStopped && emulator.chipset.GetRunningState()) {
				EmuStopped = false;
				emulator.chipset.EmuTimerSkipped = false;
			}
		}
	}

	void Timer::DivideTicks()
	{
		if(emulator.hardware_id == HW_CLASSWIZ_II && !real_hardware) {
			if(!emulator.chipset.GetRunningState()) {
				EmuStopped = true;
			}
		}
		++ext_to_int_int_done;
		if (ext_to_int_int_done == ext_to_int_frequency)
		{
			ext_to_int_int_done = 0;
			ext_to_int_counter = 0;
			cycles_per_second = emulator.GetCyclesPerSecond();
		}
		ext_to_int_next = cycles_per_second * (ext_to_int_int_done + 1) / ext_to_int_frequency;

		if (data_control & 0x01)
		{
			if (data_counter >= (emulator.chipset.EmuTimerSkipped ? 1 : data_interval))
			{
				data_counter = 0;
				raise_required = true;
			}
			++data_counter;
		}
	}
}
