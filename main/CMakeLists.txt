idf_component_register(SRCS "main.c" "wifi.c" "http.c" "console.c"
	INCLUDE_DIRS "."
	EMBED_TXTFILES web/index.html
	EMBED_TXTFILES web/style.css)