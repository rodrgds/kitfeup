{ pkgs ? import <nixpkgs> {} }:

(pkgs.buildFHSEnv {
  name = "milkv-build-env";
  targetPkgs = pkgs: with pkgs; [
    # Basic build tools
    git
    wget
    gnumake
    gcc
    unzip
    file
    bc
    rsync
    cpio
    
    # Python & dependencies for the Milk-V scripts
    python3
    python3Packages.jinja2
    
    # Kernel config tools
    ncurses
    pkg-config
  ];
  
  # Ensure 32-bit/64-bit library compatibility for pre-built toolchains
  multiPkgs = pkgs: with pkgs; [ zlib ];
  
  runScript = "bash";
}).env
