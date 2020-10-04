# export CXXFLAGS=-I\ /Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include\ -I\ /Volumes/MyTestVolume/Xcode11.5.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/11.0.3/include\ -isystem\ /Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/
#     export CXXFLAGS="$CXXFLAGS -Wno-argument-outside-range -isysroot \"/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk\" -isysroot \"/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk\" -isystem /Volumes/MyTestVolume/Xcode11.5.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/../include/c++/v1"
#     make



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
#echo "$NIX_CFLAGS_COMPILE"
#export LDFLAGS="$NIX_LDFLAGS"
# -lgl -lSDL2 -lSDL2_ttf -lglew
export CC=/usr/bin/clang++; make
#main.o Bullet.o MainGame.o
