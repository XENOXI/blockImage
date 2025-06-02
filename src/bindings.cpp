#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "BlockImage.hpp"

namespace py = pybind11;

PYBIND11_MODULE(blage, m) {
   m.doc() = "Block image processing library";

   py::class_<std::function<BlockImage&(std::pair<uint32_t,uint32_t>)>>(m,"_proxy_blocks")
      .def("__getitem__", &std::function<BlockImage&(std::pair<uint32_t,uint32_t>)>::operator(),
            "Index in the inner blocks")
      .def("__setitem__",[](const std::function<BlockImage&(std::pair<uint32_t,uint32_t>)> &self,std::pair<uint32_t,uint32_t> key,BlockImage& value){self(key) = value;},
            "Set block");

   py::class_<BlockImage>(m, "bim")        
         .def(py::init<py::array_t<uint8_t>, uint32_t ,uint32_t, float>(),
            py::arg("arr"), py::arg("pixels_per_block"), py::arg("blocks_per_block")=2, py::arg("color_threshold") = 0.0f,
            "Create a BlockImage from numpy array with specified pixels per block, blocks per block and optional color threshold")
      
         .def("to_numpy", &BlockImage::toNumpy,
            "Convert the BlockImage to a numpy array")

         .def_property_readonly("resolution", &BlockImage::resolution,
         "Get the BlockImage resolution")

         .def_property_readonly("pixels_per_block",&BlockImage::pixelsPerBlock,
         "Get pixels count per block")

         .def_property_readonly("blocks_per_block",&BlockImage::blocksPerBlock,
         "Get blocks count per block")

         .def_property_readonly("has_inner_block",&BlockImage::hasInnerBlocks,
         "Return if block consist another blocks")
         
         .def_property_readonly("channels",&BlockImage::channels,
         "Get image channels")
         
         .def_property_readonly("blocks",&BlockImage::getBlockProxy,
         "Get inner blocks")

         .def_property_readonly("canvas",&BlockImage::getCanvas,
         "Get canvas")

         .def("save", static_cast<void (BlockImage::*)(const std::string&) const>(&BlockImage::save), py::arg("filename"),
            "Save the BlockImage to a file")
      
      ;

      
   m.def("load", [](const std::string& filename) { return BlockImage::load(filename); }, py::arg("filename"),
         "Load a BlockImage from a file");

   m.def("zeros", &BlockImage::zeros ,py::arg("width"), py::arg("height"), py::arg("channels"), py::arg("pixelsPerBlock"), py::arg("blocksPerBlock")=2,
         "Create a new BlockImage filled zeros with specified width, height, channels, and pixels per block");
   

   m.attr("__version__") = "1.0.0";

}