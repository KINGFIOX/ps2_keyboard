{
  description = "Chisel Nix";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-parts = {
      url = "github:hercules-ci/flake-parts";
      inputs.nixpkgs-lib.follows = "nixpkgs";
    };
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    zaozi.url = "github:sequencer/zaozi";
    mill-ivy-fetcher.url = "github:Avimitin/mill-ivy-fetcher";
  };

  outputs =
    inputs@{
      self,
      nixpkgs,
      flake-parts,
      ...
    }:
    let
      overlay = import ./nix/overlay.nix;
    in
    flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "aarch64-darwin"
      ];

      imports = [
        inputs.treefmt-nix.flakeModule
      ];

      flake.overlays.default = overlay;

      perSystem =
        { system, pkgs, ... }:
        {
          _module.args.pkgs = import nixpkgs {
            inherit system;
            # TODO: Do not depend on overlay of zaozi in favor of importing its outputs explicitly to avoid namespace pollution.
            overlays = with inputs; [
              zaozi.overlays.default
              mill-ivy-fetcher.overlays.default
              overlay
            ];
          };

          legacyPackages = pkgs;

          treefmt = {
            projectRootFile = "flake.nix";
            programs.scalafmt = {
              enable = true;
              includes = [ "*.mill" ];
            };
            programs.nixfmt = {
              enable = true;
              excludes = [ "*/generated.nix" ];
            };
          };

          devShells.default =
            with pkgs;
            mkShell ({
              inputsFrom = [
                dependencies.ivy-chisel
                dependencies.ivy-omlib
              ];
              nativeBuildInputs = [
                verilator
                meson
                ninja
                pkg-config
                python3
              ];
              buildInputs = [
                SDL2
                SDL2_image
                SDL2_ttf
              ];
              packages = [
                mill
                metals
                nixd
                nvfetcher
                clang-tools
              ];
              CIRCT_INSTALL_PATH = circt-install;
              MLIR_INSTALL_PATH = mlir-install;
              JEXTRACT_INSTALL_PATH = jextract-21;
              NVBOARD_HOME = "/home/wangfiox/Documents/ps2_keyboard/subprojects/nvboard/";
              JAVA_TOOL_OPTIONS = "-Divy.home=${dependencies.ivyLocalRepo} -Dcoursier.ivy.home=${dependencies.ivyLocalRepo}";
            });
        };
    };
}
