env = Environment()
env['BUILD_DIR'] = 'build'
debug = ARGUMENTS.get('debug', 0)
if debug:
    env.Append(CPPDEFINES={'_DEBUG': 1})
    env.Append(CFLAGS=['-g'])
log_filename = ARGUMENTS.get('logfile', '')
if log_filename:
    env.Append(CPPDEFINES={'_DEBUG_FILENAME': log_filename})
SConscript('src/SConscript', variant_dir=env['BUILD_DIR'], duplicate=False, exports='env')
