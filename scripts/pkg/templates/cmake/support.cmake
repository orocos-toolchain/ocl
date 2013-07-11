#
# Building a normal library (optional):
#
# Creates a library lib@pkgname@-support-<target>.so and installs it in
# lib/
#
orocos_library(@pkgname@-support support.cpp) # ...you may add multiple source files
#
# You may add multiple orocos_library statements.


