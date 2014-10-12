#! /bin/sh

name=`date '+%Y%m%d-%H%M'`

echo ${name}

cp *.sh -u ./backup
cp *.c -u ./backup
cp *.h -u ./backup
cp Makefile -u ./backup

tar -cvzf ~/backup/backup_${name}_[simulation].tar.gz backup

git add -A
git commit -a -m "message"
#git push https://github.com/satoshi-1115/CPUex.git

