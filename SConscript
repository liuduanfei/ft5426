from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add ft6206 src files.
if GetDepend('PKG_USING_FT5426'):
    src += Glob('src/ft5426.c')

if GetDepend('PKG_USING_FT5426_SAMPLE'):
    src += Glob('examples/example_ft5426.c')

# add ft5426 include path.
path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('ft5426', src, depend = ['PKG_USING_FT5426'], CPPPATH = path)

Return('group')