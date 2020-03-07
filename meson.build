project (
  'libsndfile-test', ['cpp'],
  version: '0.1',
  license: 'ISC',
  default_options: [
    'warning_level=1',
    ],
  )

cc = meson.get_compiler ('cpp')

# detect OS
os_darwin = false
os_linux = false
os_freebsd = false
os_windows = false
if host_machine.system() == 'darwin'
  os_darwin = true
elif host_machine.system() == 'linux'
  os_linux = true
elif host_machine.system() == 'freebsd'
  os_freebsd = true
elif host_machine.system() == 'windows'
  os_windows = true
endif

cargs = []
deps = [
  dependency('jack'),
  dependency('sndfile'),
  ]
inc_dirs = [
  include_directories (join_paths ('src', 'include')),
  ]

sndfile_test= executable (
  'sndfile_test',
  sources: [
    join_paths ('src', 'audio.cpp'),
    join_paths ('src', 'main.cpp'),
    ],
  cpp_args: cargs,
  dependencies: deps,
  include_directories: inc_dirs,
)
