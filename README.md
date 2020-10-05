This project is based off of a game engine called [Bengine](https://github.com/Barnold1953/GraphicsTutorials) by Benjamin Arnold.

# Main project goals

- Analyze the basic concepts behind an Entity Component System (ECS).
- Create a high-performance ECS for Bengine.

# Compile and run

## macOS and Linux

- To compile, install [Nix](https://nixos.org/download.html), then run `nix-shell` in the project root to enter a shell with the dependencies loaded, and then run `make -j4` to build or rebuild changed cpp files.
- From the project root, use `./fossECS` to run the game.

## Windows

Todo

# Background info

- Bengine was created in [MakingGamesWithBen](https://www.youtube.com/user/makinggameswithben), a YouTube channel, as a [tutorial series](https://www.youtube.com/watch?v=FxCC9Ces1Yg&list=PLSPw4ASQYyymu3PfG9gxywSPghnSMiOAW) for learning C++ and low-level game and engine development with modern OpenGL. 
  - It has a number of low-level features that make it useful to experiment with and still keep track of the inner workings:
	- 2D graphics (with [OpenGL](https://www.opengl.org/) and [GLEW](http://glew.sourceforge.net/) and modern (high performance) techniques such as sprite batches and shaders)
	- Window creation & keyboard and mouse input (with [SDL](https://www.libsdl.org/))
	- Texture loading
	- 2D audio (with [SDL Mixer](https://www.libsdl.org/projects/SDL_mixer/))
	- Font rendering (with [SDL TTF](https://www.libsdl.org/projects/SDL_ttf/))
	- GUI (currently able to use [CEGUI](http://cegui.org.uk/) but this project aims to replace it)
	- Rigid-body physics (with [Box2D](https://box2d.org/))
	- Particle systems

# Secondary project goals

- Make Bengine more complete, possibly adding 3D graphics and hot reload with [jet-live](https://www.reddit.com/r/gamedev/comments/amojyy/c_hot_code_reload_for_linux_and_macos/) (see [GitHub](https://github.com/ddovod/jet-live) and [demo video](https://www.youtube.com/watch?v=5xfgViYchqg)) for Linux and macOS, and [blink](https://github.com/crosire/blink) for Windows. (Both of these claim to not require restructuring of the project to work, which is why they are chosen here.)
- Replace the GUI system with a more lightweight system than CEGUI.
- Notably lacking in Bengine's features are:
  - An editor
  - An object model such as an ECS
  - AI systems
