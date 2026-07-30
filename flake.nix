{
    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    };

    outputs = { self, nixpkgs, ... }:
    let
        system = "x86_64-linux";
        pkgs = import nixpkgs { inherit system; };

        debug = pkgs.writeShellScriptBin "debug" "cmake --workflow --preset debug && ./build/snake";
        release = pkgs.writeShellScriptBin "release" "cmake --workflow --preset release && ./build/release/snake";
        san = pkgs.writeShellScriptBin "san" "cmake --workflow --preset san && LSAN_OPTIONS=detect_leaks=0 ./build/san/snake";
        tsan = pkgs.writeShellScriptBin "tsan" "cmake --workflow --preset tsan && TSAN_OPTIONS=suppressions=/dev/null ./build/tsan/snake";
    in {
        devShells."${system}".default = pkgs.mkShell.override {
            stdenv = pkgs.clang19Stdenv;
        } {
            name = "snake";

            packages = with pkgs; [
                clang-tools
                cmake
                ninja
                mold
                ccache

                shaderc
                vulkan-validation-layers

                gdb
                valgrind
                renderdoc
                perf

                debug
                release
                san
                tsan
            ];

            LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath [
                vulkan-loader
                sdl3
            ];
        };
    };
}

