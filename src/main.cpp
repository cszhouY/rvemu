#include <CPU.h>

#include <fstream>
#include <vector>


int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " --in <filename>" << std::endl;
        return 0;
    }

    std::ifstream file(argv[2], std::ios::binary);
    if(!file){
        std::cerr << "open file error" << std::endl;
        return 0;
    }
    std::vector<uint8_t> code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    CPU cpu(code);

    cpu.circle();

    cpu.dump_registers();

    return 0;
}
