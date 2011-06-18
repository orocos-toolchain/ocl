#
# Building a Service:
#
# Creates a plugin library lib@pkgname@-service-<target>.so
# and installs in the directory lib/orocos/@pkgname@/plugins/
#
orocos_service(@pkgname@-service @pkgname@-service.cpp) # ...only one service per library !
#
# You may add multiple orocos_service statements.


