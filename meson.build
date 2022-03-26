# /////////////////////////////////////////
# This file is Licensed MIT, please refer to LICENSE.md for more information.
# /////////////////////////////////////////

project('voxgui', 'c', 'cpp', version: '0.3.0', license: 'MIT', meson_version: '>=0.62.0', default_options: ['c_std=c11', 'cpp_std=c++11'])
cc = meson.get_compiler('c')

# /////////////////////////////////////////
# VKH Library Build Configurations
# /////////////////////////////////////////

# VKH Options
# Enable Vulkan Validation Layer
ENABLE_VALIDATION_OPT = get_option('ENABLE_VALIDATION_OPT')
# Enable Vulkan Memory Allocator - For more information: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
VKH_ENABLE_VMA = get_option('VKH_ENABLE_VMA')

# VMA Options
# Enable VMA memory recording for debugging
VMA_RECORDING_ENABLED = get_option('VMA_RECORDING_ENABLED')
# Use C++ STL containers instead of VMA's containers
VMA_USE_STL_CONTAINERS = get_option('VMA_USE_STL_CONTAINERS')
# Link statically with Vulkan API
VMA_STATIC_VULKAN_FUNCTIONS = get_option('VMA_STATIC_VULKAN_FUNCTIONS')
# Fetch pointers to Vulkan functions internally (no static linking)
VMA_DYNAMIC_VULKAN_FUNCTIONS = get_option('VMA_DYNAMIC_VULKAN_FUNCTIONS')
# Every allocation will have its own memory block
VMA_DEBUG_ALWAYS_DEDICATED_MEMORY = get_option('VMA_DEBUG_ALWAYS_DEDICATED_MEMORY')
# Automatically fill new allocations and destroyed allocations with some bit pattern
VMA_DEBUG_INITIALIZE_ALLOCATIONS = get_option('VMA_DEBUG_INITIALIZE_ALLOCATIONS')
# Enable single mutex protecting all entry calls to the library
VMA_DEBUG_GLOBAL_MUTEX = get_option('VMA_DEBUG_GLOBAL_MUTEX')
# Never exceed VkPhysicalDeviceLimits::maxMemoryAllocationCount and return error
VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT = get_option('VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT')
# Minimum alignment of all allocations, in bytes.
# Set to more than 1 for debugging purposes only. Must be power of two.
VMA_DEBUG_ALIGNMENT = get_option('VMA_DEBUG_ALIGNMENT')
# Minimum margin before and after every allocation, in bytes.
# Set nonzero for debugging purposes only.
VMA_DEBUG_MARGIN = get_option('VMA_DEBUG_MARGIN')
# Minimum value for VkPhysicalDeviceLimits::bufferImageGranularity.
# Set to more than 1 for debugging purposes only. Must be power of two.
VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY = get_option('VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY')
# Maximum size of a memory heap in Vulkan to consider it "small".
VMA_SMALL_HEAP_MAX_SIZE = get_option('VMA_SMALL_HEAP_MAX_SIZE') # 1 Gigabyte default
# Default size of a block allocated as single VkDeviceMemory from a "large" heap.
VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE = get_option('VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE') # 256 Megabytes default

# Required Dependencies for VKH
vulkan_dep = dependency('vulkan')
threads_dep = dependency('threads')
vkh_dependencies = [vulkan_dep, threads_dep]

# Required Configurations for Debug/Release builds
if (get_option('debug') == true)
    vkh_compile_options = ['-DDEBUG']
    ENABLE_VALIDATION_OPT = true
    if (build_machine.system() == 'linux')
        vkh_compile_options += ['-Wall', '-Wno-extra', '-Wno-unknown-pragmas']
    elif (build_machine.system() == 'windows')
        vkh_compile_options += ['/W4', '/wd4204', '/wd4221', '/wd4100']
    endif
else
    vkh_compile_options = []
    if (build_machine.system() == 'linux')
        vkh_compile_options += ['-w']
    elif (build_machine.system() == 'windows')
        vkh_compile_options += ['/W0']
    endif
endif

# VKH Options Define Flags
if (VKH_ENABLE_VMA == true)
    vkh_compile_options += '-DUSE_VMA'
endif

if (ENABLE_VALIDATION_OPT == true)
    vkh_compile_options += '-DVKH_USE_VALIDATION'
endif

# VMA Options Define Flags
if (VMA_RECORDING_ENABLED == true)
    vkh_compile_options += '-DVMA_RECORDING_ENABLED=1'
endif

if (VMA_USE_STL_CONTAINERS == true)
    vkh_compile_options += '-DVMA_USE_STL_CONTAINERS=1'
endif

if (VMA_STATIC_VULKAN_FUNCTIONS == true)
    vkh_compile_options += '-DVMA_STATIC_VULKAN_FUNCTIONS=1'
endif

if (VMA_DYNAMIC_VULKAN_FUNCTIONS == true)
    vkh_compile_options += '-DVMA_DYNAMIC_VULKAN_FUNCTIONS=1'
endif

if (VMA_DEBUG_ALWAYS_DEDICATED_MEMORY == true)
    vkh_compile_options += '-DVMA_DEBUG_ALWAYS_DEDICATED_MEMORY=1'
endif

if (VMA_DEBUG_INITIALIZE_ALLOCATIONS == true)
    vkh_compile_options += '-DVMA_DEBUG_INITIALIZE_ALLOCATIONS=1'
endif

if (VMA_DEBUG_GLOBAL_MUTEX == true)
    vkh_compile_options += '-DVMA_DEBUG_GLOBAL_MUTEX=1'
endif

if (VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT == true)
    vkh_compile_options += '-DVMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT=1'
endif

if (VMA_DEBUG_ALIGNMENT != 1)
    vkh_compile_options += '-DVMA_DEBUG_ALIGNMENT=' + VMA_DEBUG_ALIGNMENT
endif

if (VMA_DEBUG_MARGIN != 0)
    vkh_compile_options += '-DVMA_DEBUG_MARGIN=' + VMA_DEBUG_MARGIN
endif

if (VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY != 1)
    vkh_compile_options += '-DVMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY=' + VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY
endif

if (VMA_SMALL_HEAP_MAX_SIZE != 1073741824) # 1 Gigabyte
    vkh_compile_options += '-DVMA_SMALL_HEAP_MAX_SIZE=' + VMA_SMALL_HEAP_MAX_SIZE
endif

if (VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE != 268435456) # 256 Megabytes
    vkh_compile_options += '-DVMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE=' + VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE
endif

vkh_src = [
    'vkh/src/vkh_app.c',
    'vkh/src/vkh_buffer.c',
    'vkh/src/vkh_device.c',
    'vkh/src/vkh_image.c',
    'vkh/src/vkh_phyinfo.c',
    'vkh/src/vkh_presenter.c',
    'vkh/src/vkh_queue.c',
    'vkh/src/vkhelpers.c',
    'vkh/src/deps/tinycthread.c',
    'vkh/src/VmaUsage.cpp'
]

vkh_include = [
    'vkh/include',
    'vkh/src',
    'vkh/src/deps'
]

vkh_shared_library = shared_library('vkh', 
                                    c_args: [vkh_compile_options, '-DVKH_SHARED_BUILD'],
                                    cpp_args: [vkh_compile_options, '-DVKH_SHARED_BUILD'],
                                    build_by_default: true,
                                    install: true,
                                    soversion: '1',
                                    sources: vkh_src,
                                    include_directories: vkh_include,
                                    dependencies: vkh_dependencies
)

vkh_static_library = static_library('vkh', 
                                    c_args: [vkh_compile_options, '-DVKH_STATIC_BUILD'],
                                    cpp_args: [vkh_compile_options, '-DVKH_STATIC_BUILD'],
                                    build_by_default: true,
                                    install: true,
                                    sources: vkh_src,
                                    include_directories: vkh_include,
                                    dependencies: vkh_dependencies
)

install_headers('vkh/include/vkh.h', subdir: 'vkh')

vkh_shared_dep = declare_dependency(dependencies: vkh_dependencies, include_directories: 'vkh/include/', link_with: vkh_shared_library)
vkh_static_dep = declare_dependency(dependencies: vkh_dependencies, include_directories: 'vkh/include/', link_with: vkh_static_library)

# /////////////////////////////////////////
# VKVG Library Build Configurations
# /////////////////////////////////////////

# VKVG Options
# Enable VKVG Debug Utilities
VKVG_ENABLE_DBG_UTILS = get_option('VKVG_ENABLE_DBG_UTILS')
# Compile with -pg options
VKVG_ENABLE_PROFILING = get_option('VKVG_ENABLE_PROFILING')
# Enable experimental recording functions
VKVG_RECORDING = get_option('VKVG_RECORDING')
# Use premultiplied alpha for internal rendering
VKVG_PREMULT_ALPHA = get_option('VKVG_PREMULT_ALPHA')
# Record contexts statistics in the device
VKVG_DBG_STATS = get_option('VKVG_DBG_STATS')
# Fill non-zero with glu tesselator
VKVG_USE_GLUTESS = get_option('VKVG_USE_GLUTESS')
# Render svg with vkvg-svg library (Turned off if on Linux/Unix platform)
if (build_machine.system() == 'linux')
    VKVG_SVG = false
else
    VKVG_SVG = true
endif
# Use freetype to load and render font glyphs
VKVG_USE_FREETYPE = get_option('VKVG_USE_FREETYPE')
# Use FontConfig to resolve font names
VKVG_USE_FONTCONFIG = get_option('VKVG_USE_FONTCONFIG')
# Use harfbuzz for text layouting
VKVG_USE_HARFBUZZ = get_option('VKVG_USE_HARFBUZZ')
# Enable freetype lcd font filtering
VKVG_LCD_FONT_FILTER = get_option('VKVG_LCD_FONT_FILTER')
# Enable scalar block layout support
VKVG_VK_SCALAR_BLOCK_SUPPORTED = get_option('VKVG_VK_SCALAR_BLOCK_SUPPORTED')
# Apply validation
VKVG_ENABLE_VALIDATION = get_option('VKVG_ENABLE_VALIDATION')
# Add vulkan layers for supporting RenderDoc
VKVG_ENABLE_RENDERDOC = get_option('VKVG_ENABLE_RENDERDOC')
# Enable Debug Wire Fill
VKVG_ENABLE_WIRED_FILL = get_option('VKVG_ENABLE_WIRED_FILL')

# VKVG Source Code
vkvg_src = [
    'src/cross_os.c',
    'src/vkvg_buff.c',
    'src/vkvg_context_internal.c',
    'src/vkvg_context.c',
    'src/vkvg_device_internal.c',
    'src/vkvg_device.c',
    'src/vkvg_experimental.c',
    'src/vkvg_fonts.c',
    'src/vkvg_matrix.c',
    'src/vkvg_pattern.c',
    'src/vkvg_surface_internal.c',
    'src/vkvg_surface.c'
]

vkvg_include = [
    'include',
    'src',
    'vkh/src/' # Including this directory for vk_mem_alloc.h
]

# Required Dependencies for VKVG
freetype2_dep = dependency('freetype2')
fontconfig_dep = dependency('fontconfig')
harfbuzz_dep = dependency('harfbuzz')
m_dep = cc.find_library('m', required: false)
glfw3_dep = dependency('glfw3')
vkvg_dependencies = []

# VKVG Compile Options
vkvg_compile_options = []

# Disable warning for unused value
if (build_machine.system() == 'linux')
    vkvg_compile_options += ['-Wno-unused-value', '-Wno-unused-variable', '-Wno-unknown-pragmas']
endif
# GLSL Defines
GLSLDEFS = ''

# If VKVG_VK_SCALAR_BLOCK_SUPPORTED is true, include defines for both VKVG and GLSL
if (VKVG_VK_SCALAR_BLOCK_SUPPORTED == true)
    vkvg_compile_options += '-DVKVG_VK_SCALAR_BLOCK_SUPPORTED'
    GLSLDEFS = '-DVKVG_VK_SCALAR_BLOCK_SUPPORTED'
endif

# Shader Compilation Stage
# Attempts to find GLSLC and XXD program
COMPILE_SHADERS = get_option('COMPILE_SHADERS')
ANDROID_NDK = get_option('ANDROID_NDK')
VULKAN_SDK = get_option('VULKAN_SDK')
GLSLC_PATH = get_option('GLSLC_PATH')
XXD_PATH = get_option('XXD_PATH')
if (COMPILE_SHADERS == true)
    glslc = find_program('glslc', '/bin/glslc', '/usr/bin/glslc', ANDROID_NDK / 'shader-tools/', VULKAN_SDK / 'bin', GLSLC_PATH, required: false, disabler: true)
    xxd = find_program('xxd', '/bin/xxd', '/usr/bin/xxd', XXD_PATH, required: false, disabler: true)

    if (glslc.found() == true and xxd.found() == true)
        shaders_h = custom_target('shaders.h', command: ['shaders/helper/GenShaders.sh', '@INPUT@' ], 
            env: {'GLSLC_BIN': glslc.full_path(), 'GLSLDEFS': GLSLDEFS, 'XXD_BIN': xxd.full_path()}, 
            input: ['shaders/shader.comp', 'shaders/shader2.comp', 'shaders/vkvg_main_lcd.frag',
                    'shaders/vkvg_main.frag', 'shaders/vkvg_main.vert', 'shaders/wired.frag'],
            output: 'shaders.h', install: true, install_dir: get_option('includedir') / 'vkvg', build_always_stale: true, build_by_default: true)

        message('Please note that shaders.h was generated, but must be copied manually to src/ folder, because this is the nature of Meson Build restricting in-source modification during build.')
        message('You can find shaders.h in either build directory or install directory under include/vkvg/')
    else
        if (glslc.found() == false)
            message('GLSLC could not be found. Please refer to https://www.lunarg.com/vulkan-sdk/ or https://developer.android.com/ndk/ to obtain GLSLC program. Then specify path using either ANDROID_NDK, VULKAN_SDK, or GLSLC_PATH whichever applicable.')
        endif
        if (xxd.found() == false)
            message('XXD could not be found. Please search for XXD package on your respective platform or refer to https://sourceforge.net/projects/xxd-for-windows/ for Windows-based XXD software. Then specify path using XXD_PATH option.')
        endif
        message('Program(s) for shader compilation could not be found, skipping shader compilation.')
    endif
endif

# VKVG Options Define Flags
if (VKVG_ENABLE_DBG_UTILS == true)
    vkvg_compile_options += '-DVKVG_DBG_UTILS'
endif

if (VKVG_USE_FREETYPE == true)
    vkvg_compile_options += '-DVKVG_USE_FREETYPE'
    vkvg_dependencies += freetype2_dep
endif

if (VKVG_USE_FONTCONFIG == true)
    vkvg_compile_options += '-DVKVG_USE_FONTCONFIG'
    vkvg_dependencies += fontconfig_dep
endif

if (VKVG_USE_HARFBUZZ == true)
    vkvg_compile_options += '-DVKVG_USE_HARFBUZZ'
    vkvg_dependencies += harfbuzz_dep
endif

if (VKVG_LCD_FONT_FILTER == true)
    vkvg_compile_options += '-DVKVG_LCD_FONT_FILTER'
endif

if (VKVG_USE_GLUTESS == true)
    vkvg_compile_options += '-DVKVG_FILL_NZ_GLUTESS'
    vkvg_include += 'external/glutess/include'

    glutess_src = [
        'external/glutess/src/dict.c',
        'external/glutess/src/geom.c', 
        'external/glutess/src/memalloc.c', 
        'external/glutess/src/mesh.c', 
        'external/glutess/src/normal.c', 
        #'external/glutess/src/priorityq-heap.c', Commenting this out, because priorityq.c directly includes this
        'external/glutess/src/priorityq.c',
        'external/glutess/src/render.c',
        'external/glutess/src/sweep.c',
        'external/glutess/src/tess.c',
        'external/glutess/src/tessmono.c'
    ]
    glutess_include = ['external/glutess/src', 'external/glutess/include']
    glutess_dep = declare_dependency(sources: glutess_src, include_directories: glutess_include, compile_args: '-DVKVG_FILL_NZ_GLUTESS')
    vkvg_dependencies += glutess_dep
endif


if (VKVG_ENABLE_VALIDATION == true)
    vkvg_compile_options += '-DVKVG_USE_VALIDATION'
	if (VKVG_ENABLE_RENDERDOC == true)
        vkvg_compile_options += '-DDVKVG_USE_RENDERDOC'
    endif
endif

if (VKVG_ENABLE_WIRED_FILL == true)
    vkvg_compile_options += '-DVKVG_WIRED_DEBUG'
endif

# List of VKVG Public Headers
vkvg_public_headers = ['include/vkvg.h']

if (VKVG_SVG == true)
    vkvg_svg_src = ['external/vkvg-svg/src/fmemopen.c', 'external/vkvg-svg/src/vkvg_svg.c']
    vkvg_svg_include = ['external/vkvg-svg/src']
    vkvg_svg_dep = declare_dependency(sources: vkvg_svg_src, include_directories: vkvg_svg_include, compile_args: '-DVKVG_SVG')
    vkvg_dependencies += vkvg_svg_dep
else
    vkvg_public_headers += 'include/vkvg-svg.h'
    vkvg_src += ['src/nsvg/vkvg_nsvg.c']
    vkvg_include += 'src/nsvg'
endif

if (VKVG_RECORDING == true)
    vkvg_compile_options += '-DVKVG_RECORDING'
    vkvg_src += ['src/recording/vkvg_record_internal.c', 'src/recording/vkvg_record.c']
    vkvg_include += 'src/recording'
endif

if (VKVG_ENABLE_PROFILING == true)
    vkvg_compile_options += [
        '-pg',
        '-fno-reorder-functions',
        '-fno-inline',
		'-O1',
		'-fthread-jumps',
		'-falign-functions',
        '-falign-jumps',
		'-falign-loops',
        '-falign-labels'
    ]
endif

# THIS IS THE PART WHERE DISABLING STRICT ANSI FIXES IT
# Feel free to uncomment this line to see what happens
vkvg_compile_options += '-U__STRICT_ANSI__ '

vkvg_static_library = static_library('vkvg', 
                                    c_args: [vkvg_compile_options, '-DVKVG_STATIC_BUILD'],
                                    cpp_args: [vkvg_compile_options, '-DVKVG_STATIC_BUILD'],
                                    build_by_default: true,
                                    install: true,
                                    sources: vkvg_src,
                                    include_directories: vkvg_include,
                                    dependencies: [vkvg_dependencies, vkh_static_dep]
)

# For shared library, making sure to link in libm on Linux
if (build_machine.system() == 'linux')
    vkvg_dependencies += m_dep
endif

vkvg_shared_library = shared_library('vkvg', 
                                    c_args: [vkvg_compile_options, '-DVKVG_SHARED_BUILD'],
                                    cpp_args: [vkvg_compile_options, '-DVKVG_SHARED_BUILD'],
                                    build_by_default: true,
                                    install: true,
                                    soversion: '0',
                                    sources: vkvg_src,
                                    include_directories: vkvg_include,
                                    dependencies: [vkvg_dependencies, vkh_shared_dep]
)
install_headers(vkvg_public_headers, subdir: 'vkvg')

vkvg_shared_dep = declare_dependency(dependencies: [vkvg_dependencies, vkh_shared_dep], include_directories: 'include/', link_with: vkvg_shared_library)
vkvg_static_dep = declare_dependency(dependencies: [vkvg_dependencies, vkh_static_dep], include_directories: 'include/', link_with: vkvg_static_library)

# Test Engine Codes
test_engine_src = [
    'tests/common/vkengine.c',
    'tests/common/test.c',
    'tests/common/rnd.c',
    'tests/common/tinycthread.c'
]

test_engine_deps = [
    vkvg_static_dep,
    glfw3_dep
]

test_engine_includes = [
    'tests/common',
    'tests/',
    'vkh/src/', # For vkh implementation headers
    'src/' # For vectors.h
]

# Test Cases
INSTALL_TEST = get_option('INSTALL_TEST')
if (INSTALL_TEST == true)
    # tests/ cases
    arcs_test = executable('arcs_test', sources: [test_engine_src, 'tests/arcs.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    bezier_test = executable('bezier_test', sources: [test_engine_src, 'tests/bezier.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    circles_test = executable('circles_test', sources: [test_engine_src, 'tests/circles.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    clip_test = executable('clip_test', sources: [test_engine_src, 'tests/clip.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    colinear_test = executable('colinear_test', sources: [test_engine_src, 'tests/colinear.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    compositing_test = executable('compositing_test', sources: [test_engine_src, 'tests/compositing.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    context_test = executable('context_test', sources: [test_engine_src, 'tests/context.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    curve_test = executable('curve_test', sources: [test_engine_src, 'tests/curve.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    dashes_test = executable('dashes_test', sources: [test_engine_src, 'tests/dashes.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    fill_and_stroke_test = executable('fill_and_stroke_test', sources: [test_engine_src, 'tests/fill_and_stroke.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    fill_non_zero_test = executable('fill_non_zero_test', sources: [test_engine_src, 'tests/fill_non_zero.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    fill_test = executable('fill_test', sources: [test_engine_src, 'tests/fill.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    gradient_test = executable('gradient_test', sources: [test_engine_src, 'tests/gradient.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    gradient2_test = executable('gradient2_test', sources: [test_engine_src, 'tests/gradient2.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    img_surf_test = executable('img_surf_test', sources: [test_engine_src, 'tests/img_surf.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    inverse_colinear_test = executable('inverse_colinear_test', sources: [test_engine_src, 'tests/inverse_colinear.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    line_caps_test = executable('line_caps_test', sources: [test_engine_src, 'tests/line_caps.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    line_join_test = executable('line_join_test', sources: [test_engine_src, 'tests/line_join.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    lines_test = executable('lines_test', sources: [test_engine_src, 'tests/lines.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    offscreen_test = executable('offscreen_test', sources: [test_engine_src, 'tests/offscreen.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    paint_surf_test = executable('paint_surf_test', sources: [test_engine_src, 'tests/paint_surf.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    path_extents_test = executable('path_extents_test', sources: [test_engine_src, 'tests/path_extents.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    pattern_transforms_test = executable('pattern_transforms_test', sources: [test_engine_src, 'tests/pattern_transforms.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    radial_gradient_test = executable('radial_gradient_test', sources: [test_engine_src, 'tests/radial_gradient.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    recording_test = executable('recording_test', sources: [test_engine_src, 'tests/recording.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    rect_fill_test = executable('rect_fill_test', sources: [test_engine_src, 'tests/rect_fill.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    save_restore_test = executable('save_restore_test', sources: [test_engine_src, 'tests/save_restore.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    simple_paint_test = executable('simple_paint_test', sources: [test_engine_src, 'tests/simple_paint.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    stroke_test = executable('stroke_test', sources: [test_engine_src, 'tests/stroke.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    surface_test = executable('surface_test', sources: [test_engine_src, 'tests/surface.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    svg_test = executable('svg_test', sources: [test_engine_src, 'tests/svg.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    test1_test = executable('test1_test', sources: [test_engine_src, 'tests/test1.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    text_test = executable('text_test', sources: [test_engine_src, 'tests/text.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    transform_test = executable('transform_test', sources: [test_engine_src, 'tests/transform.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')

    # tests/multithreading
    multithreaded_test = executable('multithreaded_test', sources: [test_engine_src, 'tests/multithreading/multithreaded.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    multithreaded2_test = executable('multithreaded2_test', sources: [test_engine_src, 'tests/multithreading/multithreaded2.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    threaded_create_surf_test = executable('threaded_create_surf_test', sources: [test_engine_src, 'tests/multithreading/threaded_create_surf.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    threaded_text_test = executable('threaded_text_test', sources: [test_engine_src, 'tests/multithreading/threaded_text.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')

    # perfs
    random_rects_test = executable('random_rects_test', sources: [test_engine_src, 'tests/perfs/random_rects.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')
    randoms_test = executable('randoms_test', sources: [test_engine_src, 'tests/perfs/randoms.c'], dependencies: test_engine_deps, include_directories: test_engine_includes, install: INSTALL_TEST, install_dir: get_option('bindir') / 'vkvg_tests/')

    # Ensure data folder get copied over
    install_subdir('tests/data', install_dir: get_option('bindir') / 'vkvg_tests/data', strip_directory: true)
endif

# Summaries of all of the configuration options
summary({
    'bindir': get_option('bindir'),
    'libdir': get_option('libdir'),
    'datadir': get_option('datadir'),
}, section: 'Directories')

summary ({
    'ENABLE_VALIDATION_OPT': ENABLE_VALIDATION_OPT,
    'VKH_ENABLE_VMA': VKH_ENABLE_VMA
}, section: 'VKH Options')

summary ({
    'VMA_RECORDING_ENABLED': VMA_RECORDING_ENABLED,
    'VMA_USE_STL_CONTAINERS': VMA_USE_STL_CONTAINERS,
    'VMA_STATIC_VULKAN_FUNCTIONS': VMA_STATIC_VULKAN_FUNCTIONS,
    'VMA_DYNAMIC_VULKAN_FUNCTIONS': VMA_DYNAMIC_VULKAN_FUNCTIONS,
    'VMA_DEBUG_ALWAYS_DEDICATED_MEMORY': VMA_DEBUG_ALWAYS_DEDICATED_MEMORY,
    'VMA_DEBUG_INITIALIZE_ALLOCATIONS': VMA_DEBUG_INITIALIZE_ALLOCATIONS,
    'VMA_DEBUG_GLOBAL_MUTEX': VMA_DEBUG_GLOBAL_MUTEX,
    'VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT': VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT,
    'VMA_DEBUG_ALIGNMENT': VMA_DEBUG_ALIGNMENT,
    'VMA_DEBUG_MARGIN': VMA_DEBUG_MARGIN,
    'VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY': VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY,
    'VMA_SMALL_HEAP_MAX_SIZE': VMA_SMALL_HEAP_MAX_SIZE,
    'VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE': VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE,
}, section: 'VMA Options')

summary({
    'VKVG_ENABLE_PROFILING': VKVG_ENABLE_PROFILING,
    'VKVG_RECORDING': VKVG_RECORDING,
    'VKVG_PREMULT_ALPHA': VKVG_PREMULT_ALPHA,
    'VKVG_DBG_STATS': VKVG_DBG_STATS,
    'VKVG_USE_GLUTESS': VKVG_USE_GLUTESS,
    'VKVG_SVG': VKVG_SVG,
    'VKVG_ENABLE_DBG_UTILS': VKVG_ENABLE_DBG_UTILS,
    'VKVG_USE_FREETYPE': VKVG_USE_FREETYPE,
    'VKVG_USE_FONTCONFIG': VKVG_USE_FONTCONFIG,
    'VKVG_USE_HARFBUZZ': VKVG_USE_HARFBUZZ,
    'VKVG_LCD_FONT_FILTER': VKVG_LCD_FONT_FILTER,
    'VKVG_VK_SCALAR_BLOCK_SUPPORTED': VKVG_VK_SCALAR_BLOCK_SUPPORTED,
    'VKVG_ENABLE_VALIDATION': VKVG_ENABLE_VALIDATION,
    'VKVG_ENABLE_RENDERDOC': VKVG_ENABLE_RENDERDOC,
    'VKVG_ENABLE_WIRED_FILL': VKVG_ENABLE_WIRED_FILL
}, section: 'VKVG Options')

summary({
    'INSTALL_TEST': INSTALL_TEST
}, section: 'Test Cases Option')

summary({
    'COMPILE_SHADERS': COMPILE_SHADERS,
    'ANDROID_NDK': ANDROID_NDK,
    'VULKAN_SDK': VULKAN_SDK,
    'GLSLC_PATH': GLSLC_PATH,
    'XXD_PATH': XXD_PATH
}, section: 'Shader Compilation Options')