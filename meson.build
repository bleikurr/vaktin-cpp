project('vaktin', 'cpp',
  default_options : ['cpp_std=c++20'],
  version : '0.0.1'
  )

subdir('src/')

executable(
  'vping',
  ['src/ping.cpp', src_files],
  include_directories: src_includes,
  install: true
)
