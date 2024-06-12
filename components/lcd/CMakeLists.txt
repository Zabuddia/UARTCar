idf_component_register(SRCS "glcdfont.c" "lcd.c" "lcd_test.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver)
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
