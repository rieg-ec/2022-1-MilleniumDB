#include "storage/index/heap_file/heap_page.h"

#include "storage/page.h"
#include <cassert>
#include <string>
#include <tuple>

HeapPage::HeapPage(Page& page) : pageno(page.get_page_number()), page(page), bytes(page.get_bytes()) {
    memcpy(bytes, &pageno, sizeof(pageno) - sizeof(short));
    memcpy(&dirsize, &bytes[6], sizeof(short));

    if (dirsize == 0) {
        // page is empty, need to add page size minus header size as freespace
        freespace = Page::MDB_PAGE_SIZE - 10;
        memcpy(&bytes[8], &freespace, sizeof(short));
    } else {
        memcpy(&freespace, &bytes[8], sizeof(short));
    }
}

std::tuple<unsigned short, unsigned short> HeapPage::get_dir_entry(unsigned short position) {
    unsigned short str_pos;
    // could go from 10 to 4095
    memcpy(&str_pos, &bytes[10 + position * 4], sizeof(unsigned short));

    unsigned short size;
    memcpy(&size, &bytes[12 + position * 4], sizeof(unsigned short));

    return std::make_tuple(str_pos, size);
}

std::string HeapPage::get_string(unsigned short str_pos, unsigned short size) {
    std::string str(&bytes[str_pos], size);
    return str;
}

unsigned short HeapPage::insert_string(const std::string& str) {
    // returns created entry's directory index
    unsigned short length = str.size();
    assert(length <= (freespace - 4) && "not enough free space");

    unsigned short last_str_pos, last_str_size;

    if (dirsize == 0) {
        // mark 4096th byte as occupied to start adding from 4095th
        last_str_pos  = 4096;
        last_str_size = 0;
    } else {
        std::tie(last_str_pos, last_str_size) = get_dir_entry(dirsize - 1);
    }

    assert(last_str_pos - 10 > length && "not enough free space");
    unsigned short position = last_str_pos - (length);
    // insert directory
    memcpy(&bytes[10 + dirsize * 4], &position, sizeof(unsigned short));
    memcpy(&bytes[12 + dirsize * 4], &length, sizeof(unsigned short));
    // insert string
    memcpy(&bytes[position], str.c_str(), sizeof(char) * length);
    // update directory
    freespace -= (length + 4);
    dirsize++;
    memcpy(&bytes[6], &dirsize, sizeof(short));
    memcpy(&bytes[8], &freespace, sizeof(short));

    return dirsize - 1;
}
