project('vaktin', 'cpp',
  default_options : ['cpp_std=c++20'],
  version : '0.0.1'
  )

netpp_subproject = subproject('netpp')
netpp_dep = netpp_subproject.get_variable('netpp_library_dep')

subdir('src/')

includes = include_directories('include')
executable(
  'vping',
  ['src/ping.cpp', src_files],
  include_directories: includes,
  dependencies: netpp_dep,
  install: true
)
