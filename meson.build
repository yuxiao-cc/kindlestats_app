project('kindlestats_app', 'cpp',
  version: '1.0.0',
  default_options: ['cpp_std=c++17'],
  meson_version: '>=0.56'
)

gtk_dep = dependency('gtk+-2.0')

inc = include_directories('include')

executable('kindlestats_app',
  'main.cpp',
  'shared.cpp',
  'draw_callbacks.cpp',
  'month_dialog.cpp',
  'dashboard_page.cpp',
  'books_page.cpp',
  'today_page.cpp',
  include_directories: inc,
  dependencies: [gtk_dep],
  cpp_args: '-static-libstdc++',
  link_args: ['-static-libstdc++', '-pthread'],
)
