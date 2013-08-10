#!/bin/bash
swig -c++ -python -o RxBert_wrap.cpp RxBert.i
python ./setup.py build
