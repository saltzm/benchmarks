

Installation:

sudo apt-get install python3-venv
python3 -m venv pyenv
source pyenv/bin/activate
pip install conan
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
mkdir build && cd build
conan install ..
cmake .. -G "Unix Makefiles" -DCMAKE\_BUILD\_TYPE=Release
cmake --build .
