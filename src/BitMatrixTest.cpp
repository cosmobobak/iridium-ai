#include "utilities/BitMatrix.hpp"

int main(int argc, char const *argv[])
{
    auto bb = BitMatrix<15,15>();
    for (auto bit : {51, 52, 53, 65, 66, 67, 68, 69, 79, 80, 81, 82, 83, 84, 85, 93, 94, 95, 96, 97, 98, 99, 100, 101, 108, 109, 110, 111, 112, 113, 114, 115, 116, 124, 125, 126, 127, 128, 129, 130, 140, 141, 142, 143, 144, 156, 157, 158}) {
        bb.set_bit(bit);
    }
    bb.show();
    puts("");
    bb = bb << 5;
    bb.show();
    puts("");
    bb = bb >> 5;
    bb.show();
    puts("");
    return 0;
}
