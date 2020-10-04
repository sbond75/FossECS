{}:

with import <nixpkgs> {};

stdenv.mkDerivation rec {
  #Bengine = callPackage ./bengine.nix { };
  #CEGUI_ = callPackage ./cegui.nix { };
  name = "FossECS";
  src = ./.;
  nativeLibc = true;
  buildInputs = [ clang libGL glew glm SDL2 SDL2_mixer SDL2_ttf #CEGUI_
                  llvmPackages.libstdcxxClang ];
  patchPhase = ''
    
  ''; # this modifies files in a pure environment (effectively copies the files to modify them. so this re-runs each time!)

  #preConfigure = "LD=$CC";
  
  buildPhase = ''
    ./build.sh
  '';
}
