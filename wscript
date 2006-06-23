#! /usr/bin/env python
# encoding: utf-8

import Params

# the following two variables are used by the target "waf dist"
VERSION='0.1.0'
APPNAME='orocos-components'

# the srcdir is mandatory ('/' are converted automatically)
srcdir = '.'
blddir = '_build_'

def build(bld):
	# process subfolders from here
	bld.add_subdirs('reporting taskbrowser hardware/kuka hardware/lias motion_control/naxes motion_control/cartesian kinematics hardware/camera') 

def configure(conf):
	conf.checkTool(['g++'])
	#conf.sub_config('reporting')

	# Read the options.
	oroloc = Params.g_options.oropath
	orotgt = Params.g_options.orotarget
	opencv = Params.g_options.opencvpath

	if not conf.checkHeader('corelib/CoreLib.hpp', pathlst=[ oroloc+'/include' ]):
		Params.fatal("Orocos headers not found: aborting.")
		exit
	if not conf.checkHeader('os/'+orotgt+'.h', pathlst=[ oroloc+'/include' ]):
		Params.fatal("Headers for target "+orotgt+" not found: aborting.")

	if not conf.checkPkg('orocos-'+orotgt,'OROCOS', '0.23.0', oroloc+'/lib/pkgconfig'):
		Params.fatal("Orocos pkgconf file not found !")
	conf.env['LIB_OROCOS'].append( 'orocos-'+orotgt )
		
	if conf.checkPkg('opencv','OPENCV','',opencv+'/lib/pkgconfig'):
		conf.checkPkg('gtk+-2.0','GTK')
		conf.checkPkg('gthread-2.0','GTHREAD')
		conf.env['LIB_OPENCV'].append( 'dc1394_control')
		conf.env['LIB_OPENCV'].append( 'raw1394')
		conf.env['LIB_OPENCV'].append( 'jpeg')
	else:
		Params.fatal("Opencv pkgconf file not found ! hardware/Camera will not be available")
		
def set_options(opt):
	opt.add_option('--prefix', type='string', help='installation prefix', dest='prefix')
	opt.add_option('--with-orocos', type='string', help='Orocos installation directory', dest='oropath', default='/usr/local/orocos')
	opt.add_option('--target', type='string', help='Build target (lxrt, xenomai, gnulinux)', dest='orotarget', default='gnulinux')
	opt.add_option('--with-opencv',type='string',help='Opencv installation directory',dest='opencvpath', default='/usr/local/')
	#opt.sub_options('src')
