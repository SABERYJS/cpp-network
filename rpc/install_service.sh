#/bin/bash

service_file=/lib/systemd/system/rpcserver.service;
project_dir=/data/cimserver;

reload=0;

if [ -f $service_file ]
then
    reload=1;
fi  

cp -f $project_dir/rpc/rpcserver.service /lib/systemd/system/;

if [ $reload -eq 1 ]
then
    #systemctl daemon-reload;
    if [ -f "/proc/sys/fs/binfmt_misc/WSLInterop" ]
    then
        echo "wsl environment";
    else
        systemctl daemon-reload;
    fi
fi

service enable rpcserver;
service rpcserver restart;