#! /bin/bash

input=$1

if [ -z $input ]; then
    echo "No input file specified."
    exit 1
fi

cd scripts

prefix=""

if [ ${input:0:1} != '/' ];
then
	prefix="../"
fi

python2 moin.py $prefix$input

res=$?

if [ $res != 0 ];
then
    echo 'Python script error, check input file (if exists)'
    exit
fi

mkdir -p /home/vagrant/packet

mv botstream.xml /home/vagrant/packet/

cd ..

cp blockmon /home/vagrant/packet

rm -f FeatureLib.so

g++ -fPIC -shared -std=c++11 -Icore -Imessages -Ilib -Ilib/fc scripts/featurelib.cpp -o /home/vagrant/packet/FeatureLib.so

res=$?

if [ $res != 0  ]
then
    echo 'Compile error, check ' $input 'file'
    exit 1
fi

cd /home/vagrant/packet

if [ -e alone.tar ]
then 
    rm alone.tar
fi

tar -cvf alone.tar blockmon botstream.xml FeatureLib.so

echo "Ready."