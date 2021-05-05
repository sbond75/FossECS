make -j4
ASAN_OPTIONS=detect_stack_use_after_return=1 ./ecs
