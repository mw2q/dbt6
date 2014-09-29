#!/bin/sh

DIR=`dirname $0`
. ${DIR}/dbt3_profile || exit 1

SRCDIR=${DBT3_HOME}

if [ $# -ne 1 ]; then
	echo "usage: gen_data.sh <scale factore>"
fi

SF=$1

dbgenssbm -fF -s $SF -T c &
dbgenssbm -fF -s $SF -T p &
dbgenssbm -fF -s $SF -T s &
dbgenssbm -fF -s $SF -T d &
dbgenssbm -fF -s $SF -T l &

wait
