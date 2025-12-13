#include "Inode.h"

namespace pagecache
{

    Inode::Inode(uint64_t ino, const std::string &path)
        : ino_(ino), path_(path), size_(0), fd_(-1), open_count_(0)
    {
    }

    Inode::~Inode()
    {
    }

}
