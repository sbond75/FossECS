{}:

with import <nixpkgs> {};

stdenv.mkDerivation rec {
  #Bengine = callPackage ./bengine.nix { };
  #CEGUI_ = callPackage ./cegui.nix { };
  name = "FossECS";
  src = ./.;
  #nativeLibc = true;
  buildInputs = [ libGL glew glm SDL2 SDL2_mixer SDL2_ttf #CEGUI_
                  llvmPackages.libstdcxxClang
                  ginac # https://www.ginac.de/tutorial/#A-comparison-with-other-CAS
                  #glib
                ];
  patchPhase = ''
    
  ''; # this modifies files in a pure environment (effectively copies the files to modify them. so this re-runs each time!)

  #preConfigure = "LD=$CC";

  #NIX_CFLAGS_COMPILE = [ "-lm" ];

  nativeBuildInputs = [ clang pkgconfig ];
  
  # propagatedBuildInputs = stdenv.lib.optionals stdenv.isDarwin
  #   [ darwin.apple_sdk.frameworks.Cocoa ];
  
  # buildPhase = ''
  #   make -j4
  # '';
}

  # Phases: unpack phase, generic build (cmake project then builds that) -- magic. But there's also fixups: patch phase, post phase, etc. (see nix manual)
