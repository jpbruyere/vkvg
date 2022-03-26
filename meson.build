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
ENABLE_VALIDATION_OPT = false
# Enable Vulkan Memory Allocator - For more information: https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
VKH_ENABLE_VMA = false

# VMA Options
# Enable VMA memory recording for debugging
VMA_RECORDING_ENABLD = false
# Use C++ STL containers instead of VMA's containers
VMA_USE_STL_CONTAINERS = false
# Link statically with Vulkan API
VMA_STATIC_VULKAN_FUNCTIONS = false
# Fetch pointers to Vulkan functions internally (no static linking)
VMA_DYNAMIC_VULKAN_FUNCTIONS = true
# Every allocation will have its own memory block
VMA_DEBUG_ALWAYS_DEDICATED_MEMORY = false
# Automatically fill new allocations and destroyed allocations with some bit pattern
VMA_DEBUG_INITIALIZE_ALLOCATIONS = false
# Enable single mutex protecting all entry calls to the library
VMA_DEBUG_GLOBAL_MUTEX = false
# Never exceed VkPhysicalDeviceLimits::maxMemoryAllocationCount and return error
VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT = false
# Minimum alignment of all allocations, in bytes.
# Set to more than 1 for debugging purposes only. Must be power of two.
VMA_DEBUG_ALIGNMENT = 1
# Minimum margin before and after every allocation, in bytes.
# Set nonzero for debugging purposes only.
VMA_DEBUG_MARGIN = 0
# Minimum value for VkPhysicalDeviceLimits::bufferImageGranularity.
# Set to more than 1 for debugging purposes only. Must be power of two.
VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY = 1
# Maximum size of a memory heap in Vulkan to consider it "small".
VMA_SMALL_HEAP_MAX_SIZE = 1073741824 # 1 Gigabyte
# Default size of a block allocated as single VkDeviceMemory from a "large" heap.
VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE = 268435456 # 256 Megabytes

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
if (VMA_RECORDING_ENABLD == true)
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
VKVG_ENABLE_DBG_UTILS = false
# Compile with -pg options
VKVG_ENABLE_PROFILING = false
# Enable experimental recording functions
VKVG_RECORDING = true
# Use premultiplied alpha for internal rendering
VKVG_PREMULT_ALPHA = true
# Record contexts statistics in the device
VKVG_DBG_STATS = true
# Fill non-zero with glu tesselator
VKVG_USE_GLUTESS = true
# Render svg with vkvg-svg library (Turned off if on Linux/Unix platform)
if (build_machine.system() == 'linux')
    VKVG_SVG = false
else
    VKVG_SVG = true
endif
# Use freetype to load and render font glyphs
VKVG_USE_FREETYPE = true
# Use FontConfig to resolve font names
VKVG_USE_FONTCONFIG = true
# Use harfbuzz for text layouting
VKVG_USE_HARFBUZZ = true
# Enable freetype lcd font filtering
VKVG_LCD_FONT_FILTER = false
# Enable scalar block layout support
VKVG_VK_SCALAR_BLOCK_SUPPORTED = true
# Apply validation
VKVG_ENABLE_VALIDATION = false
# Add vulkan layers for supporting RenderDoc
VKVG_ENABLE_RENDERDOC = false
# Enable Debug Wire Fill
VKVG_ENABLE_WIRED_FILL = false

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
GLSLDEFS = []

# If VKVG_VK_SCALAR_BLOCK_SUPPORTED is true, include defines for both VKVG and GLSL
if (VKVG_VK_SCALAR_BLOCK_SUPPORTED == true)
    vkvg_compile_options += '-DVKVG_VK_SCALAR_BLOCK_SUPPORTED'
    GLSLDEFS += '-DVKVG_VK_SCALAR_BLOCK_SUPPORTED'
endif

# TODO: Write shader generation build recipe here

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
    'VMA_RECORDING_ENABLD': VMA_RECORDING_ENABLD,
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