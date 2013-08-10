#!/usr/bin/env python

"""
setup.py file for SWIG RxBert
"""

from distutils.core import setup, Extension
from distutils.core import setup, Extension
from distutils.command.build_ext import build_ext

copt =  { 'CXX' : ['-fopenmp','-O3','-ffast-math','-march=native']       }
lopt =  {'CXX' : [''] }

class build_ext_subclass( build_ext ):
    def build_extensions(self):
        c = self.compiler.compiler_type
        if copt.has_key(c):
           for e in self.extensions:
               e.extra_compile_args = copt[ c ]
        if lopt.has_key(c):
            for e in self.extensions:
                e.extra_link_args = lopt[ c ]
        build_ext.build_extensions(self)


RxBert_module = Extension('_RxBert',
                           sources=['RxBert.cpp', 'RxBert_wrap.cpp'],
                           )

setup (name = 'RxBert',
       version = '0.1',
       author      = "Peter Fetterer",
       description = """BERT PN Pattern Checker""",
       ext_modules = [RxBert_module],
       py_modules = ["RxBert"],
       cmdclass = {'build_ext': build_ext_subclass }
       )

