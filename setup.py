from distutils.core import setup, Extension

vsomeipPy = Extension('vsomeip',
                    sources = ['vsomeip_python.cpp'],
                    libraries = ['vsomeip', 'vsomeip-cfg'],
                    library_dirs = ['/usr/local/lib/'],
                    extra_compile_args=['-std=c++11'],
                    runtime_library_dirs=['/usr/local/lib/'])

setup (name = 'vsomeipPy',
       version = '1.0',
       description = 'vsomeip crude Python bindings',
       ext_modules = [vsomeipPy])
