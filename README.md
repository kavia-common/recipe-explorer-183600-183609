# recipe-explorer-183600-183609

This workspace contains a native Qt6 recipe application in the `recipe_app_native` folder.

Quick start:
- cd recipe_app_native
- cmake -S . -B build
- cmake --build build
- cmake --build build --target run

The app provides:
- Home, Search, Recipe Detail, and Favorites screens
- Ocean Professional theme (blue primary with clean, minimalist UI)
- Mock dataset and local persistence for favorites (QSettings)