{
  description = "C/C++ environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, ... }@inputs:
    utils.lib.eachDefaultSystem (system:
      let
        p = import nixpkgs { 
            inherit system; 
        };
        llvm = p.llvmPackages_latest;

        mymake = p.writeShellScriptBin "mk" ''
          if [ -f "$1.c" ]; then
            i="$1.c"
            c=$CC
          else
            i="$1.cpp"
            c=$CXX
          fi
          o=$1
          shift
          $c -ggdb $i -o $o -lm -Wall $@
        '';

      in {
        devShell = p.mkShell.override { stdenv = p.clangStdenv; } rec {
          name = "C";
          packages = with p; [
            # builder
            gnumake
            cmake
            bear

            zig
            zls

            # debugger
            llvm.lldb
            gdb

            # fix headers not found
            clang-tools

            # LSP and compiler
            llvm.libstdcxxClang

            # other tools
            cppcheck
            llvm.libllvm
            valgrind
            mymake

            # stdlib for cpp
            llvm.libcxx
              
            # libs
            glm
            SDL2
            SDL2_gfx

            mosquitto
          ];

          shellHook = ''
            tmux -L sefsafs
          '';
        };
      }
    );
}
