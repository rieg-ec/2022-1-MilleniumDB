/* HeapPage is an utility class to make it easier for HeapFile to read
 * and format a page.
 */
#pragma once

#include "storage/page.h"
#include <string>
#include <tuple>

class HeapPage {
public:
    HeapPage(Page& page);
    ~HeapPage() = default;

    const uint_fast64_t pageno;
    unsigned short      dirsize;
    unsigned short      freespace;

    std::string                                get_string(unsigned short str_pos, unsigned short size);
    std::tuple<unsigned short, unsigned short> get_dir_entry(unsigned short position);

    unsigned short insert_string(const std::string& str);

    void make_dirty() {
        page.make_dirty();
    }

private:
    Page& page;
    char* bytes;
};
