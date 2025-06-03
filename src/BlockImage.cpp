#include "BlockImage.hpp"
namespace py = pybind11;


std::shared_ptr<BlockImage[]> BlockImage::initInnerBlockImage(uint32_t blocksPerBlock)
{
    return std::shared_ptr<BlockImage[]>(static_cast<BlockImage*>(operator new(blocksPerBlock*blocksPerBlock*sizeof(BlockImage))),
    [blocksPerBlock](BlockImage* p){
            for (uint32_t i = 0; i < blocksPerBlock * blocksPerBlock; ++i) {
                (p + i)->~BlockImage();
            }
            operator delete(p);
        });
}


BlockImage::BlockImage(const py::array_t<uint8_t>& arr,uint32_t x_st,uint32_t y_st,uint32_t size_,uint32_t pixelsPerBlock, uint32_t blocksPerBlock,float colorThershold)
        : chans(arr.shape(2)), PPB(pixelsPerBlock), size(size_), BPB(blocksPerBlock)
{

    auto arr_a = arr.unchecked<3>();
    float min_color=0,max_color=1;
    std::shared_ptr<uint64_t[]> color(new uint64_t[chans]);
    for (uint32_t i = x_st; i < x_st+size; ++i) {
        for (uint32_t j = y_st; j < y_st+size; ++j) {
            float value = 0;
            for (uint32_t k = 0; k < chans; ++k) {
                auto v = arr_a(i,j,k);
                value += (v*v)/65025.f;
                color.get()[k] += v;
            }
            value = sqrt(value);
            min_color = std::min(min_color, value); 
            max_color = std::max(max_color, value); 
        }
    }
    for (uint32_t i = 0; i < chans; ++i) {
        color.get()[i] /= size*size;
    }
    auto next_size = size / BPB;
    if (max_color - min_color > colorThershold && size > PPB)
    {
        innerBlockImage = initInnerBlockImage(blocksPerBlock);
        for (uint32_t i = 0; i < BPB; ++i) {
            for (uint32_t j = 0; j < BPB; ++j) {
                auto next_x_st = x_st + i * next_size;
                auto next_y_st = y_st + j * next_size;
                new (innerBlockImage.get() + i*BPB + j) BlockImage(arr,next_x_st,next_y_st,next_size, PPB,BPB,colorThershold);
            }
        }
        
        return;
    }

    BPB = 0;
    if (max_color - min_color > colorThershold)
    {
        data = py::array_t<uint8_t>({PPB,PPB,chans});
        auto &link = data.mutable_unchecked<3>();
        for (uint32_t i = 0; i < PPB; ++i) {
            for (uint32_t j = 0; j < PPB; ++j) {
                for (uint32_t k = 0; k < chans; ++k) {
                    link(i,j,k) = arr_a(x_st + i,y_st + j,k);
                }
            }
        }
        return;
    }

    data = py::array_t<uint8_t>({chans});
    auto &link = data.mutable_unchecked<1>();
    for (uint32_t i = 0; i < chans; ++i) {
        link(i) = color.get()[i];
    }
    PPB = 0;
}
 

void BlockImage::save(std::ofstream& file) const
{
    if (innerBlockImage != nullptr)
    {
        file.write("I", 1);
        for (uint32_t i = 0; i < BPB*BPB; ++i) {
            innerBlockImage.get()[i].save(file);
        }
        
    }
    else if (PPB == 0)
    {
        file.write("C", 1);
        file.write(reinterpret_cast<const char*>(data.data()), chans);
    }else if (data != nullptr)
    {
        file.write("B", 1);
        file.write(reinterpret_cast<const char*>(data.data()), PPB * PPB * chans);
    }
    else
    {
        file.write("E", 1);

    }
}

BlockImage::BlockImage(uint32_t size_,uint32_t channels, uint32_t pixelsPerBlock, uint32_t blocksPerBlock,bool isRoot_)
    : size(size_), chans(channels), PPB(pixelsPerBlock), BPB(blocksPerBlock), isRoot(isRoot_) {
        
}

BlockImage BlockImage::load(std::ifstream& file,uint32_t size_,uint32_t channels, uint32_t pixelsPerBlock, uint32_t blocksPerBlock,bool isRoot) 
{
    BlockImage res(size_,channels,pixelsPerBlock,blocksPerBlock,isRoot);
    char type;
    file.read(&type, 1);
    BlockImage* buffer = nullptr;
    switch (type)
    {
    case 'C':
        res.BPB = 0;
        res.PPB = 0;
        res.data = py::array_t<uint8_t>({channels});
        file.read(reinterpret_cast<char*>(res.data.mutable_data()), channels);
        break;
    case 'B':
        res.BPB = 0;
        res.data = py::array_t<uint8_t>({pixelsPerBlock, pixelsPerBlock, channels});
        file.read(reinterpret_cast<char*>(res.data.mutable_data()), pixelsPerBlock * pixelsPerBlock * channels);
        break;
    case 'I':
        res.innerBlockImage = initInnerBlockImage(blocksPerBlock);
        for (uint32_t i = 0; i <  blocksPerBlock*blocksPerBlock; ++i) {
            new (res.innerBlockImage.get() + i) BlockImage(load(file,size_ / blocksPerBlock, channels, pixelsPerBlock,blocksPerBlock));
        }
        
        break;            
    default:
        std::cerr << "Error: Unknown block type in file" << std::endl;
        break;
    }
    return res;
}
    
void BlockImage::writeToNumpy(py::array_t<uint8_t>& arr,uint32_t x_st,uint32_t y_st) const
{   
    
    // std::cout << x_st << " " << y_st << " " << innerBlockImage.get() << std::endl;
    // std::cout << size << " " << next_size << std::endl;
    if (innerBlockImage!=nullptr)
    {
        auto next_size = size / BPB;
        for (uint32_t i = 0; i < BPB; ++i) {
            for (uint32_t j = 0; j < BPB; ++j) {
                // std::cout << x_st << " " << y_st << " " << innerBlockImage.get() << " " << i << " " << j << std::endl;
               
                innerBlockImage.get()[i*BPB + j].writeToNumpy(arr,x_st+i*next_size,y_st+j*next_size);
                
            }
        }
        return;
    }
    auto arr_a = arr.mutable_unchecked<3>();
    if (PPB == 0)
    {
        auto& link = data.unchecked<1>();
        
        for (uint32_t i = 0; i < size; ++i) {
            for (uint32_t j = 0; j < size; ++j) {
                for (uint32_t k = 0; k < chans; ++k) {
                    arr_a(x_st+i,y_st+j,k) = link[k];
                }
            }
        }
        return;
    }
    // std::cout << "THIRD" << std::endl;
    
    auto& link = data.unchecked<3>();
    for (uint32_t i = 0; i < PPB; ++i) {
        for (uint32_t j = 0; j < PPB; ++j) {
            for (uint32_t k = 0; k < chans; ++k) {
                arr_a(x_st + i,y_st + j,k) = link(i,j,k);
            }
        }
    }
    


}   


BlockImage BlockImage::innerZeros(uint32_t size, uint32_t channels,uint32_t pixelsPerBlock,uint32_t blocksPerBlock)
{
    BlockImage res(size,channels,pixelsPerBlock,blocksPerBlock);
    if (size <= pixelsPerBlock)
    {
        res.BPB = 0;
        res.data = py::array_t<uint8_t>({pixelsPerBlock, pixelsPerBlock, channels});
        memset(res.data.mutable_data(),0,sizeof(uint8_t)*pixelsPerBlock * pixelsPerBlock * channels);
        return res;
    }
    res.innerBlockImage = initInnerBlockImage(blocksPerBlock);
    for (uint32_t i = 0; i < blocksPerBlock*blocksPerBlock; ++i) {
        new (res.innerBlockImage.get() + i) BlockImage(BlockImage::innerZeros(size/blocksPerBlock,channels,pixelsPerBlock,blocksPerBlock)); 
    }
   
    
    return res;
}

    
// public:
BlockImage BlockImage::zeros(uint32_t width, uint32_t height,uint32_t pixelsPerBlock,uint32_t channels,uint32_t blocksPerBlock)
{
    auto prefered_size = std::max(width, height);

    auto size = pixelsPerBlock;
    while (size < prefered_size) {
        size *= blocksPerBlock;
    }
    BlockImage res(size,channels,pixelsPerBlock,blocksPerBlock,true);
    if (size <= pixelsPerBlock)
    {
        res.BPB = 0;
        res.data = py::array_t<uint8_t>({pixelsPerBlock, pixelsPerBlock, channels});
        memset(res.data.mutable_data(),0,sizeof(uint8_t)*pixelsPerBlock * pixelsPerBlock * channels);
        return res;
    }
    res.innerBlockImage = initInnerBlockImage(blocksPerBlock);
    for (uint32_t i = 0; i < blocksPerBlock*blocksPerBlock; ++i) {
        new (res.innerBlockImage.get() + i) BlockImage(BlockImage::innerZeros(size/blocksPerBlock,channels,pixelsPerBlock,blocksPerBlock)); 
    }
    
    return res;
}


BlockImage::BlockImage(const py::array_t<uint8_t>& arr,uint32_t pixelsPerBlock,uint32_t blocksPerBlock, float colorThershold)
    : PPB(pixelsPerBlock), BPB(blocksPerBlock), isRoot(true) {
    if (arr.ndim() != 3 || arr.shape(0) == 0 || arr.shape(1) == 0 || arr.shape(2) == 0)
        throw std::invalid_argument("Not a image");
    if (blocksPerBlock == 0 || pixelsPerBlock == 0)
        throw std::invalid_argument("Pixels per block or blocks per block is zero");
    if (colorThershold < 0)
        throw std::invalid_argument("Color thershold < 0");
    chans = arr.shape(2);
    auto prefered_size = std::max(arr.shape(0), arr.shape(1));
    size = PPB;
    while (size < prefered_size) {
        size *= BPB;
    }
    
    float min_color=0,max_color=1;
    std::shared_ptr<uint64_t[]> color(new uint64_t[chans]);
    auto buffer = py::array_t<uint8_t>({size, size, chans});
    auto arr_a = arr.unchecked<3>();
    auto buff_a = buffer.mutable_unchecked<3>();
    auto padding_x = (size - arr.shape(0)) / 2;
    auto padding_y = (size - arr.shape(1)) / 2;
    for (uint32_t i = 0; i < arr.shape(0); ++i) {
        for (uint32_t j = 0; j < arr.shape(1); ++j) {
            float value = 0;
            for (uint32_t k = 0; k < chans; ++k) {
                auto v = arr_a(i,j,k);
                buff_a(i + padding_x,j + padding_y,k) = v; 
                value += (v*v)/65025.f;
                color.get()[k] += v;
            }
            value = sqrt(value);
            min_color = std::min(min_color, value/chans); 
            max_color = std::max(max_color, value/chans); 

        }
    }
    for (uint32_t i = 0; i < chans; ++i) {
        color.get()[i] /= size*size;
    }
    
    auto next_size = size / BPB;
    
    if (max_color - min_color > colorThershold && size > PPB)
    {
        innerBlockImage = initInnerBlockImage(BPB);
        for (uint32_t i = 0; i < BPB; ++i) {
            for (uint32_t j = 0; j < BPB; ++j) {
                new (innerBlockImage.get() + i*BPB + j) BlockImage(buffer,i * next_size,j * next_size,next_size, PPB,BPB,colorThershold);
            } 
        }
        return;
    }
    BPB = 0;
    if (max_color - min_color > colorThershold)
    {
        uint32_t x_st = 0;
        uint32_t y_st = 0;
        data = buffer;
        return;
    }
    data = py::array_t<uint8_t>({chans});
    auto& link = data.mutable_unchecked<1>();
    for (uint32_t i = 0; i < chans; ++i) {
        link(i) = color.get()[i];
    }
    PPB = 0;  
}

BlockImage BlockImage::fromcolor(py::tuple color, uint32_t size)
{
    BlockImage res(size,0,0);
    res.data = py::array_t<uint8_t>({static_cast<long long>(color.size())});
    auto& link = res.data.mutable_unchecked<1>();
    for (uint32_t i = 0;i<color.size();i++)
        link(i) = color[i].cast<uint8_t>();

    return res;
}

BlockImage &BlockImage::operator=(const BlockImage &other)
{
    if (isRoot)
    {
        size = other.size;
        chans = other.chans;
        PPB = other.PPB;
        BPB = other.BPB;
        innerBlockImage = other.innerBlockImage; 
        data = other.data;
        return *this;
    }

    if (size != other.size || chans != other.chans || !(BPB == 0 && (PPB==0 || PPB == other.PPB) || BPB == other.BPB))
        throw std::invalid_argument("Not compatible blocks");
    
    innerBlockImage = other.innerBlockImage;
    data = other.data;
    
    return *this;
}



void BlockImage::save(const std::string& filename) const
{
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    file.write(reinterpret_cast<const char*>(&chans), sizeof(chans));
    file.write(reinterpret_cast<const char*>(&PPB), sizeof(PPB));
    file.write(reinterpret_cast<const char*>(&BPB), sizeof(BPB));
    
    save(file);
    file.close();
}

BlockImage BlockImage::load(const std::string& filename) {
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Error opening file for reading: " + filename);
    }
    uint32_t size = 0,chans = 0, PPB = 0,BPB = 0;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    file.read(reinterpret_cast<char*>(&chans), sizeof(chans));
    file.read(reinterpret_cast<char*>(&PPB), sizeof(PPB));
    file.read(reinterpret_cast<char*>(&BPB), sizeof(BPB));
    auto block = load(file,size, chans, PPB, BPB,true);
    file.close();
    return block;
}

py::array_t<uint8_t>  BlockImage::toNumpy() const {
    py::array_t<uint8_t> arr({size, size, chans});
    writeToNumpy(arr,0,0);
    return arr;       

}

BlockImage& BlockImage::getBlock(std::pair<uint32_t,uint32_t> idx) const
{
    
    if (innerBlockImage.get() == nullptr || idx.first >= BPB || idx.second >= BPB) {
        throw std::out_of_range("Error: Index out of bounds.");
    }
    return innerBlockImage.get()[idx.first*BPB + idx.second];
}

BlockImage::BlockImage(const BlockImage& other) : size(other.size), chans(other.chans), PPB(other.PPB), BPB(other.BPB),
 isRoot(other.isRoot), data(other.data), innerBlockImage(other.innerBlockImage) {
}