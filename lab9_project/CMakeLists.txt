add_executable(lab9.elf main.c board.c gameControl.c graphics.c pill.c)
target_link_libraries(lab9.elf ${330_LIBS} intervalTimer interrupts buttons_switches)
set_target_properties(lab9.elf PROPERTIES LINKER_LANGUAGE CXX)