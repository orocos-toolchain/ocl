# Creates a component library lib@pkgname@-<target>.so
# and installs in the directory lib/orocos/@pkgname@/
#
orocos_component(@pkgname@ @pkgname@-component.hpp @pkgname@-component.cpp) # ...you may add multiple source files
#
# You may add multiple orocos_component statements.

#
# Additional headers:
#
# Installs in the include/orocos/@pkgname@/ directory
#
orocos_install_headers(@pkgname@-component.hpp) # ...you may add multiple header files
#
# You may add multiple orocos_install_headers statements.

