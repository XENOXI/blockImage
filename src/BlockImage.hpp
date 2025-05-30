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
    bool isRoot = false;
    std::shared_ptr<BlockImage[]> innerBlockImage = nullptr;
    std::shared_ptr<uint8_t[]> data = nullptr;
    std::function<BlockImage&(std::pair<uint32_t,uint32_t>)> blocksProxy = std::bind(&BlockImage::getBlock,this,std::placeholders::_1);
    std::function<uint8_t&(std::pair<uint32_t,uint32_t>)> pixelsProxy = std::bind(&BlockImage::getPixel,this,std::placeholders::_1);

    void setPixel(uint32_t x, uint32_t y, uint32_t channel, uint8_t value);

    void save(std::ofstream& file);
    static BlockImage load(std::ifstream& file,uint32_t size_,uint32_t channels, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2);
    void writeToNumpy(py::array_t<uint8_t>& arr, uint32_t x_st, uint32_t y_st);
    BlockImage(const py::array_t<uint8_t>& arr, uint32_t x_st, uint32_t y_st, uint32_t size, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2, float colorThreshold = 0.0f);
    BlockImage(uint32_t size_,uint32_t channels, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2,bool isRoot_ = false);
    static BlockImage innerZeros(uint32_t _size, uint32_t channels,uint32_t pixelsPerBlock,uint32_t blocksPerBlock);

public:

    static BlockImage zeros(uint32_t width, uint32_t height, uint32_t channels, uint32_t pixelsPerBlock,uint32_t blocksPerBlock=2);
    BlockImage(const py::array_t<uint8_t>& arr, uint32_t pixelsPerBlock, uint32_t blocksPerBlock=2, float colorThreshold = 0.0f);
    
    BlockImage& operator=(const BlockImage& other);

    void save(const std::string& filename);
    static BlockImage load(const std::string& filename);
    py::array_t<uint8_t> toNumpy();

    uint32_t resolution() { return size; }
    uint32_t pixelsPerBlock() { return PPB; }
    uint32_t blocksPerBlock() { return BPB; }   
    uint32_t channels() { return chans; }
    bool hasInnerBlocks() {return BPB != 0;}
    BlockImage(const BlockImage& other);
    std::function<BlockImage&(std::pair<uint32_t,uint32_t>)> getBlockProxy() {return blocksProxy;};
    std::function<uint8_t&(std::pair<uint32_t,uint32_t>)> getCanvasProxy() {return pixelsProxy;};
    BlockImage& getBlock(std::pair<uint32_t,uint32_t> idx) const;
    
    uint8_t& const getPixel(std::pair<uint32_t,uint32_t> idx) const {
        if (data.get() == nullptr || idx.first >= BPB || idx.second >= BPB) {
        throw std::out_of_range("Error: Index out of bounds.");
    }
        return data.get()[idx.first*PPB + idx.second];
    }
    void setCanvas(py::array_t<uint8_t>& newCanvas,bool copy=true);
    ~BlockImage() {std::cout << this << " DELETION" << std::endl << isRoot << " DELETION" << std::endl << " " << innerBlockImage << std::endl;};
};


