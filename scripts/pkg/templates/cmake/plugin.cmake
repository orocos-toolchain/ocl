#
# Building a Plugin
#
# Creates a plugin library lib@pkgname@-plugin-<target>.so
# and installs in the directory lib/orocos/@pkgname@/plugins/
#
# Be aware that a plugin may only have the loadRTTPlugin() function once defined in a .cpp file.
# This function is defined by the plugin and service CPP macros.
#
orocos_plugin(@pkgname@-plugin @pkgname@-plugin.cpp) # ...only one plugin function per library !
#
# You may add multiple orocos_plugin statements.


