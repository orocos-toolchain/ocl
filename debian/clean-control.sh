#!/bin/bash

targets="lxrt gnulinux xenomai"

rm control

for i in $targets; do rm liborocos*$i*install orocos*$i*install; done
