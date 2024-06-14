#include "ExternalInterrupts.hpp"

#include "../Logger.hpp"
#include "../Chipset/MMU.hpp"
#include "../Emulator.hpp"
#include "../Chipset/Chipset.hpp"

namespace casioemu
{
    void ExternalInterrupts::Initialise() {
        clock_type = CLOCK_UNDEFINED;

        emulator.chipset.data_EXICON = 0;

        region_EXICON.Setup(0xF018, 1, "ExternalInterrupts/EXICON", &emulator.chipset.data_EXICON, MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);
        region_F048.Setup(0xF048, 1, "ExternalInterrupts/F048", this, [](MMURegion* region, size_t) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            uint8_t pin_levels = self->pin_levels[1] | (self->pin_levels[2] << 1) | (self->pin_levels[3] << 2);
            return (uint8_t)(((pin_levels & self->data_F04C) | (self->data_F048 & ~self->data_F04C)) & 0x07);
        }, [](MMURegion* region, size_t, uint8_t data) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            self->data_F048 = data & 0x07;
        }, emulator);
        region_F049.Setup(0xF049, 1, "ExternalInterrupts/F049", &data_F049, MMURegion::DefaultRead<uint8_t, 0x01>, MMURegion::DefaultWrite<uint8_t, 0x01>, emulator);
        region_F04A.Setup(0xF04A, 1, "ExternalInterrupts/F04A", this, [](MMURegion* region, size_t) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            return self->data_F04A;
        }, [](MMURegion* region, size_t, uint8_t data) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            self->data_F04A = data & 0x07;
        }, emulator);
        region_F04B.Setup(0xF04B, 1, "ExternalInterrupts/F04B", this, [](MMURegion* region, size_t) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            return self->data_F04B;
        }, [](MMURegion* region, size_t, uint8_t data) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            self->data_F04B = data & 0x07;
        }, emulator);
        region_F04C.Setup(0xF04C, 1, "ExternalInterrupts/F04C", this, [](MMURegion* region, size_t) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            return self->data_F04C;
        }, [](MMURegion* region, size_t, uint8_t data) {
            ExternalInterrupts* self = (ExternalInterrupts*)region->userdata;
            self->data_F04C = data & 0x07;
        }, emulator);
    }

    void ExternalInterrupts::Tick() {
        //EXI0INT is handled by keyboard
        for(int index = 1; index <= 3; index++) {
            switch ((emulator.chipset.data_EXICON >> (2 * index)) & 0x03)
            {
            case 0:
                if(pin_levels_last[index] && !pin_levels[index])
                    emulator.chipset.MaskableInterrupts[EXIINTS[index]].TryRaise();
                break;
            case 1:
                if(!pin_levels_last[index] && pin_levels[index])
                    emulator.chipset.MaskableInterrupts[EXIINTS[index]].TryRaise();
                break;
            case 2:
                if(pin_levels[index])
                    emulator.chipset.MaskableInterrupts[EXIINTS[index]].TryRaise();
                break;
            case 3:
                if(!pin_levels[index])
                    emulator.chipset.MaskableInterrupts[EXIINTS[index]].TryRaise();
                break;
            default:
                break;
            }
            pin_levels_last[index] = pin_levels[index];
        }
    }

    void ExternalInterrupts::Reset() {
        emulator.chipset.data_EXICON = 0;
        
        for(int index = 0; index <= 3; index++) {
            pin_levels[index] = true;
            pin_levels_last[index] = true;
        }
    }
}