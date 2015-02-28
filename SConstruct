env = Environment()
env['BUILD_DIR'] = 'build'
debug = ARGUMENTS.get('debug', 0)
if debug:
    env.Append(CPPDEFINES={'_DEBUG': 1})
SConscript('src/SConscript', variant_dir=env['BUILD_DIR'], duplicate=False, exports='env')
