# sbasmCpp - C++ Assembler for qCore

## Overview
This project serves as a replacement for the current Python-based assembler used with the qCore architecture. It is designed for educational purposes at TGM.

The assembler is written in **C++14**.

---

## Installation
### Option 1: Install via Prebuilt Installer (Windows)
1. Download **`sbasmCpp-0.1.1-win64.exe`**.
2. Run the installer.
3. By default, the assembler will be installed under:
   ```
   C:\Program Files\sbasmCpp 0.1.1
   ```
   (You can specify a different installation path if desired.)

### Option 2: Build from Source
#### Prerequisites
Ensure you have the following installed:
- **CMake** & **Make**
- **Clang** or **GCC** (for Linux/macOS)
- **Visual Studio** (for Windows)

#### Clone the Repository
You'll need **Git** installed on your machine.

1. Open a terminal/command prompt.
2. Navigate to your preferred directory (e.g., `Documents`):
   ```sh
   cd ~/Documents  # or any other preferred location
   ```
3. Clone the repository:
   ```sh
   git clone https://github.com/landmaschine/sbasmCpp.git
   ```

#### Building on Linux/macOS
```sh
mkdir build
cd build
cmake ..
make
```

#### Building on Windows
1. Open a terminal and run:
   ```sh
   mkdir build
   cd build
   ```
2. In the `build` directory, a `.sln` file will be generated.
3. Open the `.sln` file in **Visual Studio**.
4. Click **Build** in Visual Studio.
5. The compiled `.exe` will be located in the `build` folder.

---

# Setting Up Global Access to the Assembler on Windows 

To use the assembler from any command prompt on Windows:

1. Open Environment Variables by pressing the Windows key and typing `environment variables`
2. Select "Edit the system environment variables"
3. In the System Properties window, click the "Environment Variables..." button
4. Under "User variables," find and select the "Path" variable, then click "Edit"
5. Click "New" and add the assembler's installation directory: `C:\Program Files\sbasmCpp\bin`
6. Click "OK" on all open windows to save your changes

You can now run the assembler from any command prompt without specifying its full path.

## Usage
```sh
# Basic assembly (produces a.mif by default)
./sbasmCpp input_file.s

# Specify output file
./sbasmCpp input_file.s -o output.mif

# Enable verbose output
./sbasmCpp input_file.s -v

# Combine options
./sbasmCpp input_file.s -o output.mif -v

# Display help
./sbasmCpp --help
```
---

## License
Copyright (c) 2025 Leon Wessely

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
associated documentation files (the "Software"), to deal in the Software without restriction, including 
without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to 
the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial 
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN 
NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
