#!/bin/bash

echo "This scripts creates the debian/control and debian/*.install files from several template files."

major=$(head -1 changelog | sed "s/.*(\([0-9]\+\.[0-9]\+\).*/\1/g")

echo "Detected RTT Major version: $major"

targets="gnulinux lxrt xenomai"

# Prepare control file:
rm -f control

echo "Creating control ..."
cat control.common | sed -e"s/@TARGET@/$t/g;s/@TARGET-DEV@/$tdev/g;s/@LIBVER@/$major/g" > control
for t in $targets; do 
    # append control-template.in to control file
    cat control-template.in | sed -e"s/@TARGET@/$t/g;s/@LIBVER@/$major/g" >> control
    cat control-$t.in >> control
done

# Prepare *.install files:

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
    # Replace contents and write to *-target.install files:
    fname=$(echo "$i" | sed -e"s/-sover/$major/g")
    echo "Creating $fname ..."
    cat $i > $fname
done
