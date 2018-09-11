# kinect2 rgb depth
Using the kinect 2.0 to acquire rgb and depth frame
# required
- **Release x64**
- opencv3(x64)
- kinect sdk 2.0
- include dir:
` $(KINECTSDK20_DIR)\inc;F:\opencv\build\include\opencv2;F:\opencv\build\include\;$(IncludePath)`
- lib dir:
  `$(KINECTSDK20_DIR)\lib\x64;F:\opencv\build\x64\vc15\lib;$(LibraryPath)`
- linker-input:
`opencv_world343.lib;kinect20.lib;kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib;%(AdditionalDependencies)`
