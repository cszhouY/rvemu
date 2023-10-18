#include <CPU.h>

#include <fstream>
#include <vector>


int main(int argc, char* argv[]) {
    if (2 != argc && 3 != argc) {
        std::cout << "Usage: " << argv[0] << " <file name> <(option)disk image>" << std::endl;
        return 0;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if(!file){
        std::cerr << "open file error" << std::endl;
        return 0;
    }
    std::vector<uint8_t> code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::vector<uint8_t> disk_img;

    if (3 == argc) {
        std::ifstream fdisk(argv[2], std::ios::binary);
        if (!fdisk) {
            std::cerr << "open file error" << std::endl;
            return 0;
        }
        std::vector<uint8_t> tmp((std::istreambuf_iterator<char>(fdisk)), std::istreambuf_iterator<char>());
        fdisk.close();
        disk_img.assign(tmp.begin(), tmp.end());
    }
    
    CPU cpu(code, disk_img);

    cpu.circle();

    cpu.dump_registers();

    return 0;
}
