#!/bin/sh
BUNDLE_DIR=perl6_bundle

if [ $# -ne 1 ]; then
	echo "ERROR - Must supply a path to a perl6 installation"
	exit 1
fi

perl6_install_dir=$1
if [ ! -d $perl6_install_dir ]; then
	echo "ERROR - '$perl6_install_dir' does not exist or is not a directory"
	exit 1
fi

# wipe out previous bundle, if it exists
rm -rf $BUNDLE_DIR
mkdir -p $BUNDLE_DIR/bin
mkdir -p $BUNDLE_DIR/lib
mkdir -p $BUNDLE_DIR/share/nqp
mkdir -p $BUNDLE_DIR/share/perl6

cp $perl6_install_dir/bin/perl6-m $BUNDLE_DIR/bin/perl6-m
cp $perl6_install_dir/lib/libmoar.so $BUNDLE_DIR/lib/libmoar.so
cp -r $perl6_install_dir/share/nqp/lib $BUNDLE_DIR/share/nqp/lib
cp -r $perl6_install_dir/share/perl6/lib $BUNDLE_DIR/share/perl6/lib
cp -r $perl6_install_dir/share/perl6/site $BUNDLE_DIR/share/perl6/site
cp -r $perl6_install_dir/share/perl6/core $BUNDLE_DIR/share/perl6/core
cp -r $perl6_install_dir/share/perl6/vendor $BUNDLE_DIR/share/perl6/vendor
cp -r $perl6_install_dir/share/perl6/runtime $BUNDLE_DIR/share/perl6/runtime
