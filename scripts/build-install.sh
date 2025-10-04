sudo dnf install -y qt5-qtbase-devel qt5-qtsvg-devel qt5-qttools-devel \
                   opencv-devel mpv-devel \
                   cmake ninja-build gcc-c++ git
cd ~/repos/qimgv
rm -rf build
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DOPENCV_SUPPORT=ON \
  -DVIDEO_SUPPORT=ON
ninja -C build
cd build
sudo ninja install
