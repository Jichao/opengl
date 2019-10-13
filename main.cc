#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
int texture();
int refImp();
int main() {
    auto file_logger =
        spdlog::basic_logger_mt("basic_logger", "log.txt");
    spdlog::set_default_logger(file_logger);
    return texture();
}