from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension
from pybind11 import get_include

__version__ = "1.0.0"

ext_modules = [
    Pybind11Extension(
        "blage",  
        ["src/bindings.cpp","src/BlockImage.cpp"],  
        include_dirs=[get_include()],  
        language="c++", 
        define_macros=[("VERSION_INFO", __version__)]
    )
]

setup(
    name="blage",
    version=__version__,
    author="Vasiliy Lekomtsev",
    author_email="mexerily@gmail.com",
    url="https://github.com/XENOXI/blockImage",
    description="A Python library for block-based image processing",
    ext_modules=ext_modules,
    package_data={"blage": ["blage.pyi"]}
)
