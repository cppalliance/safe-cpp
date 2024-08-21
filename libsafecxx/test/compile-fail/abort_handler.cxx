#include <cstdlib>
#include <csignal>

// This is a hack to implement death tests in CTest.
extern "C" void error_test_handle_abort(int) {
    std::_Exit(EXIT_FAILURE);
}

struct test_override_abort {
    test_override_abort() noexcept {
        std::signal(SIGABRT, error_test_handle_abort);
    }
};

test_override_abort handler{};
