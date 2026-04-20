#include "unity.h"
#include "svg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_svg_create_and_close(void) {
    Svg svg = svg_create();
    TEST_ASSERT_NOT_NULL(svg);
    
    // Testa escrita num arquivo na area de teste
    svg_render_and_close(svg, NULL, NULL, "test_output.svg");
    
    FILE* f = fopen("test_output.svg", "r");
    TEST_ASSERT_NOT_NULL(f);
    if(f) {
        char buffer[256];
        fgets(buffer, sizeof(buffer), f);
        TEST_ASSERT_NOT_NULL(strstr(buffer, "<svg"));
        fclose(f);
    }
}

void test_svg_overlays(void) {
    Svg svg = svg_create();
    svg_add_overlay(svg, "<circle cx=\"10\" cy=\"10\" r=\"5\"/>");
    
    svg_render_and_close(svg, NULL, NULL, "test_output.svg");
    
    FILE* f = fopen("test_output.svg", "r");
    TEST_ASSERT_NOT_NULL(f);
    if(f) {
        char buffer[1024];
        int matches = 0;
        while(fgets(buffer, sizeof(buffer), f)) {
            if (strstr(buffer, "<circle cx=\"10\" cy=\"10\" r=\"5\"/>")) matches++;
        }
        fclose(f);
        TEST_ASSERT_EQUAL(1, matches);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_svg_create_and_close);
    RUN_TEST(test_svg_overlays);
    return UNITY_END();
}
