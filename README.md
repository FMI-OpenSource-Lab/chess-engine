# KhaosChess
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

## 3.1 Release mode (default)
cmake ..
## 3.2 Debug mode
cmake -DCMAKE_BUILD_TYPE=Debug ..

# 4. Build
cmake --build .

# 5. After building
./KhaosChess
```
---
### Visualization:

You can use KhaosChess with a graphical chess interface (GUI) like [Arena Chess GUI](https://www.playwitharena.de/)

#### How to integrate with Arena GUI (Windows example):

1. Open Arena GUI.
2. Go to **Engines → Install New Engine**.
3. Navigate to your compiled `KhaosChess.exe` binary and select it.
4. Arena will prompt you for engine settings (you can leave most as default).
5. The engine will now appear in your list and can be used to play, test or run engine-vs-engine matches.

**Note**: The engine accepts the **UCI (Universal Chess Interface)** protocol