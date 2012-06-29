#!/bin/bash

echo "This script builds docs on Peter's Orocos tree
1a. expects a directory build/orocos-ocl-VERSION and build/orocos-ocl-VERSION/build
1b. OR if version is 'latest' , do fresh check out from trunk 
3. make docs in build
4. tar the result
5. copy over all the doc-tar files
6. extract and move files on server
"
set -ex

if [ x$1 == x ] ; then 
echo "Please provide version-string parameter"
exit 1
fi;
if [ x$2 != xmaster ] ; then 
DEV=no
else
DEV=yes
fi;

DOLOCAL=yes

. release-config

if test $DOLOCAL = yes; then
  USER=$USER
  SERVER=localhost
  SPREFIX=src/export/upload
else
  if test x$DOOROCOSORG = xyes -a x$DEV = xno; then
    USER=bruyninckxh2
    SERVER=www.orocos.org
    SPREFIX=www.orocos.org
  else
    USER=psoetens
    SERVER=ftp.mech.kuleuven.be
    SPREFIX=/www/orocos/pub
    APREFIX=/www/orocos
  fi
fi

if test x$DEV = xyes; then
BRANCH=devel
VERSION=master
VVERSION=master
else
BRANCH=stable
VERSION=$1
VVERSION=v$1
fi

# i.e. 1.2.1 -> 1.2
BRANCHVERSION=$(echo $VERSION | sed -e 's/\(.*\)\.\(.*\)\..*/\1.\2/g')

topdir=$(pwd)

if test x$DOAUTO != xyes; then
    echo "VERSION is set to $VERSION (use 'master' to install trunk on server)"
    echo "DEV is set to $DEV (use 'master' as arg2 to install in 'devel' on server)"
    echo "Press c to continue, any other key to upload files to server and Ctrl-C to abort..."
    read -s -n1 x
else
    x=c
fi

if [ x$x == xc ] ;  then

#if master, check out trunk
mkdir -p build; cd build
if test x$VERSION = xmaster -o x$DOCHECKOUT = xyes; then
  rm -rf orocos-toolchain-$VERSION/ocl
  cd $topdir/orocos-toolchain-ocl
  git archive --format=tar --prefix=orocos-toolchain-$VERSION/ocl/ HEAD | (cd $topdir/build && tar xf -)
  cd $topdir/build
  cd orocos-toolchain-$VERSION/ocl
  mkdir build
  cd build
  cmake .. -DDOC_ONLY=TRUE
fi
cd $topdir/build

#all should be equal for MASTER and normal :
if  ! test -d orocos-toolchain-$VERSION/ocl ; then
    echo "Could not find orocos-toolchain-$VERSION/ocl !"
    exit 1
fi
cd orocos-toolchain-$VERSION/ocl

# Doxygen
mkdir -p build
cd build
  cmake ..  -DDOC_ONLY=TRUE
  make docapi
  cd doc
  tar -cjf orocos-ocl-$VERSION-api.tar.bz2 api
  mv orocos-ocl-$VERSION-api.tar.bz2 api ..
  #make install-docs # install ocl.tag file !
  cd ..
cd ..

# generated Lua API docs
which luadoc
if [ $? -ne 0 ]; then
    echo "No luadoc found, can't build and upload Lua API docs!"
    exit 1
else
    pushd build &> /dev/null
    make luadocapi
    cd doc
    tar cjvf orocos-ocl-$VERSION-luaapi.tar.bz2 luaapi
    mv orocos-ocl-$VERSION-luaapi.tar.bz2 ..
    popd &> /dev/null
fi

# Build base package
cd build
  cd doc
  make dochtml -j12
  make docpdf -j12
  
  cp -a xml doc-xml
  rm -rf doc-xml/images/hires # not for distribution
  tar -cjf orocos-ocl-$VERSION-doc.tar.bz2 $(find doc-xml -name "*.png" -o -name "*.pdf" -o -name "*.html" -o -name "*css")  ||exit 1
  rm -rf doc-xml
  mv orocos-ocl-$VERSION-doc.tar.bz2 ..
  cd ..
cd ..

if test x$DOAUTO != xyes; then
    echo "Press a key to copy Docs to server, Ctrl-C to abort..."
    read -s -n1
fi

else
 cd $topdir/build/orocos-toolchain-$VERSION/ocl
fi; # press d

while [ 1 ]; do

echo -e "\n**** COPYING TO $SERVER: ****\n"

# Docs :
# Save in version subdir as tar, save master in doc dir. (saves space).
cd build
# Copy over tar.bz2 files
ssh $USER@$SERVER "ls $APREFIX; mkdir -p $SPREFIX/$BRANCH/ocl/$VVERSION"
scp orocos-ocl-$VERSION-doc.tar.bz2 $USER@$SERVER:$SPREFIX/$BRANCH/ocl/$VVERSION
scp orocos-ocl-$VERSION-api.tar.bz2 $USER@$SERVER:$SPREFIX/$BRANCH/ocl/$VVERSION
scp orocos-ocl-$VERSION-luaapi.tar.bz2 $USER@$SERVER:$SPREFIX/$BRANCH/ocl/$VVERSION
# Install them in the 'documentation' dir:
# 'doc' is not physically existing, it is the drupal path to imported docs
# 'doc-xml' are the xml generated html/pdf files
# 'api' is the doxygen generated sources
if test x$DEV = xno; then
ssh $USER@$SERVER "mkdir -p $SPREFIX/$BRANCH/documentation/ocl/v$BRANCHVERSION.x/"
ssh $USER@$SERVER "cd $SPREFIX/$BRANCH/documentation/ocl/v$BRANCHVERSION.x/ &&
rm -rf doc api doc-xml &&
tar -xjf ../../../ocl/$VVERSION/orocos-ocl-$VERSION-doc.tar.bz2 && 
tar -xjf ../../../ocl/$VVERSION/orocos-ocl-$VERSION-api.tar.bz2 &&
tar -xjf ../../../ocl/$VVERSION/orocos-ocl-$VERSION-luaapi.tar.bz2 &&
rm -f ../../../ocl/$VVERSION/orocos-ocl-$VERSION-api.tar.bz2 \
      ../../../ocl/$VVERSION/orocos-ocl-$VERSION-luaapi.tar.bz2 \
      ../../../ocl/$VVERSION/orocos-ocl-$VERSION-doc.tar.bz2 &&
cd .. && { linkv=\$(ls -l v2.x | sed -e\"s/l.*-> v//;s/\.//g;s/x//\"); branchv=\$(echo $BRANCHVERSION | sed -e\"s/\.//g\"); if test 0\$branchv -gt 0\$linkv; then
rm -f v2.x && ln -s v$BRANCHVERSION.x v2.x ;
fi;
}
"
else
ssh $USER@$SERVER "mkdir -p $SPREFIX/$BRANCH/documentation/ocl/$VVERSION"
ssh $USER@$SERVER "cd $SPREFIX/$BRANCH/documentation/ocl/$VVERSION &&
rm -rf doc api doc-xml &&
tar -xjf ../../../ocl/$VVERSION/orocos-ocl-$VERSION-doc.tar.bz2 && 
tar -xjf ../../../ocl/$VVERSION/orocos-ocl-$VERSION-api.tar.bz2 &&
tar -xjf ../../../ocl/$VVERSION/orocos-ocl-$VERSION-luaapi.tar.bz2 &&
rm -f ../../../ocl/$VVERSION/orocos-ocl-$VERSION-api.tar.bz2 ../../../ocl/$VVERSION/orocos-ocl-$VERSION-doc.tar.bz2
"
fi
cd ..

# copy latest news to packages directory :
scp NEWS $USER@$SERVER:$SPREFIX/$BRANCH/ocl/NEWS.txt
scp README $USER@$SERVER:$SPREFIX/$BRANCH/ocl/README.txt

  if test x$DOOROCOSORG = xno -o x$DOLOCAL = xyes -o x$DEV = xyes; then
      echo "Completed succesfully."
      exit 0;
  fi
  # redo for making a copy on the mech server as well:
  USER=psoetens
  SERVER=ftp.mech.kuleuven.be
  SPREFIX=/www/orocos/pub
  APREFIX=/www/orocos
  DOOROCOSORG=no

done; # while [ 1 ]

