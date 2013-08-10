#!/usr/bin/env python

"""
setup.py file for SWIG RxBert
"""

from distutils.core import setup, Extension


TxBert_module = Extension('_TxBert',
                           sources=['TxBert.cpp', 'TxBert_wrap.cpp'],
                           )

setup (name = 'TxBert',
       version = '0.1',
       author      = "Peter Fetterer",
       description = """BERT PN Pattern Generator""",
       ext_modules = [TxBert_module],
       py_modules = ["TxBert"],
       )

