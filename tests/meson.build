add_global_arguments('-Dabs_srcdir="'+meson.source_root()+'"', language : 'c')

### tests
executable('test-govirt',
           [ 'test-govirt.c', 'mock-httpd.c', 'mock-httpd.h' ],
           dependencies: govirtlib_dep,
           link_with: govirtlib)
