# WebAssembly-Drawing-Tool

This is a C++ drawing tool developed to be used in the browser using WebAssembly. If you want to host it locally, please follow the instructions below. Otherwise, feel free to check it out [here](https://gdhillon2.github.io/WASM-Draw/).

## Prerequisites

Before running WASM-Draw, you need to install the following tools:

- [Emscripten Compiler](https://emscripten.org/docs/getting_started/downloads.html): This tool will allow you to compile C++ code into WebAssembly.

## Setup

1. Clone the repository or download the source files.

2. Install Emscripten by following the [installation guide](https://emscripten.org/docs/getting_started/downloads.html).

3. Compile the C++ code using the provided Makefile:
```
make
```

4. Host a local server to serve the files:
```
python3 -m http.server
```

5. Open a browser and navigate to the appropriate localhost port, typically `http://localhost:8000`.

## Using WASM-Draw

Once the server is running, you use the tool directly in your browser.

## Troubleshooting

- If the webpage isn't loading or you're facing issues, make sure that you have properly set up the environment and compiled WASM-Draw.
- If the webpage doesn't render or interact as expected, check the browser console for errors.
