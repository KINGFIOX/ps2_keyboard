#include "VPS2Keyboard.h"
#include "verilated.h"
#include "verilated_fst_c.h"
#include <cstdint>
#include <cstdio>
#include <nvboard.h>
#include <string>

void nvboard_bind_all_pins(VPS2Keyboard *top);

static VPS2Keyboard *dut = nullptr;

// PS/2 Set 1 部分 scancode -> 可读名称 (make code)
static const char *scancode_to_str(uint8_t sc) {
  switch (sc) {
  case 0x1C:
    return "A";
  case 0x32:
    return "B";
  case 0x21:
    return "C";
  case 0x23:
    return "D";
  case 0x24:
    return "E";
  case 0x2B:
    return "F";
  case 0x34:
    return "G";
  case 0x33:
    return "H";
  case 0x43:
    return "I";
  case 0x3B:
    return "J";
  case 0x42:
    return "K";
  case 0x4B:
    return "L";
  case 0x3A:
    return "M";
  case 0x31:
    return "N";
  case 0x44:
    return "O";
  case 0x4D:
    return "P";
  case 0x15:
    return "Q";
  case 0x2D:
    return "R";
  case 0x1B:
    return "S";
  case 0x2C:
    return "T";
  case 0x3C:
    return "U";
  case 0x2A:
    return "V";
  case 0x1D:
    return "W";
  case 0x22:
    return "X";
  case 0x35:
    return "Y";
  case 0x1A:
    return "Z";
  case 0x45:
    return "0";
  case 0x16:
    return "1";
  case 0x1E:
    return "2";
  case 0x26:
    return "3";
  case 0x25:
    return "4";
  case 0x2E:
    return "5";
  case 0x36:
    return "6";
  case 0x3D:
    return "7";
  case 0x3E:
    return "8";
  case 0x46:
    return "9";
  case 0x29:
    return "Space";
  case 0x5A:
    return "Enter";
  case 0x66:
    return "Backspace";
  case 0x76:
    return "Esc";
  case 0x0D:
    return "Tab";
  case 0xF0:
    return "(break)";
  default:
    return nullptr;
  }
}
static VerilatedFstC *tfp = nullptr;
static vluint64_t main_time = 0;

static void tick() {
  dut->clock = 0;
  dut->eval();
  if (tfp)
    tfp->dump(main_time);
  main_time++;

  dut->clock = 1;
  dut->eval();
  if (tfp)
    tfp->dump(main_time);
  main_time++;
}

static void reset(int n) {
  dut->reset = 1;
  while (n-- > 0)
    tick();
  dut->reset = 0;
}

int main(int argc, char **argv) {
  VerilatedContext *contextp{new VerilatedContext};
  contextp->traceEverOn(true);
  contextp->commandArgs(argc, argv);

  dut = new VPS2Keyboard{contextp, "TOP"};

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "+trace") {
      tfp = new VerilatedFstC;
      dut->trace(tfp, 0);
      tfp->open("ps2_keyboard.fst");
      break;
    }
  }

  nvboard_bind_all_pins(dut);
  nvboard_init();

  reset(10);

  dut->nextdata = 0;
  printf("PS/2 keyboard simulation started, press keys in NVBoard to print to this terminal.\n");

  while (!contextp->gotFinish()) {
    nvboard_update();

    if (dut->valid) {
      uint8_t sc = dut->data;
      const char *name = scancode_to_str(sc);
      if (name) {
        printf("[key code] 0x%02x (%s)\n", sc, name);
      } else {
        printf("[key code] 0x%02x\n", sc);
      }
      if (dut->overflow) {
        printf("  [warning] FIFO overflow\n");
      }
      dut->nextdata = 1;
    } else {
      dut->nextdata = 0;
    }
    tick();
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
