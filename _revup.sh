#! /bin/sh


name=`date '+%Y%m%d-%H%M'`

echo ${name}



cp *.c ./backup
cp Makefile ./backup

tar -cvzf ~/backup/backup_${name}_[simulation].tar.gz ./backup
