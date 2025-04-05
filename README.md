# Chess Engine
This project was created as a diploma thesis for the completion of my Bachelor's degree in Computer Science.

---

## Build Instructions

### Requirements:
- CMake ≥ 3.10
- C++17-compatible compiler (GCC / Clang / MSVC)
- (Optional) Ninja or Make

---
### Build steps:

```bash
# 1. Clone the repository
git clone https://github.com/FMI-OpenSource-Lab/chess-engine.git
cd chess-engine

# 2. Create a build directory
mkdir build
cd build

# 3. Configure the project (defaults to Release mode)
cmake ..

# 4. Build (Release) default
cmake --build .

# 4.1 Build (Debug)
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# 5. After building
./Engine
