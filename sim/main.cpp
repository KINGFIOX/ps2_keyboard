#include <cstdio>
#include <nvboard.h>
#include "VPS2Keyboard.h"
#include "verilated.h"

void nvboard_bind_all_pins(VPS2Keyboard *top);

static VPS2Keyboard *dut = nullptr;

static void single_cycle() {
    dut->clock = 0;
    dut->eval();
    dut->clock = 1;
    dut->eval();
}

static void reset(int n) {
    dut->reset = 1;
    while (n-- > 0) single_cycle();
    dut->reset = 0;
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);

    dut = new VPS2Keyboard;

    nvboard_bind_all_pins(dut);
    nvboard_init();

    reset(10);

    while (!Verilated::gotFinish()) {
        nvboard_update();
        single_cycle();
    }

    nvboard_quit();
    delete dut;
    return 0;
}
