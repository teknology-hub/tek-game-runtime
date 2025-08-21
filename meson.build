project(
  'tek-game-runtime',
  'cpp',
  version: files('meson.version'), meson_version: '>=1.4.0',
  license: 'GPL-3.0-or-later', license_files: 'COPYING',
  default_options: {
    'cpp_std': 'gnu++23',
    'warning_level': '3'
  }
)
add_project_arguments('-gcodeview', '-DUNICODE', '-D_UNICODE', language: 'cpp')
add_project_link_arguments('-municode', '-static', language: 'cpp')
# Make file version for the .rc file
rc_version = meson.project_version().replace('.', ',')
if rc_version.contains('-')
  rc_version = rc_version.split('-')[0]
endif
num_elems = rc_version.split(',').length()
if num_elems == 2
  rc_version += ',0,0'
endif
if num_elems == 3
  rc_version += ',0'
endif
compiler = meson.get_compiler('cpp')
# -Wno-attributes: Project code uses many attributes that are supported only by
#    specific compilers or even specific compiler versions
# -Wno-nullability-extension: Fallbacks for these attributes are provided by
#    common.hpp anyway
add_project_arguments(
  compiler.get_supported_arguments(
    '-Wno-attributes', '-Wno-nullability-extension'),
  language: 'cpp'
)
src = [
  'src/main.cpp',
  'src/settings.cpp',
  'src/steam_api.cpp',
  'src/tek-steamclient.cpp'
]
subdir('src/steam')
src += import('windows').compile_resources(
  configure_file(
    input: 'res/tek-game-runtime.rc.in',
    output: 'tek-game-runtime.rc',
    configuration: {
      'file_version': rc_version,
      'pretty_version': meson.project_version()
    }
  )
)
compiler.has_header('tek-steamclient/am.h', required: true)
compiler.has_header('tek-steamclient/base.h', required: true)
compiler.has_header('tek-steamclient/cm.h', required: true)
compiler.has_header('tek-steamclient/error.h', required: true)
compiler.has_header('tek-steamclient/os.h', required: true)
shared_library(
  'tek-game-runtime',
  src,
  link_args: '-Wl,--pdb=libtek-game-runtime.pdb',
  dependencies: [
    dependency('RapidJSON'),
    compiler.find_library('dbghelp'),
    compiler.find_library('synchronization'),
    compiler.find_library('version'),
    subproject('ValveFileVDF').get_variable('valve_file_vdf_dep')
  ],
  include_directories: 'src',
  gnu_symbol_visibility: 'hidden',
  install: true
)
