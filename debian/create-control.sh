#!/bin/bash

echo "This scripts creates the debian/control and debian/*.install files from several template files."

major=$(head -1 changelog | sed "s/.*(\([0-9]\+\.[0-9]\+\).*/\1/g")

echo "Detected OCL Major version: $major"

# default to gnulinux if no targets are given
if test x$1 = x; then
    targets="gnulinux"
else
    targets="$@"
fi

echo "Building scripts for targets: $targets"

# Prepare control file:
rm -f control control.targets

echo "Creating control ..."
bindeps=""
builddeps=""
for t in $targets; do 
	bd="orocos-ocl-$t$major-bin (= \${binary:Version})"
	if test x$bindeps = x; then bindeps="$bd"; else bindeps="$bindeps, $bd"; fi
	builddeps=", liborocos-rtt-corba-$t$major-dev ( >= $major ) $builddeps"
    if test $t = xenomai; then tdev=", xenomai-dev | libxenomai-dev"; builddeps="$tdev $builddeps"; fi
    if test $t = lxrt; then tdev=", librtai-dev"; builddeps=", $tdev $builddeps"; fi
    cat control-template.in | sed -e"s/@TARGET@/$t/g;s/@TARGET-DEV@/$tdev/g;s/@LIBVER@/$major/g" >> control.targets
    cat control-$t.in >> control.targets
done
# Final control file = common + targets
cat control.common | sed -e"s/@LIBVER@/$major/g;s/@BUILD_DEPS@/$builddeps/g;s/@BIN_DEPS@/$bindeps/g" > control
cat control.targets >> control
rm control.targets

# Prepare target *.install files:
for i in $(ls *template*install); do
    for t in $targets; do
	    # Replace contents and write to *-target.install files:
		fname=$(echo "$i" | sed -e"s/template/$t$major/g;")
		echo "Creating $fname ..."
		cat $i | sed -e"s/@TARGET@/$t/g" > $fname
    done
done

# Prepare *sover-dev.install files:
for i in $(ls *sover-dev.install); do
    # Copy contents to *-target.install files:
    fname=$(echo "$i" | sed -e"s/-sover/$major/g")
    echo "Creating $fname ..."
    cat $i > $fname
done

echo "Creating rules files..."
# Prepare rules files:
rm -f rules.targets
for t in $targets; do
    CONFIGURE_TARGETS="$CONFIGURE_TARGETS configure-$t"
    BUILD_TARGETS="$BUILD_TARGETS build-$t"
    INSTALL_TARGETS="$INSTALL_TARGETS install-$t"
    INCLUDES=$(echo -e "$INCLUDES\ninclude debian/rules.$t")
done
cat >> rules.targets <<EOF
CONFIGURE_TARGETS=$CONFIGURE_TARGETS
BUILD_TARGETS=$BUILD_TARGETS
INSTALL_TARGETS=$INSTALL_TARGETS

$INCLUDES
EOF

echo "Done!"
