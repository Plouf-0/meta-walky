project('walky-hmi-controller',
  'c',
  version: '1.0',
  default_options: ['warning_level=2', 'c_std=gnu99'],
)

libweston_dep = dependency('libweston-13')
weston_dep    = dependency('weston')

weston_moduledir = join_paths(get_option('libdir'), 'weston')

shared_library('walky-hmi-controller',
  'walky-hmi-controller.c',
  dependencies: [libweston_dep, weston_dep],
  name_prefix: '',
  install: true,
  install_dir: weston_moduledir,
)