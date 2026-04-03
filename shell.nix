{
  pkgs ? import <nixpkgs> { },
}:

(pkgs.buildFHSEnv {
  name = "milkv-build-env";
  targetPkgs =
    pkgs: with pkgs; [
      git
      wget
      gnumake
      gcc
      unzip
      file
      bc
      rsync
      cpio
      parted
      gawk
      python3
      python3Packages.jinja2
      meson
      ninja
      ncurses
      pkg-config
      sshpass
      flex
      bison
      openssl
      openssl.dev
      libxcrypt
      glibc.dev
      dtc
      parted
      gawk
      sshpass
      mtools
      ubootTools
    ];

  multiPkgs = pkgs: with pkgs; [ zlib ];

  profile = ''
    if [ -d "$PWD/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin" ]; then
      export PATH="$PWD/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin:$PATH"
    fi

        compile() {
          riscv64-unknown-linux-musl-gcc "$@"
        }
        export -f compile

        my-compile() {
          if [ -z "$1" ]; then
            echo "Usage: my-compile <input-file.c> [output-file]"
            return 1
          fi

          local INPUT=$1
          local OUTPUT=''${2:-"''${INPUT%.c}_riscv"}

          echo "Compiling $INPUT -> $OUTPUT..."
          
          riscv64-unknown-linux-musl-gcc \
              -I patched-umdp/umdp/ \
              -I patched-umdp/libumdp/include/ \
              "$INPUT" \
              -L patched-umdp/libumdp/build/ \
              -L sysroot/lib/ \
              -lumdp -lnl-genl-3 -lnl-3 \
              -o "$OUTPUT"

          if [ $? -eq 0 ]; then
            echo "Success!"
          else
            echo "Compilation failed."
          fi
        }
        export -f my-compile
  '';

  runScript = "bash";
}).env
