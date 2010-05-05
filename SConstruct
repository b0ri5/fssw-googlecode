"""
  Copyright 2010 Greg Tener
  Released under the Lesser General Public License v3.
"""

import os
import checks

env = Environment(tools = ['default', 'packaging', 'boris_scons_tools'])

# begin checks
conf = Configure(env, checks.all_checks)

# check for gtest
has_gtest = conf.CheckLibWithHeader('gtest', 'gtest/gtest.h', 'C++')

# only check for cmake if needed
if not has_gtest:
  print '  gtest not found, checking for cmake to build it'
  has_cmake = conf.CheckExecutable('cmake')
  if not has_cmake:
    print '  cmake not found, download it at http://www.cmake.org/'
conf.Finish()

# set up for using multiple configurations
configs = ARGUMENTS.get('config', 'debug')
config_libsuffixes = {'debug' : '-db', 'release' : ''}

for config in configs.split(','):
  config_env = checks.config(env, config)
  libsuffix = config_libsuffixes[config]
  
  # export the environment and libsuffix to the SConscripts
  Export({'env': config_env, 'libsuffix': libsuffix})
  
  # build the fssw library
  config_env.SConscript('src/SConscript',
          build_dir='build/debug/src', duplicate=0)

  conf = Configure(config_env, checks.all_checks)
  config_env['CONFIGURATION'] = config
  
  config_env['GTEST_LIB'] = ''
  config_env['GTEST_INCLUDE'] = ''
  libgtest = None

  if not has_gtest and has_cmake:
    # build the copy of gtest in third-party
    
    config_env['GTEST_INCLUDE'] = '#/third-party/gtest-1.5.0/include'
    # the SConscript will set GTEST_LIB appropriately
    config_env.SConscript('third-party/SConscript')
    Import('libgtest')
    has_gtest = True
 
  if has_gtest:
    Export('libgtest')
    config_env.SConscript('test/SConscript',
          build_dir='build/debug/test', duplicate=0)

