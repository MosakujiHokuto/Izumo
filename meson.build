project('izumo', 'cpp',
        version: '0.0.1',
        license: 'Apache',
       default_options: ['cpp_std=c++20'])

inc = include_directories('include')

srcs = []

subdir('src')

executable('izumo', srcs, include_directories : inc)
