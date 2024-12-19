#include "GCode.h"
#include "GCodeProcessor.h"
#include "nist_float.h"

#include "../Unity/src/unity.h"
#include "TestRegistry.h"

#include <cstring>

REGISTER_TEST(GCodeTest,basic)
{
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gca;

    const char *g1("G32 X1.2 Y-2.3");
    bool ok= gp.parse(g1, gca);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(1, gca.size());
    GCode gc1= gca[0];
    TEST_ASSERT_TRUE(gc1.has_g());
    TEST_ASSERT_TRUE(!gc1.has_m());
    TEST_ASSERT_EQUAL_INT(32, gc1.get_code());
    TEST_ASSERT_EQUAL_INT(0, gc1.get_subcode());
    TEST_ASSERT_EQUAL_INT(2, gc1.get_num_args());
    TEST_ASSERT_TRUE(gc1.has_arg('X'));
    TEST_ASSERT_TRUE(gc1.has_arg('Y'));
    TEST_ASSERT_TRUE(!gc1.has_arg('Z'));
    TEST_ASSERT_EQUAL_FLOAT(1.2, gc1.get_arg('X'));
    TEST_ASSERT_EQUAL_FLOAT(-2.3, gc1.get_arg('Y'));
}

GCode gc2;

REGISTER_TEST(GCodeTest,subcode)
{
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gca;
    const char *g2("G32.2 X1.2 Y2.3");
    bool ok= gp.parse(g2, gca);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(1, gca.size());
    gc2= gca[0];

    TEST_ASSERT_TRUE(gc2.has_g());
    TEST_ASSERT_TRUE(!gc2.has_m());
    TEST_ASSERT_EQUAL_INT(32, gc2.get_code());
    TEST_ASSERT_EQUAL_INT(2, gc2.get_subcode());
    TEST_ASSERT_EQUAL_INT(2, gc2.get_num_args());
    TEST_ASSERT_TRUE(gc2.has_arg('X'));
    TEST_ASSERT_TRUE(gc2.has_arg('Y'));
    TEST_ASSERT_EQUAL_FLOAT(1.2, gc2.get_arg('X'));
    TEST_ASSERT_EQUAL_FLOAT(2.3, gc2.get_arg('Y'));
}

REGISTER_TEST(GCodeTest,copy)
{
    // test equals
    GCode gc3;
    gc3= gc2;
    TEST_ASSERT_TRUE(gc3.has_g());
    TEST_ASSERT_TRUE(!gc3.has_m());
    TEST_ASSERT_EQUAL_INT(32, gc3.get_code());
    TEST_ASSERT_EQUAL_INT(2, gc3.get_subcode());
    TEST_ASSERT_EQUAL_INT(2, gc3.get_num_args());
    TEST_ASSERT_TRUE(gc3.has_arg('X'));
    TEST_ASSERT_TRUE(gc3.has_arg('Y'));
    TEST_ASSERT_EQUAL_FLOAT(1.2, gc3.get_arg('X'));
    TEST_ASSERT_EQUAL_FLOAT(2.3, gc3.get_arg('Y'));

    // test copy ctor
    GCode gc4(gc2);
	TEST_ASSERT_TRUE(gc4.has_g());
	TEST_ASSERT_TRUE(!gc4.has_m());
	TEST_ASSERT_EQUAL_INT(32, gc4.get_code());
	TEST_ASSERT_EQUAL_INT(2, gc4.get_subcode());
	TEST_ASSERT_EQUAL_INT(2, gc4.get_num_args());
	TEST_ASSERT_TRUE(gc4.has_arg('X'));
	TEST_ASSERT_TRUE(gc4.has_arg('Y'));
	TEST_ASSERT_EQUAL_FLOAT(1.2, gc4.get_arg('X'));
	TEST_ASSERT_EQUAL_FLOAT(2.3, gc4.get_arg('Y'));
}

REGISTER_TEST(GCodeTest, Multiple_commands_on_line_no_spaces) {
    GCodeProcessor gp;
    const char *gc= "M123X1Y2G1X10Y20Z0.634";
    GCodeProcessor::GCodes_t gcodes;
    bool ok= gp.parse(gc, gcodes);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT( 2, gcodes.size());
    auto a= gcodes[0];
    TEST_ASSERT_TRUE(a.has_m());
    TEST_ASSERT_EQUAL_INT(123, a.get_code());
    TEST_ASSERT_TRUE(a.has_arg('X')); TEST_ASSERT_EQUAL_INT(1, a.get_arg('X'));
    TEST_ASSERT_TRUE(a.has_arg('Y')); TEST_ASSERT_EQUAL_INT(2, a.get_arg('Y'));
    auto b= gcodes[1];
    TEST_ASSERT_TRUE(b.has_g());
    TEST_ASSERT_EQUAL_INT(1, b.get_code());
    TEST_ASSERT_TRUE(b.has_arg('X')); TEST_ASSERT_EQUAL_INT(10, b.get_arg('X'));
    TEST_ASSERT_TRUE(b.has_arg('Y')); TEST_ASSERT_EQUAL_INT(20, b.get_arg('Y'));
    TEST_ASSERT_TRUE(b.has_arg('Z')); TEST_ASSERT_EQUAL_INT(0.634f, b.get_arg('Z'));
}

REGISTER_TEST(GCodeTest, Modal_G1_and_comments) {
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gcodes;
    bool ok= gp.parse("G1 X0", gcodes);
    TEST_ASSERT_TRUE(ok);
    const char *gc= "( this is a comment )X100Y200 ; G23 X0";
    gcodes.clear();
    ok= gp.parse(gc, gcodes);
    printf("%s\n", gcodes[0].get_error_message());
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(1, gcodes.size());
    auto a = gcodes[0];
    TEST_ASSERT_TRUE(a.has_g());
    TEST_ASSERT_EQUAL_INT(1, a.get_code());
    TEST_ASSERT_TRUE(a.has_arg('X')); TEST_ASSERT_EQUAL_INT(100, a.get_arg('X'));
    TEST_ASSERT_TRUE(a.has_arg('Y')); TEST_ASSERT_EQUAL_INT(200, a.get_arg('Y'));
    TEST_ASSERT_FALSE(a.has_arg('Z'));
}

REGISTER_TEST(GCodeTest, Line_numbers_and_checksums) {
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gcodes;
    bool ok= gp.parse("N10 G1 X0", gcodes);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_TRUE(gcodes.empty());

    // test reset line number
    gcodes.clear();
    ok= gp.parse("N10 M110*123", gcodes);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(10, gp.get_line_number());
    TEST_ASSERT_TRUE(gcodes.empty());

    // test alternate reset syntax
    gcodes.clear();
    ok= gp.parse("N0 M110 N0*125", gcodes);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(0, gp.get_line_number());
    TEST_ASSERT_TRUE(gcodes.empty());

    // Bad line number
    gcodes.clear();
    ok= gp.parse("N95 G1 X-4.992 Y-14.792 F12000.000*97", gcodes);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_TRUE(gcodes.empty());

    gcodes.clear();
    ok= gp.parse("N94 M110*123", gcodes);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(94, gp.get_line_number());
    ok= gp.parse("N95 G1 X-4.992 Y-14.792 F12000.000*97", gcodes);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(1, gcodes.size());

    // Bad checksum
    gcodes.clear();
    ok= gp.parse("N94 M110*123", gcodes);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(94, gp.get_line_number());
    ok= gp.parse("N95 G1 X-4.992 Y-14.792 F12000.000*98", gcodes);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_TRUE(gcodes.empty());
}

REGISTER_TEST(GCodeTest,t_code)
{
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gca;

    // test that a T1 will be converted effectively to M6 T1
    const char *g1("T1");
    bool ok= gp.parse(g1, gca);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(1, gca.size());
    GCode gc1= gca[0];
    TEST_ASSERT_FALSE(gc1.has_g());
    TEST_ASSERT_TRUE(gc1.has_m());
    TEST_ASSERT_EQUAL_INT(6, gc1.get_code());
    TEST_ASSERT_EQUAL_INT(0, gc1.get_subcode());
    TEST_ASSERT_EQUAL_INT(1, gc1.get_num_args());
    TEST_ASSERT_TRUE(gc1.has_arg('T'));
    TEST_ASSERT_EQUAL_INT(1, gc1.get_arg('T'));
}

REGISTER_TEST(GCodeTest, illegal_command_word) {
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gcodes;
    bool ok= gp.parse("1 X0", gcodes);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_FALSE(gcodes.empty());
    TEST_ASSERT_TRUE(gcodes.back().has_error());
    TEST_ASSERT_NOT_NULL(gcodes.back().get_error_message());
    TEST_ASSERT_TRUE(strlen(gcodes.back().get_error_message()) > 0);
}

REGISTER_TEST(GCodeTest, illegal_parameter_word) {
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gcodes;
    bool ok= gp.parse("G1 1.2 X0", gcodes);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_FALSE(gcodes.empty());
    TEST_ASSERT_TRUE(gcodes.back().has_error());
    TEST_ASSERT_NOT_NULL(gcodes.back().get_error_message());
    TEST_ASSERT_TRUE(strlen(gcodes.back().get_error_message()) > 0);
}

REGISTER_TEST(GCodeTest, illegal_parameter_word_value) {
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gcodes;
    bool ok= gp.parse("G1 Y X0", gcodes);
    TEST_ASSERT_FALSE(ok);
    TEST_ASSERT_FALSE(gcodes.empty());
    TEST_ASSERT_TRUE(gcodes.back().has_error());
    TEST_ASSERT_NOT_NULL(gcodes.back().get_error_message());
    TEST_ASSERT_TRUE(strlen(gcodes.back().get_error_message()) > 0);
}

REGISTER_TEST(GCodeTest, gcode_float_edge_cases) {
    GCodeProcessor gp;
    GCodeProcessor::GCodes_t gca;

    // make sure 0X16 is not hex
    const char *g1("G1Y0X16G1X1E4");
    bool ok= gp.parse(g1, gca);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(2, gca.size());
    GCode gc= gca[0];
    TEST_ASSERT_TRUE(gc.has_g());
    TEST_ASSERT_EQUAL_INT(1, gc.get_code());
    TEST_ASSERT_EQUAL_INT(2, gc.get_num_args());
    TEST_ASSERT_TRUE(gc.has_arg('X'));
    TEST_ASSERT_TRUE(gc.has_arg('Y'));
    TEST_ASSERT_EQUAL_FLOAT(16.0, gc.get_arg('X'));
    TEST_ASSERT_EQUAL_FLOAT(0, gc.get_arg('Y'));

    GCode gc1= gca[1];
    TEST_ASSERT_TRUE(gc1.has_g());
    TEST_ASSERT_EQUAL_INT(1, gc1.get_code());
    TEST_ASSERT_EQUAL_INT(2, gc1.get_num_args());
    TEST_ASSERT_TRUE(gc1.has_arg('X'));
    TEST_ASSERT_TRUE(gc1.has_arg('E'));
    TEST_ASSERT_EQUAL_FLOAT(1.0, gc1.get_arg('X'));
    TEST_ASSERT_EQUAL_FLOAT(4.0, gc1.get_arg('E'));
}

REGISTER_TEST(GCodeTest, nist_float) {
    char *np= 0;
    const char *p= "1.2345 -54.321 1e10 0x11.23";
    float f = parse_float(p, &np);
    TEST_ASSERT_EQUAL_FLOAT(1.2345, f);
    TEST_ASSERT_TRUE(np == &p[6]);

    const char *t= np;
    f = parse_float(t, &np);
    TEST_ASSERT_EQUAL_FLOAT(-54.321, f);
    TEST_ASSERT_TRUE(np == &p[14]);

    t= np;
    f = parse_float(t, &np);
    TEST_ASSERT_EQUAL_FLOAT(1.0, f);
    TEST_ASSERT_TRUE(np == &p[16]);

    t= &p[20];
    f = parse_float(t, &np);
    TEST_ASSERT_EQUAL_FLOAT(0.0, f);
    TEST_ASSERT_TRUE(np == &p[21]);

    t= &p[22];
    f = parse_float(t, &np);
    TEST_ASSERT_EQUAL_FLOAT(11.23, f);
    TEST_ASSERT_TRUE(np == &p[27]);
}

REGISTER_TEST(GCodeTest, nist_float_limits) {
    char *np= 0;
    const char *p= "1.12345678 1.12345678999 ";
    float f = parse_float(p, &np);
    TEST_ASSERT_FLOAT_WITHIN(0.000000001, 1.12345678, f);
    TEST_ASSERT_TRUE(np == &p[10]);

    const char *t= np;
    f = parse_float(t, &np);
    TEST_ASSERT_FLOAT_WITHIN(0.000000001, 1.12345678, f);
    TEST_ASSERT_TRUE(np == &p[24]);
}
