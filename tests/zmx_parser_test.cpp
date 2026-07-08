#include <gtest/gtest.h>
#include "modalith/io/zmx_parser.hpp"
#include <fstream>
#include <filesystem>

TEST(ZmxParserTest, ParseBasicSequential) {
    std::string filename = "test_seq.zmx";
    std::ofstream out(filename);
    out << "VERS 140106\n"
        << "SURF 0\n"
        << "  TYPE STANDARD\n"
        << "  CURV 0.0\n"
        << "  DISP 10.0\n"
        << "  DIAM 5.0\n"
        << "SURF 1\n"
        << "  TYPE EVENASPH\n"
        << "  CURV 0.01\n"
        << "  DISP 2.0\n"
        << "  GLAS N-BK7\n"
        << "  CONI -0.5\n"
        << "  PARM 1 0.001\n"
        << "  PARM 2 0.0002\n";
    out.close();

    auto sys = modalith::io::parse_zmx(filename);

    EXPECT_EQ(sys.surfaces.size(), 2);
    
    // Surf 0
    EXPECT_EQ(sys.surfaces[0].profile.type, modalith::SurfaceType::Plane);
    EXPECT_DOUBLE_EQ(sys.surfaces[0].transform.translation.z, 0.0);
    EXPECT_DOUBLE_EQ(sys.surfaces[0].semi_diameter, 5.0);

    // Surf 1
    EXPECT_EQ(sys.surfaces[1].profile.type, modalith::SurfaceType::EvenAsphere);
    EXPECT_DOUBLE_EQ(sys.surfaces[1].transform.translation.z, 10.0);
    EXPECT_DOUBLE_EQ(sys.surfaces[1].profile.radius, 100.0);
    EXPECT_DOUBLE_EQ(sys.surfaces[1].profile.conic_constant, -0.5);
    EXPECT_EQ(sys.surfaces[1].profile.even_asphere_coefficients.size(), 2);
    EXPECT_DOUBLE_EQ(sys.surfaces[1].profile.even_asphere_coefficients[0], 0.001);
    EXPECT_DOUBLE_EQ(sys.surfaces[1].profile.even_asphere_coefficients[1], 0.0002);
    
    std::filesystem::remove(filename);
}
