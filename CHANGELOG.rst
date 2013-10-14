^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package ocl
^^^^^^^^^^^^^^^^^^^^^^^^^

2.7.0 (2013-10-14)
------------------
* catkin: added run_depend on catkin to package.xml for released packages
  Signed-off-by: Johannes Meyer <johannes@intermodalics.eu>
* Merge branch 'master' of https://github.com/jhu-lcsr-forks/ocl
* Merge pull request #3 from jhu-lcsr-forks/add-orocos-cmake-config
  Adding CMake config file for OCL
* remvoing variable from old RTT config
* removing REQUIRED from ocl cmake config file components
* fixing doc
* Adding CMake config file for OCL
  This cmake config file allows people to treat OCL like a single package
  with subcomponents, which is how it is really set up. This works in a
  similar manner to the way the RTT cmake config file works.
* compatibility with rosdep just isn't worth this
* adding package.xml files corresponding to orocos ocl subcomponents
* Merge branch 'master' of https://github.com/smits/ocl
* orocreate-pkg: support both devel as installed versions of ocl
  Signed-off-by: Ruben Smits <ruben@intermodalics.eu>
* orocreate-pkg: add orogen dependency to generated manifest
  Signed-off-by: Ruben Smits <ruben.smits@intermodalics.eu>
* orocreate-pkg: provide package.xml file
  Signed-off-by: Ruben Smits <ruben.smits@intermodalics.eu>
* fixing deps
* Merge remote-tracking branch 'origin/master'
* Merge branch 'toolchain-2.6'
* orocreate-pkg: add pkgname prefix for support library
  Signed-off-by: Ruben Smits <ruben.smits@intermodalics.eu>
* Merge branch 'master' of gitorious.org:orocos-toolchain/ocl
* install orocreate-pkg templates in correct package folder
  Signed-off-by: Ruben Smits <ruben.smits@intermodalics.eu>
* install orocreate-pkg templates in correct package folder
  Signed-off-by: Ruben Smits <ruben.smits@intermodalics.eu>
* orocreate-pkg: add .tpl extension to template, confused buildfarm
  The manifest.xml file was discovered and caused the ROS build farm
  to build our template package...
  Signed-off-by: Peter Soetens <peter@thesourceworks.com>
* install package.xml file in install_prefix/share/package path
  Signed-off-by: Ruben Smits <ruben.smits@intermodalics.eu>
