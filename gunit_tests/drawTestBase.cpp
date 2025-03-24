#include "drawTestBase.h"

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
//#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

DrawTestBase::DrawTestBase() {
    targetDir = fs::path(GTEST_PNG_ROOT) / ::testing::UnitTest::GetInstance()->current_test_suite()->name();
    if (!fs::is_directory(targetDir) || !fs::exists(targetDir))
        fs::create_directories(targetDir);
    diffDir = fs::path("DiffImages") / ::testing::UnitTest::GetInstance()->current_test_suite()->name();
    if (!fs::is_directory(diffDir) || !fs::exists(diffDir))
        fs::create_directories(diffDir);

    vkvg_device_create_info_t info{};
    info.samples = VK_SAMPLE_COUNT_4_BIT;
    dev  = vkvg_device_create(&info);
}

DrawTestBase::~DrawTestBase() {
    vkvg_device_destroy(dev);
}

void DrawTestBase::SetUp() {
    surf = vkvg_surface_create(dev, 256, 256);
}

void DrawTestBase::TearDown() {
    vkvg_surface_destroy(surf);
}

void DrawTestBase::compareWithRefImage() {
    fs::path targetPath = targetDir / ::testing::UnitTest::GetInstance()->current_test_info()->name();
    targetPath.replace_extension(".png");

    if (fs::exists(targetPath)) {
        int w = 0, h = 0, channels = 0;
        unsigned char *refImg = stbi_load((char*)targetPath.c_str(), &w, &h, &channels, 4); // force 4 components per pixel
        EXPECT_TRUE(refImg != nullptr) << "Could not load image from " << targetPath << stbi_failure_reason();
        EXPECT_EQ(vkvg_surface_get_width(surf),w);
        EXPECT_EQ(vkvg_surface_get_height(surf),h);

        unsigned char *img = (unsigned char*)malloc(w*h*4);
        unsigned char *diffImg = (unsigned char*)malloc(w*h*4);

        vkvg_status_t result = vkvg_surface_write_to_memory(surf, img);
        EXPECT_EQ(result, VKVG_STATUS_SUCCESS);

        uint32_t totDiff = 0;

        for(int y = 0; y < h; y++) {
            for(int x = 0; x < w; x++) {
                unsigned char* refPix = refImg + (y * w + x) * 4;
                unsigned char* pix = img + (y * w + x) * 4;
                unsigned char* diffPix = diffImg + (y * w + x) * 4;
                if (*refPix < *pix)
                    *diffPix = *pix - *refPix;
                else if (*refPix > *pix)
                    *diffPix = *refPix - *pix;
                else
                    *diffPix = 0;
                totDiff += (uint32_t)*diffPix;
            }
        }
        EXPECT_EQ(totDiff, 0);

        if (totDiff > 0) {
            fs::path diffPath = diffDir / ::testing::UnitTest::GetInstance()->current_test_info()->name();
            targetPath.replace_extension(".png");
            stbi_write_png((char*)diffPath.c_str(), (int32_t)w, (int32_t)h, 4, diffImg, (int32_t)(4 * w));
        }

        free(img);
        free(diffImg);
    } else {
        vkvg_status_t result = vkvg_surface_write_to_png(surf, (char*)targetPath.c_str());
        EXPECT_EQ(result, VKVG_STATUS_SUCCESS);
        GTEST_SKIP() << "Updating reference image: " << targetPath;
    }
}


