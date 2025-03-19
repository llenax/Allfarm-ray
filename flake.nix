{
  description = "Raylib development environment";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    devenv.url = "github:cachix/devenv";
  };

  outputs = { self , nixpkgs, devenv,... } @inputs:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
    };
  in {
    devShells."${system}".default = devenv.lib.mkShell {
      inherit inputs pkgs;
      modules = [
        {
          packages = [
            pkgs.libGL

            # X11 dependencies
            pkgs.xorg.libX11
            pkgs.xorg.libX11.dev
            pkgs.xorg.libXcursor
            pkgs.xorg.libXi
            pkgs.xorg.libXinerama
            pkgs.xorg.libXrandr

            # Uncomment the line below if you want to build Raylib with web support
            # pkgs.emscripten
          ];
        }
      ];
    };
  };
}
