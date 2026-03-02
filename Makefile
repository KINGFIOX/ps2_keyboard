TOPMODULE      := PS2Keyboard

BUILD_DIR      := build
RTL_DIR        := $(BUILD_DIR)/rtl
OBJ_DIR        := $(BUILD_DIR)/obj_dir
MESON_DIR      := $(BUILD_DIR)/meson

VERILATOR      := verilator
FIRTOOL        := $(CIRCT_INSTALL_PATH)/bin/firtool

CHISEL_SRCS    := $(wildcard ps2_keyboard/src/*.scala elaborator/src/*.scala)

TARGET         := $(MESON_DIR)/ps2_keyboard_sim

.PHONY: all verilog verilate build run clean

all: build

# ==============================================================================
#  Stage 1  Chisel -> Verilog
# ==============================================================================

verilog: $(RTL_DIR)/$(TOPMODULE).sv

$(RTL_DIR)/$(TOPMODULE).fir: $(CHISEL_SRCS)
	@mkdir -p $(RTL_DIR)
	mill elaborator.run --target-dir $(CURDIR)/$(RTL_DIR)

$(RTL_DIR)/$(TOPMODULE).sv: $(RTL_DIR)/$(TOPMODULE).fir
	$(FIRTOOL) $< \
		--disable-all-randomization \
		--strip-debug-info \
		--lowering-options=disallowLocalVariables \
		--split-verilog -o $(RTL_DIR)

# ==============================================================================
#  Stage 2  Verilog -> C++ model
# ==============================================================================

verilate: $(OBJ_DIR)/V$(TOPMODULE)__ALL.a

$(OBJ_DIR)/V$(TOPMODULE)__ALL.a: $(RTL_DIR)/$(TOPMODULE).sv
	@mkdir -p $(OBJ_DIR)
	$(VERILATOR) --cc --build -j 0 \
		--top-module $(TOPMODULE) \
		--Mdir $(OBJ_DIR) \
		-Wno-fatal \
		-I$(RTL_DIR) \
		$<

# ==============================================================================
#  Stage 3  C++ -> ELF  (meson + ninja)
# ==============================================================================

build: $(OBJ_DIR)/V$(TOPMODULE)__ALL.a
	@if [ ! -f $(MESON_DIR)/build.ninja ]; then \
		meson setup $(MESON_DIR); \
	fi
	ninja -C $(MESON_DIR)

# ==============================================================================
#  Run
# ==============================================================================
# NVBoard 运行时需要 NVBOARD_HOME 指向资源目录（字体、图片等）
NVBOARD_HOME ?= $(CURDIR)/subprojects/nvboard

run: build
	@NVBOARD_HOME="$(NVBOARD_HOME)" SDL_VIDEODRIVER=x11 $(TARGET)

# ==============================================================================
#  Clean
# ==============================================================================

clean:
	rm -rf $(BUILD_DIR)
