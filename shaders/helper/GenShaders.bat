$GLSLC_BIN $GLSLDEFS %1 -o "shader_comp.spv"

$GLSLC_BIN $GLSLDEFS %2 -o "shader2_comp.spv"

$GLSLC_BIN $GLSLDEFS %3 -o "vkvg_main_lcd_frag.spv"

$GLSLC_BIN $GLSLDEFS %4 -o "vkvg_main_frag.spv"

$GLSLC_BIN $GLSLDEFS %5 -o "vkvg_main_vert.spv"

$GLSLC_BIN $GLSLDEFS %6 -o "wired_frag.spv"

$XXD_BIN -i "shader_comp.spv" > "shaders.h"
$XXD_BIN -i "shader2_comp.spv" >> "shaders.h"
$XXD_BIN -i "vkvg_main_lcd_frag.spv" >> "shaders.h"
$XXD_BIN -i "vkvg_main_frag.spv" >> "shaders.h"
$XXD_BIN -i "vkvg_main_vert.spv" >> "shaders.h"
$XXD_BIN -i "wired_frag.spv" >> "shaders.h"