#define NOB_IMPLEMENTATION
#include "nob.h"

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);
    Nob_Cmd cmd = {0};
    char* raylib_path = "./external/raylib-5.5/src/";
    nob_cmd_append(
        &cmd,
        "cc",
        "main.c",
        "-I",
        raylib_path,
        "-L",
        raylib_path,
        "-lraylib",
        "-lGL",
        "-lm",
        "-lpthread",
        "-ldl",
        "-lrt",
        "-lX11"
    );
    if (!nob_cmd_run_sync(cmd)) return 1;
    return 0;
}
