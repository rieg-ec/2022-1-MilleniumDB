#include "heap_file.h"

#include <cassert>

#include "storage/buffer_manager.h"
#include "storage/file_manager.h"
#include "storage/index/heap_file/heap_page.h"
#include "storage/page.h"

using namespace std;

HeapFile::HeapFile(const string& filename) : file_id(file_manager.get_file_id(filename)) { }

std::string HeapFile::get_string(unsigned long id) const {
    // left-most 48 bits encode page number and right-most 16 bits encode dir entry
    unsigned long  pageno    = id >> 16;
    unsigned short dir_entry = id % 65536;
    Page&          page      = buffer_manager.get_page(file_id, pageno);
    HeapPage       heap_page = HeapPage(page);
    if (heap_page.dirsize - 1 < dir_entry) {
        return NULL;
    }

    auto [str_pos, size] = heap_page.get_dir_entry(dir_entry);
    std::string str      = heap_page.get_string(str_pos, size);

    buffer_manager.unpin(page);

    return str;
}

Page& HeapFile::find_available_page(const std::string& bytes) {
    /* returns a page with enough space for bytes plus 4 bytes of the dir entry */
    unsigned short bytes_length = bytes.length();
    uint_fast32_t  pages_n      = file_manager.count_pages(file_id);
    uint_fast32_t  page_n       = 0;
    bool           found_page   = false;
    Page*          page;

    while (page_n < pages_n && !found_page) {
        page = &buffer_manager.get_page(file_id, page_n);
        HeapPage heap_page(*page);
        if (heap_page.freespace >= bytes_length + 4) {
            found_page = true;
        } else {
            buffer_manager.unpin(*page);
            page_n++;
        }
    }

    if (found_page) {
        return *page;
    } else {
        return buffer_manager.append_page(file_id);
    }
}

unsigned long HeapFile::write(const std::string& str) {
    /* returns ID consisting of page number and directory entry in the heap file */
    Page&          page = find_available_page(str);
    HeapPage       heap_page(page);
    unsigned short dir_entry = heap_page.insert_string(str);

    heap_page.make_dirty();
    unsigned long id = page.get_page_number();
    buffer_manager.unpin(page);

    return (id << 16) + dir_entry;
}
