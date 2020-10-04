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
    #linkedStuff=-lSDL2 -lSDL2_mixer -lSDL2_ttf -lCEGUIBase-0 -lCEGUICommonDialogs-0 -lCEGUIOgreRenderer-0 -lCEGUIOpenGLRenderer-0 -lGL -lGLEW -lSDL2main -lm -lc
                       # -lstdc++fs

    #compile into object files and executable#    
    #mkdir "$out"
    #c++ -o "$out/" ./*.cpp -idirafter./deps/include/ -std=c++17 $NIX_CFLAGS_COMPILE
    # #
    
    #combine Bengine's `.o` object files into a single `.a` archive with `ar`.
    #$AR_FOR_TARGET rcs "$out/Bengine.a" "$out/*.o"

    #export CXXFLAGS="-Wno-argument-outside-range -isysroot \"$(xcrun --show-sdk-path)\" -isysroot \"$(xcrun --show-sdk-path)/usr/include\""

    export CXXFLAGS="$NIX_CFLAGS_COMPILE -std=c++11"
    export LDFLAGS="$NIX_LDFLAGS"
    export CC=/usr/bin/clang; make
    #main.o Bullet.o MainGame.o
  '';
}
