make:
	emcc main.cpp -s WASM=1 -s USE_SDL=2 -O3 -s ASYNCIFY -s \
	NO_EXIT_RUNTIME=1 -s EXPORTED_RUNTIME_METHODS=['ccall'] \
	-s EMBIND_STD_STRING_IS_UTF8=0 \
	-lembind \
	--shell-file html_template/template.html -o index.html
clean:
	rm index.js index.wasm index.html
