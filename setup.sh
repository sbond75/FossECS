# macOS and Linux

# 1. Install Nix
# 2. Run:
nix-shell -p libGL glew glm SDL2 SDL2_mixer SDL2_ttf
#NVM: nix-env -iA nixpkgs.libGL nixpkgs.glew nixpkgs.glm nixpkgs.SDL2 nixpkgs.SDL2_mixer nixpkgs.SDL2_ttf
make -j4

# Windows
