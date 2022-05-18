#!/bin/bash
rm -rf fakeroot/*
mkdir -p fakeroot/etc/bvpm/packages
echo "PACKAGECOUNT=0" > fakeroot/etc/bvpm/packages/_count
echo "acceptAll=yes" > fakeroot/etc/bvpm/bvpm.cfg
