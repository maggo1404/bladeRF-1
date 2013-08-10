#!/bin/bash
cd RxBert
./build.sh
cd ..
cd TxBert
./build.sh
cd ..

cp -v ./RxBert/build/lib.linux-x86_64-2.7/* ./modules/
rm ./RxBert/build/lib.linux-x86_64-2.7/*
cp -v ./TxBert/build/lib.linux-x86_64-2.7/* ./modules/
rm ./TxBert/build/lib.linux-x86_64-2.7/*

