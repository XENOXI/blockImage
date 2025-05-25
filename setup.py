from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        "bim",  # Module name
        ["src/bindings.cpp","src/BlockImage.cpp"],  # Source files
        include_dirs=[pybind11.get_include()],  # Include pybind11 headers
        language="c++",  # Specify C++ as the language
    )
]

setup(
    name="bim",
    version="1.0.0",
    ext_modules=ext_modules,
    package_data={"bim": ["bim.pyi"]}
)
