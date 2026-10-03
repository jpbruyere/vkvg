test_src = [
    'arcs.c',
    'bezier.c',
    'circles.c',
    'clip.c',
    'colinear.c',
    'compositing.c',
    'context.c',
    'curve.c',
    'dashes.c',
    'fill_and_stroke.c',
    'fill.c',
    'fill_non_zero.c',
    'gradient2.c',
    'gradient.c',
    'img_surf.c',
    'inverse_colinear.c',
    'line_caps.c',
    'line_join.c',
    'lines.c',
    'offscreen.c',
    'paint_surf.c',
    'path_extents.c',
    'pattern_transforms.c',
    'radial_gradient.c',
    'recording.c',
    'rect_fill.c',
    'save_restore.c',
    'simple_paint.c',
    'stroke.c',
    'surface.c',
    'svg.c',
    'test1.c',
    'text.c',
    'transform.c',
]

deps = [
    vkvg_dep,
    dependency('vkh', fallback: ['vkh', 'vkh_dep']),
    dependency('glfw', fallback: [ 'glfw','glfw_dep',])
]

test_include = include_directories(['common', ''])
common_src = files(
    'common/nanosvg.h',
    'common/rnd.c',
    'common/test.c',
    'common/tinycthread.c',
    'common/vkengine.c',
)

test_common = static_library(
    'test_common',
    common_src,
    dependencies: deps,
    include_directories: test_include,
)

c_args = ['-DTESTS_DATA_ROOT="@0@/data"'.format(meson.current_source_dir())]

foreach src : test_src
    test_name = src.split('.')[0]
    t = executable(
        test_name,
        src,
        dependencies: deps,
        link_with: test_common,
        include_directories: test_include,
        c_args: c_args,
    )

    test(test_name, t)
endforeach