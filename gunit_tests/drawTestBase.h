#include "vkvg.h"
#include <gtest/gtest.h>
#include <filesystem>

namespace fs = std::filesystem;

class DrawTestBase : public testing::Test {
  public:
    VkvgDevice  dev;
    VkvgSurface surf;
    fs::path targetDir;
    fs::path diffDir;

  protected:
    DrawTestBase();
    ~DrawTestBase() override;
    void SetUp() override;
    void TearDown() override;

    void compareWithRefImage();
};

