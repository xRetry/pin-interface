{ 
    description = "";

    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
        flake-utils.url = "github:numtide/flake-utils";
    };

    outputs = { nixpkgs, flake-utils, ...}:
        flake-utils.lib.eachDefaultSystem (system:
        let 
            pkgs = import nixpkgs { inherit system; };
        in rec {
            devShell = (pkgs.buildFHSUserEnv {
                name = "esp";
                targetPkgs = pkgs: (with pkgs; [
                    (pkgs.callPackage ./esp32_toolchain.nix {})
                    git
                    wget
                    gnumake
                    flex
                    bison
                    gperf
                    pkg-config
                    cmake
                    ncurses5
                    ninja

                    (python310.withPackages (p: with p; [
                        pip
                        #virtualenv
                    ]))
                ]);
                profile = ''
                    export IDF_PATH=$HOME/lib/esp-idf
                    export PATH=$IDF_PATH/tools:$PATH
                    export IDF_PYTHON_ENV_PATH=$(pwd)/.venv

                    #sh $IDF_PATH/install.sh

                    if [ ! -e $IDF_PYTHON_ENV_PATH ]; then
                      python -m venv $IDF_PYTHON_ENV_PATH
                      . $IDF_PYTHON_ENV_PATH/bin/activate
                      sh $IDF_PATH/install.sh
                    else
                      . $IDF_PYTHON_ENV_PATH/bin/activate
                    fi
                '';
                runScript = "bash";
            }).env;
        });
}
