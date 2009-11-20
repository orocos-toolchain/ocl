#!/bin/bash

major=$(head -1 changelog | sed "s/.*(\([0-9]\+\.[0-9]\+\).*/\1/g")

echo "Detected OCL Major version: $major"

rm -f control
rm -f orocos*$major*install liborocos*$major*install
rm -f rules.targets control.targets

echo "Done!"
