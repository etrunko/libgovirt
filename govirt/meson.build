add_global_arguments('-Wno-deprecated-declarations', language: 'c')

lib_sources = [
    'glib-compat.c',
    'ovirt-action-rest-call.c',
    'ovirt-api.c',
    'ovirt-cdrom.c',
    'ovirt-collection.c',
    'ovirt-error.c',
    'ovirt-options.c',
    'ovirt-proxy.c',
    'ovirt-proxy-deprecated.c',
    'ovirt-resource.c',
    'ovirt-rest-call.c',
    'ovirt-resource-rest-call.c',
    'ovirt-storage-domain.c',
    'ovirt-utils.c',
    'ovirt-vm.c',
    'ovirt-vm-display.c',
    'ovirt-vm-xml.c',
    'ovirt-vm-pool.c'
]

lib_headers = [
    'govirt.h',
    'ovirt-api.h',
    'ovirt-cdrom.h',
    'ovirt-collection.h',
    'ovirt-error.h',
    'ovirt-options.h',
    'ovirt-proxy.h',
    'ovirt-resource.h',
    'ovirt-rest-call-error.h',
    'ovirt-storage-domain.h',
    'ovirt-types.h',
    'ovirt-vm.h',
    'ovirt-vm-pool.h',
    'ovirt-vm-display.h'
]
install_headers(lib_headers, subdir : 'govirt-1.0/govirt')

noinst_lib_headers = [
    'glib-compat.h',
    'govirt-private.h',
    'ovirt-action-rest-call.h',
    'ovirt-api-private.h',
    'ovirt-collection-private.h',
    'ovirt-proxy-private.h',
    'ovirt-resource-private.h',
    'ovirt-rest-call.h',
    'ovirt-resource-rest-call.h',
    'ovirt-storage-domain-private.h',
    'ovirt-utils.h',
    'ovirt-vm-private.h'
]


## glib-mkenums

gnome = import('gnome')
ovirt_enums = gnome.mkenums('ovirt-enum-types',
                            sources : lib_headers,
                            c_template : 'ovirt-enum-types.c.template',
                            h_template : 'ovirt-enum-types.h.template')
ovirt_enums_private = gnome.mkenums('ovirt-enum-types-private',
                                    sources : noinst_lib_headers,
                                    c_template : 'ovirt-enum-types-private.c.template',
                                    h_template : 'ovirt-enum-types-private.h.template')


## symbol export

mapfile = 'govirt.sym'
vflag = '-Wl,--version-script,' + meson.current_source_dir() + '/' + mapfile
govirtlib = library(
  'govirt', lib_sources, ovirt_enums, ovirt_enums_private,
  soversion : soversion,
  version : libversion,
  dependencies : deps,
  include_directories : [ include_directories(meson.source_root()), include_directories(meson.build_root())],
  #link_args : ['-Wl,--no-undefined', vflag],
  link_depends : mapfile,
  install : true)

govirtlib_dep = declare_dependency(link_with : govirtlib,
  include_directories : [ include_directories(meson.source_root()) ],
  dependencies : deps,
  # Everything that uses libgovirt needs this built to compile
  sources : [ovirt_enums, ovirt_enums_private],
)


### pkg-config

pkgconf = configuration_data()

pkgconf.set('prefix', get_option('prefix'))
pkgconf.set('exec_prefix', '${prefix}')
pkgconf.set('libdir', '${prefix}/@0@'.format(get_option('libdir')))
pkgconf.set('includedir', '${prefix}/@0@'.format(get_option('includedir')))
pkgconf.set('VERSION', govirt_version)

pkg_install_dir = '@0@/pkgconfig'.format(get_option('libdir'))

pkg_files = ['govirt-1.0' ]

configure_file(input : 'govirt-1.0.pc.in',
               output : 'govirt-1.0.pc',
               configuration : pkgconf,
               install_dir : pkg_install_dir)



### gobject-introspection

if build_gir
  govirt_gir_extra_args = [ '--c-include=govirt/govirt.h' ]
  if meson.is_subproject()
    # FIXME: There must be a better way to do this
    # Need to pass the include path to find gst/gst.h and gst/gstenumtypes.h (built)
    govirt_gir_extra_args += ['--cflags-begin',
                              '-I' + meson.current_source_dir() + '/..',
                              '-I' + meson.current_build_dir() + '/..',
                              '--cflags-end']
  endif
  gnome.generate_gir(govirtlib,
    sources : lib_sources + lib_headers,
    namespace : 'GoVirt',
    nsversion : apiversion,
    identifier_prefix : 'Ovirt',
    symbol_prefix : 'ovirt_',
    export_packages : 'govirt-1.0',
    includes : ['Gio-2.0', 'GObject-2.0', 'Rest-0.7' ],
    install : true,
    extra_args : govirt_gir_extra_args,
  )
endif
