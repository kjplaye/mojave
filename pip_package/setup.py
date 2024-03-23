import importlib.resources as resources
import os
from distutils.command.build_ext import build_ext as build_ext_orig
from setuptools import setup, Extension


# Override to ignore the get_export_symbols for CTypes
# Credit: https://github.com/himbeles/ctypes-example/blob/master/setup.py
from distutils.command.build_ext import build_ext as build_ext_orig
class CTypesExtension(Extension):
    pass
class build_ext(build_ext_orig):
    def build_extension(self, ext):
        self._ctypes = isinstance(ext, CTypesExtension)
        return super().build_extension(ext)

    def get_export_symbols(self, ext):
        if self._ctypes:
            return ext.export_symbols
        return super().get_export_symbols(ext)

    def get_ext_filename(self, ext_name):
        if self._ctypes:
            return ext_name + ".so"
        return super().get_ext_filename(ext_name)

# Load the dynamic library path from pysdl2-dll.
sdl2_lib_path = ''
with resources.path('sdl2dll', '') as path:
    sdl2_lib_path = path
sdl2_lib_path = os.path.join(sdl2_lib_path, "dll")

extension_mod = CTypesExtension('mojave_eda._mojave',
                       sources=['mojave_eda/_mojave.c'],
                       python_requires='>=3',
                       include_dirs=['/usr/include/SDL2'],
                       library_dirs=[sdl2_lib_path],                          
                       libraries=['SDL2-2.0','SDL2_image-2.0', 'SDL2_ttf-2.0'],
                       py_modules=['mojave_eda.mojave_eda'],
)


setup(name='mojave_eda',
      version='1.0',
      description='Mojave data analysis',
      ext_modules=[extension_mod],
      py_modules=['mojave_eda,mojave_eda'],
      cmdclass={'build_ext':build_ext},
      setup_requires=["numpy"],
      install_requires=["numpy"],
)
