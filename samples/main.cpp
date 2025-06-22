#include "SampleApp.hpp"
#include "VkvgTest.hpp"

#include "argparse/argparse.hpp"

int main(int argc, char **argv) {
    SampleApp app{};

    argparse::ArgumentParser program("samples");
    program.add_argument("-w", "--width")
        .help("Output width")
        .default_value<uint32_t>(512)
        .store_into<uint32_t>(app.width);
    program.add_argument("-h", "--height")
        .help("Output height")
        .default_value<uint32_t>(512)
        .store_into<uint32_t>(app.height);
    program.add_argument("-i", "--iterations")
        .help("iterations for each test, 0 is infinite loop")
        .default_value<uint32_t>(0)
        .store_into<uint32_t>(app.iterations);
    program.add_argument("-s", "--size")
        .help("test size when applicable")
        .default_value<uint32_t>(500)
        .store_into<uint32_t>(app.testSize);

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    program.add_argument("--severity")
        .help("Debug messenger severity: 3:Err 2:Warn 1:Info 0:Verbose")
        .default_value<std::vector<uint32_t>>({0})
        .scan<'u', uint32_t>()
        .nargs(argparse::nargs_pattern::at_least_one);
    program.add_argument("--logtype")
        .help("Log type bits: 1:General 2:Validation 4:Perf 8:DevAddrBind")
        .default_value<uint32_t>(2)
        .store_into<uint32_t>(app.logType);
#endif

    program.add_argument("-t", "--test-indices")
        .default_value<std::vector<uint32_t>>({})
        .help("test indices to run")
        .scan<'u', uint32_t>()
        .nargs(argparse::nargs_pattern::at_least_one);

    program.add_argument("--list-tests")
        .help("Get tests list")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--list-gpus")
        .help("Get available GPU's, select with option '-g'")
        .default_value(false)
        .implicit_value(true)
        .store_into(app.listGpus);
    program.add_argument("-g", "--gpu")
        .help("GPU index")
        .default_value<int32_t>(-1)
        .store_into<int32_t>(app.gpuIndex);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if (program["--list-tests"] == true) {
        for (auto it = VkvgTest::tests.begin(); it != VkvgTest::tests.end(); ++it) {
            int index = std::distance(VkvgTest::tests.begin(), it);
            std::cout << std::setw(4) << index << " " << (*it)->name << std::endl;
        }
        exit(0);
    }

    app.testsToRun = program.get<std::vector<uint32_t>>("--test-indices");

#if defined(DEBUG) && defined(VKVG_DBG_UTILS)
    app.logSeverity = program.get<std::vector<uint32_t>>("--severity");
#endif

    app.Init();
    app.Run();

}
