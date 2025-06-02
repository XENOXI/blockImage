#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <cstdint>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <functional>
namespace py = pybind11;


class BlockImage {
private:
    uint32_t size = 0;
    uint32_t chans = 0;
    uint32_t PPB = 0;
    uint32_t BPB = 0;
    const bool isRoot = false;
    py::array_t<uint8_t> data;
    std::shared_ptr<BlockImage[]> innerBlockImage = nullptr;
    std::function<BlockImage&(std::pair<uint32_t,uint32_t>)> blocksProxy = std::bind(&BlockImage::getBlock,this,std::placeholders::_1);

    static std::shared_ptr<BlockImage[]> BlockImage::initInnerBlockImage(uint32_t blocksPerBlock);
    void save(std::ofstream& file) const;
    static BlockImage load(std::ifstream& file,uint32_t size_,uint32_t channels, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2,bool isRoot = false);
    void writeToNumpy(py::array_t<uint8_t>& arr, uint32_t x_st, uint32_t y_st) const;
    BlockImage(const py::array_t<uint8_t>& arr, uint32_t x_st, uint32_t y_st, uint32_t size, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2, float colorThreshold = 0.0f);
    BlockImage(uint32_t size_,uint32_t channels, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2,bool isRoot_ = false);
    static BlockImage innerZeros(uint32_t _size, uint32_t channels,uint32_t pixelsPerBlock,uint32_t blocksPerBlock);

public:

    static BlockImage zeros(uint32_t width, uint32_t height, uint32_t pixelsPerBlock=4,uint32_t channels=4,uint32_t blocksPerBlock=2);
    BlockImage(const py::array_t<uint8_t>& arr, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2, float colorThreshold = 0.0f);
    static BlockImage fromcolor(py::tuple color,uint32_t size);
    BlockImage& operator=(const BlockImage& other);

    void save(const std::string& filename) const;
    static BlockImage load(const std::string& filename);
    py::array_t<uint8_t> toNumpy() const;
    BlockImage& getBlock(std::pair<uint32_t,uint32_t> idx) const;
    uint32_t resolution() const { return size; }
    uint32_t pixelsPerBlock() const { return PPB; }
    uint32_t blocksPerBlock() const { return BPB; }   
    uint32_t channels() const { return chans; }
    bool hasInnerBlocks() const {return BPB != 0;}
    BlockImage(const BlockImage& other);
    std::function<BlockImage&(std::pair<uint32_t,uint32_t>)> getBlockProxy() const {return blocksProxy;};
    const py::array_t<uint8_t>& getCanvas() const {return data;};
    ~BlockImage()=default;
};


