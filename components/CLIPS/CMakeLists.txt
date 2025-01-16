idf_component_register(SRC_DIRS "."
                    INCLUDE_DIRS "include"
                    REQUIRES linux)

target_compile_options(${COMPONENT_LIB} PRIVATE -DBOARD_HAS_PSRAM -DCONFIG_COMPILER_OPTIMIZATION_ASSERTIONS_SILENT=0 -DDEVELOPER -DLINUX -std=c++11 -O0 -g -Wno-unused-variable -Wall -Wundef -Wpointer-arith -Wshadow -Wstrict-aliasing -Winline -Wredundant-decls -Waggregate-return )