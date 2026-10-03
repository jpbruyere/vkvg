project(
  'vkvg',
  'c',
  version: '0.3.0',
  default_options: ['c_std=c11', 'cpp_std=c++11'],
)

subproject('vkh')

deps = [
  dependency('vkh', fallback: ['vkh', 'vkh_dep']),
  dependency('threads'),
  dependency('harfbuzz', required: false),
  dependency('freetype', required: false),
  dependency('vulkan', optional_modules: ['glslangValidator']),
]

src = [
  'src/cross_os.c',
  'src/cross_os.h',
  'src/deps/tinycthread.c',
  'src/deps/tinycthread.h',
  'src/nsvg/nanosvg.h',
  'src/nsvg/vkvg_nsvg.c',
  'src/recording/vkvg_record.c',
  'src/recording/vkvg_record_internal.c',
  'src/recording/vkvg_record_internal.h',
  'src/shaders.h',
  'src/stb_image.h',
  'src/stb_image_write.h',
  'src/stb_truetype.h',
  'src/vectors.h',
  'src/vkvg_context.c',
  'src/vkvg_context_internal.c',
  'src/vkvg_context_internal.h',
  'src/vkvg_device.c',
  'src/vkvg_device_internal.c',
  'src/vkvg_device_internal.h',
  'src/vkvg_experimental.c',
  'src/vkvg_experimental.h',
  'src/vkvg_fonts.c',
  'src/vkvg_fonts.h',
  'src/vkvg_internal.h',
  'src/vkvg_matrix.c',
  'src/vkvg_matrix.h',
  'src/vkvg_pattern.c',
  'src/vkvg_pattern.h',
  'src/vkvg_surface.c',
  'src/vkvg_surface_internal.c',
  'src/vkvg_surface_internal.h',
]

incdir = include_directories(
  [
    'include',
    'src',
    'src/recording',
    'src/nsvg',
    'src/deps',
  ],
)

c_args = [
  '-DVKVG_VERSION_MAJOR=@0@'.format(meson.project_version().split('.')[0]),
  '-DVKVG_VERSION_MINOR=@0@'.format(meson.project_version().split('.')[1]),
  '-DVKVG_VERSION_REVISION=@0@'.format(meson.project_version().split('.')[2]),
]

if get_option('ENABLE_PROFILING')
  c_args += [
    '-pg',
    '-fno-reorder-functions',
    '-fno-inline',
    '-fthread-jumps',
    '-falign-functions',
    '-falign-jumps',
    '-falign-loops',
    '-falign-labels',
  ]
endif

if get_option('buildtype').startswith('debug')
  c_args += '-DDebug'
endif

glsl_defs = ['-DVKVG_PREMULT_ALPHA']
glslang_validator = find_program('glslangValidator')
python = find_program('python3')

shaders = [
  'shaders/shader.comp',
  'shaders/shader2.comp',
  'shaders/vkvg_main.frag',
  'shaders/vkvg_main.vert',
  'shaders/vkvg_main_lcd.frag',
  'shaders/wired.frag',
]

shader_targets = []

spv_h_generator = generator(
  glslang_validator,
  output: '@PLAINNAME@.spv.h',
  arguments: [
    glsl_defs,
    '-V',
    '--vn', '@PLAINNAME@',
    '@INPUT@',
    '-o', '@BUILD_DIR@/@PLAINNAME@.spv.h',
  ],
)

shaders_h = custom_target(
  'shaders',
  output: 'shaders.h',
  build_by_default: true,
  input: spv_h_generator.process(shaders),
  command: [
    python,
    '@SOURCE_ROOT@/scripts/headerconcat.py',
    '@INPUT@',
    '-o', '@OUTPUT@',
    '--replace-dots',
  ],
)

src += [shaders_h]

vkvg_lib = library(
  'vkvg',
  src + shaders_h,
  include_directories: incdir,
  dependencies: deps,
  c_args: c_args,
)

vkvg_dep = declare_dependency(
  include_directories: include_directories('include'),
  link_with: vkvg_lib,
)

if get_option('VKVG_BUILD_TESTS')
  subdir('tests')
endif