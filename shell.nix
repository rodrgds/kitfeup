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

  '';

  runScript = "bash";
}).env
