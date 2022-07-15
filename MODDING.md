## MODDING

*IMPORTANT*
If you want to mod, download the source.zip or source.tar.gz file from the github page

To change a map, edit map01.txt in the "Maps" folder.
To change textures 3DSage has a good guide to .ppm exporting. Export to TexAtlas.ppm and monsterTex.ppm
To change the music, create a midi file, rename it to D_BKROOM.mid and place in the Sounds folder.

When you want the changes to apply, recompile "backrooms.cpp" using MinGW32 G++. Midi changes do not require a recompile

Make sure you have FluidSynth added to the includes! Use the winXP x86 version.

Compile command:

    <MinGW32 g++ path here> .\src\backrooms.cpp -o .\bin\backrooms.exe -m32 -pthread -mwindows -I"<minGW32 include path here>" -I"<Fluidsynth include path here>" -L"<minGW32 lib path here>" -L"<Fluidsynth lib path here>" -lfreeglut -lopengl32 -lglu32 -lglut32 -lwinmm -lfluidsynth

	