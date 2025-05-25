#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "BlockImage.hpp"

namespace py = pybind11;

PYBIND11_MODULE(bim, m) {
    m.doc() = "Block image processing library";

    py::class_<BlockImage>(m, "BlockImage")        
          .def(py::init<py::array_t<uint8_t>, uint32_t ,uint32_t, float>(),
             py::arg("arr"), py::arg("pixels_per_block"), py::arg("blocks_per_block")=2, py::arg("color_threshold") = 0.0f,
             "Create a BlockImage from numpy array with specified pixels per block, blocks per block and optional color threshold")
        
          .def("save", static_cast<void (BlockImage::*)(const std::string&)>(&BlockImage::save), py::arg("filename"),
             "Save the BlockImage to a file")
        
          .def("to_numpy", &BlockImage::toNumpy,
             "Convert the BlockImage to a numpy array")

          .def("resolution", &BlockImage::resolution,
          "Get the BlockImage resolution")

          .def("pixels_per_block",&BlockImage::pixelsPerBlock,
          "Get pixels count per block")

          .def("blocks_per_block",&BlockImage::blocksPerBlock,
          "Get blocks count per block");
     m.def("load", [](const std::string& filename) { return BlockImage::load(filename); }, py::arg("filename"),
           "Load a BlockImage from a file");

     m.def("zeros", &BlockImage::zeros ,py::arg("width"), py::arg("height"), py::arg("channels"), py::arg("pixelsPerBlock"), py::arg("blocksPerBlock")=2,
          "Create a new BlockImage filled zeros with specified width, height, channels, and pixels per block");
     

    m.attr("__version__") = "1.0.0";

}