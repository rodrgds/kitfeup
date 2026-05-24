{
  pkgs ? import <nixpkgs> { },
}:

(pkgs.buildFHSEnv {
  name = "milkv-build-env";
  targetPkgs =
    pkgs: with pkgs; [
      git
      cmake
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
      openssh
      openssl.dev
      libxcrypt
      glibc.dev
      dtc
      gawk
      curl
      mtools
      perl
      ubootTools
    ];

  multiPkgs = pkgs: with pkgs; [ zlib ];

  profile = ''
    if [ -d "$PWD/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin" ]; then
      export PATH="$PWD/duo-buildroot-sdk-v2/host-tools/gcc/riscv64-linux-musl-x86_64/bin:$PATH"
    fi

    if command -v python3 >/dev/null 2>&1; then
      kitfeup_python_wrapper_dir="$HOME/.cache/kitfeup/nix-bin"
      if [ -n "$XDG_CACHE_HOME" ]; then
        kitfeup_python_wrapper_dir="$XDG_CACHE_HOME/kitfeup/nix-bin"
      fi
      mkdir -p "$kitfeup_python_wrapper_dir"
      ln -sf "$(command -v python3)" "$kitfeup_python_wrapper_dir/python"
      export PATH="$kitfeup_python_wrapper_dir:$PATH"
    fi

  '';

  runScript = "bash";
}).env
