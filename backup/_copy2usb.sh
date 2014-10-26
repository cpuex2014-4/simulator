#! /bin/sh

name=`date '+%Y%m%d-%H%M'`

echo ${name}

cp *.sh -u ./backup
cp *.c -u ./backup
# cp *.h -u ./backup
cp Makefile -u ./backup

tar -cvzf /media/USB_004H_SiliconPower/Simulator/backup/backup_${name}_[simulation].tar.gz ./backup

cp -u *.* /media/USB_004H_SiliconPower/Simulator
