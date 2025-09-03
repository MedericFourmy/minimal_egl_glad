A minimal example how headless rendering using [EGL](https://registry.khronos.org/EGL/sdk/docs/man/)+[GLAD](https://glad.dav1d.de/).

## Dependencies
Install implementation of EGL/OpenGL:  
`sudo apt-get install libegl1-mesa-dev`

## Build and run:
```
mkdir build; cd build;
cmake ..
make
./main
```

A `triangle.bmp` file should appear in `build`.