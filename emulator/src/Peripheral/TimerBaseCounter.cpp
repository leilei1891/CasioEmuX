#include "TimerBaseCounter.hpp"

#include "../Logger.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
    void TimerBaseCounter::Initialise() {
        clock_type = CLOCK_LSCLK;

        LTBRCounter = 0;
        current_LTBR = 0;
    }

    void TimerBaseCounter::Reset() {
        current_LTBR = 0;
        LTBRCounter = 0;
    }

    void TimerBaseCounter::Tick() {
        if(++LTBRCounter >= LTBROutputCount) {
            LTBRCounter = 0;
            emulator.chipset.data_LTBR++;
            current_LTBR = emulator.chipset.data_LTBR;
            if(!(current_LTBR & 0x02) && ((current_LTBR - 1) & 0x02))
                emulator.chipset.MaskableInterrupts[L256SINT].TryRaise();
            if(!(current_LTBR & 0x08) && ((current_LTBR - 1) & 0x08))
                emulator.chipset.MaskableInterrupts[L1024SINT].TryRaise();
            if(!(current_LTBR & 0x20) && ((current_LTBR - 1) & 0x20))
                emulator.chipset.MaskableInterrupts[L4096SINT].TryRaise();
            if(!(current_LTBR & 0x80) && ((current_LTBR - 1) & 0x80))
                emulator.chipset.MaskableInterrupts[L16384SINT].TryRaise();
        }
    }

    void TimerBaseCounter::ResetLSCLK() {
        current_LTBR = 0;
        emulator.chipset.MaskableInterrupts[L256SINT].TryRaise();
        emulator.chipset.MaskableInterrupts[L1024SINT].TryRaise();
        emulator.chipset.MaskableInterrupts[L4096SINT].TryRaise();
        emulator.chipset.MaskableInterrupts[L16384SINT].TryRaise();
    }
}