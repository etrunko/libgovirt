project('libgovirt', 'c', version: '0.3.4', meson_version: '>= 0.35.0')
deps = dependency('glib-2.0 gio-2.0 rest-0.7')

govirt_version = meson.project_version()
version_arr = govirt_version.split('.')
govirt_version_major = version_arr[0]
govirt_version_minor = version_arr[1]
govirt_version_micro = version_arr[2]

apiversion = '1.0'
soversion = 2
# maintaining compatibility with the previous libtool versioning
# current = minor * 100 + micro
libversion = '@0@.@1@.0'.format(soversion, govirt_version_minor.to_int() * 100 + govirt_version_micro.to_int())



### config.h

cdata = configuration_data()
cdata.set('GETTEXT_PACKAGE', '"libgovirt"')
cdata.set('PACKAGE_STRING', '"libgovirt @0@"'.format(govirt_version))

configure_file(output : 'config.h', configuration : cdata)



### gobject-introspection

gir = find_program('g-ir-scanner', required : false)

# Fixme, not very elegant.
build_gir = gir.found() and not meson.is_cross_build()


###

subdir('govirt')
subdir('po')


### examples

executable('examples/list-vms', 'examples/list-vms.c', dependencies: govirtlib_dep, link_with: govirtlib)
executable('examples/list-vms-async', 'examples/list-vms-async.c', dependencies: govirtlib_dep, link_with: govirtlib)
