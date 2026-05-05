project('vaktin', 'cpp',
  default_options : ['cpp_std=c++20'],
  version : '0.0.1'
  )

conf_data = configuration_data()
conf_data.set('VERSION', meson.project_version())
configure_file(
  input: 'version.hpp.in',
  output: 'version.hpp',
  configuration: conf_data
)

netpp_subproject = subproject('netpp')
netpp_dep = netpp_subproject.get_variable('netpp_library_dep')

subdir('src/')
includes = include_directories('include')
executable(
  'vping',
  ['src/vping.cpp', src_files],
  include_directories: includes,
  dependencies: netpp_dep,
  install: true
)
