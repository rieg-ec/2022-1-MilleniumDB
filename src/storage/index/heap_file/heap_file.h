#pragma once

#include <string>

#include "base/string_manager.h"
#include "storage/file_id.h"
#include "storage/page.h"

class HeapFile : StringManager {
public:
    static constexpr auto N_PAGES = 1024;

    HeapFile(const std::string& filename);
    ~HeapFile() = default;

    unsigned long write(const std::string& str);
    std::string   get_string(unsigned long id) const;

    Page& find_available_page(const std::string& bytes);

private:
    FileId file_id;
};
