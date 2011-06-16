#
# Building a typekit (recommended):
#
# Creates a typekit library lib@pkgname@-types-<target>.so
# and installs in the directory lib/orocos/@pkgname@/types/
#
orocos_typegen_headers(@pkgname@-types.hpp) # ...you may add multiple header files
#
# You may only have *ONE* orocos_typegen_headers statement !


