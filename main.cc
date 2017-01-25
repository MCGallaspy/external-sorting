#include "core/aligned_buffer.hh"
#include "core/app-template.hh"
#include "core/file.hh"
#include "core/reactor.hh"
#include "core/semaphore.hh"
#include "core/sstring.hh"

#include <iostream>

static constexpr size_t PAGE_SIZE = 4096; // I checked it for my system
static constexpr uint64_t BYTES_PER_RUN = PAGE_SIZE*10ull; // arbitrary value, adjust after benchmarking

struct file_wrapper {
    file_wrapper(file&& f) : f(std::move(f)) {}
    file f;
};

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }
    
    sstring filename(argv[1]);
    int fake_argc = 1;
    
    semaphore done {0};

    app_template app;
    app.run(fake_argc, argv,
    [filename, &done] {
        open_file_dma(filename, open_flags::rw).then(
        [&done] (file f) {
            auto fw = new file_wrapper{std::move(f)};
            uint64_t file_size = fw->f.size().get0();
            const uint32_t rem = file_size % BYTES_PER_RUN == 0 ? 0 : 1;
            const uint32_t num_runs = rem + file_size / BYTES_PER_RUN;
            std::cout << "filesize is " << file_size << "\n"
                      << "num runs is " << num_runs << "\n";
            for (uint32_t i = 0; i < num_runs; ++i) {
                uint64_t remaining = file_size - (i * BYTES_PER_RUN);
                const size_t bufsize = std::min<size_t>(remaining, BYTES_PER_RUN);
                std::cout << "bufsize is " << bufsize << "\n";
                auto buf = allocate_aligned_buffer<unsigned char>(bufsize, PAGE_SIZE);  
                fw->f.dma_read(i * PAGE_SIZE, buf.get(), bufsize).then(
                [buf = std::move(buf), &done] (size_t bytes_read) mutable {
                    std::cout << "Read " << bytes_read << " bytes\n";
                    std::cout.write(reinterpret_cast<char*>(buf.get()), bytes_read);
                    std::cout.flush();
                    done.signal();
                });
            }
        });
        return make_ready_future();
    });
    
    done.wait();
    return 0;
}