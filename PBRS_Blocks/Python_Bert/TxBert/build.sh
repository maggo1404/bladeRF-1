#!/bin/bash
swig -c++ -python -o TxBert_wrap.cpp TxBert.i
python ./setup.py build
