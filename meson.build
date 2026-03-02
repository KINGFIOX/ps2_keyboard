project('ps2_keyboard_nvboard', 'cpp',
  version: '0.1.0',
  meson_version: '>=0.56.0',
  default_options: ['cpp_std=c++17', 'optimization=2'],
)

python3 = find_program('python3')

nvboard_dir = get_option('nvboard_dir')
nvboard_src_root = meson.project_source_root() / nvboard_dir
obj_dir = meson.project_source_root() / get_option('verilator_obj_dir')

nvboard_proj = subproject('nvboard')
nvboard_dep = nvboard_proj.get_variable('nvboard_dep')

verilator_prog = find_program('verilator')
verilator_root = run_command(
  verilator_prog, '--getenv', 'VERILATOR_ROOT', check: true,
).stdout().strip()

auto_bind = custom_target('auto_bind',
  input: files('constr/ps2.nxdc'),
  output: 'auto_bind.cpp',
  env: {'NVBOARD_HOME': nvboard_src_root},
  command: [
    python3,
    nvboard_src_root / 'scripts' / 'auto_pin_bind.py',
    '@INPUT@',
    '@OUTPUT@',
  ],
)

verilated_cpp = custom_target('copy_verilated',
  output: 'verilated.cpp',
  command: ['cp', verilator_root / 'include' / 'verilated.cpp', '@OUTPUT@'],
)

verilated_threads_cpp = custom_target('copy_verilated_threads',
  output: 'verilated_threads.cpp',
  command: ['cp', verilator_root / 'include' / 'verilated_threads.cpp', '@OUTPUT@'],
)

verilated_vcd_cpp = custom_target('copy_verilated_vcd',
  output: 'verilated_vcd_c.cpp',
  command: ['cp', verilator_root / 'include' / 'verilated_vcd_c.cpp', '@OUTPUT@'],
)

executable('ps2_keyboard_sim',
  sources: ['sim/main.cpp', auto_bind, verilated_cpp, verilated_threads_cpp, verilated_vcd_cpp],
  include_directories: [
    include_directories('sim'),
    include_directories(verilator_root / 'include'),
  ],
  cpp_args: ['-I' + obj_dir],
  link_args: [obj_dir / 'VPS2Keyboard__ALL.a'],
  dependencies: [nvboard_dep],
)
