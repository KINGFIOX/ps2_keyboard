#include "VPS2Keyboard.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <cstdio>
#include <memory>
#include <string>
#include <nvboard.h>

void nvboard_bind_all_pins(VPS2Keyboard *top);

static VPS2Keyboard *dut = nullptr;
static VerilatedVcdC *tfp = nullptr;
static vluint64_t main_time = 0;

static void single_cycle() {
  dut->clock = 0;
  dut->eval();
  if (tfp) tfp->dump(main_time);
  main_time++;

  dut->clock = 1;
  dut->eval();
  if (tfp) tfp->dump(main_time);
  main_time++;
}

static void reset(int n) {
  dut->reset = 1;
  while (n-- > 0)
    single_cycle();
  dut->reset = 0;
}

int main(int argc, char **argv) {
  VerilatedContext * contextp{new VerilatedContext};
  contextp->traceEverOn(true);
  contextp->commandArgs(argc, argv);

  dut = new VPS2Keyboard{contextp, "TOP"};

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "+trace") {
      tfp = new VerilatedVcdC;
      dut->trace(tfp, 0);
      tfp->open("ps2_keyboard.vcd");
      break;
    }
  }

  nvboard_bind_all_pins(dut);
  nvboard_init();

  reset(10);

  while (!contextp->gotFinish()) {
    nvboard_update();
    single_cycle();
  }

  if (tfp) {
    tfp->close();
    delete tfp;
  }
  nvboard_quit();
  delete dut;
  delete contextp;
  return 0;
}
