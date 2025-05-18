project('po2csv', 'c',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    'c_std=c99',
    'buildtype=release'
  ]
)

# Define directories
inc_dir = include_directories('include')

# Main executable
sources = [
  'src/main.c',
]

po2csv = executable('po2csv',
  sources,
  include_directories : inc_dir,
  install : true
)

# Install headers
install_headers('include/po2csv.h', subdir : 'po2csv')