parent_env = Environment(CPPPATH='.', LIBPATH='.')

params = Variables(['custom.py'])
params.Add(PathVariable('prefix', 'System path', '/usr', PathVariable.PathAccept))
params.Add(PathVariable('libdir', 'System libraries', '${prefix}/lib', PathVariable.PathAccept))
params.Add(PathVariable('includedir', 'System includes', '${prefix}/include', PathVariable.PathAccept))
params.Add(PathVariable('datadir', 'System shares', '${prefix}/share', PathVariable.PathAccept))
params.Add(PathVariable('sysconfdir', 'System configuration', '/etc', PathVariable.PathAccept))
params.Add(PathVariable('execdir', 'System binaries', '${prefix}/bin', PathVariable.PathAccept))
params.Add(PathVariable('sysvardir', 'System var', '/var', PathVariable.PathAccept))
params.Add(PathVariable('openxrsharedir', 'OpenXR share', '${datadir}/openxr', PathVariable.PathAccept))
params.Update(parent_env)

SConscript(dirs='src', variant_dir='build', duplicate=0, exports='parent_env')
