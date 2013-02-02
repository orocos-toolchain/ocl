#
# Building a typekit using typegen (recommended):
#
# Creates a typekit library lib@pkgname@-types-<target>.so
# and installs in the directory lib/orocos/@target@/@pkgname@/types/
#
# The header will go in include/orocos/@pkgname@/types/@pkgname@/@pkgname@-types.hpp
# So you can #include <@pkgname@/@pkgname@-types.hpp>
#
orocos_typegen_headers(include/@pkgname@/@pkgname@-types.hpp) # ...you may add multiple header files
#
# You may only have *ONE* orocos_typegen_headers statement in your toplevel CMakeFile.txt !


