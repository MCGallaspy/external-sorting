#include "core/app-template.hh"
#include "core/reactor.hh"
#include "core/sstring.hh"

#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }
    
    sstring filename(argv[1]);
    int fake_argc = 1;
    
    app_template app;
    app.run(fake_argc, argv, [] {
            std::cout << smp::count << "\n";
            return make_ready_future<>();
    });
    
    return 0;
}