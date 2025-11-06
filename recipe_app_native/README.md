# Recipe Explorer (Native Qt6)

A native recipe application demonstrating browsing, searching, and managing favorite recipes. 
Uses a modern "Ocean Professional" theme with clean aesthetics, rounded corners, subtle shadows, and smooth interactions.

## Features
- Home: browse a mock dataset of recipes.
- Search: filter by recipe title or ingredient with a responsive results list.
- Recipe Detail: view full info and toggle favorite.
- Favorites: view and manage saved recipes.
- Local persistence: favorites saved using QSettings (no external APIs).
- Theming: Ocean Professional color palette and modern minimalist style.

## Ocean Professional Theme
- Primary: #2563EB
- Secondary: #F59E0B
- Success: #F59E0B
- Error: #EF4444
- Background: #f9fafb
- Surface: #ffffff
- Text: #111827

## Requirements
- Qt6 (Core, Widgets)
- CMake >= 3.16
- C++17 compiler

## Build and Run
```bash
# From the container root
cd recipe_app_native
cmake -S . -B build
cmake --build build
# Run (option 1): via CMake custom target
cmake --build build --target run

# Run (option 2): using helper script (recommended if preview tooling is unavailable)
chmod +x run.sh
./run.sh
```

Troubleshooting:
- If you see an error like `sudo: /usr/local/bin/start_vnc: command not found` during preview start,
  it means the environment lacks VNC preview tooling. Use the `run.sh` helper to build and launch
  the app directly without VNC.

This will start the application with the Home screen. Use the top-right navigation to switch between:
- Home
- Search
- Favorites

Select a recipe's "Open" to view details, and toggle the star to favorite/unfavorite. Favorites are persisted across runs.

## Project Structure
- CMakeLists.txt — build config for Qt6 Widgets app
- include/mainApp.h — declarations for Theme, RecipeStore, screens, and MainWindow
- src/mainApp.cpp — implementation (mock data, navigation, local storage, UI logic)
- README.md — this guide

## Notes
- The app uses stub/mock data only; no network calls or environment variables are required.
- QSettings stores favorite IDs under organization "RecipeExplorer" and app "RecipeApp".

Additional notes:
- If automated preview attempts to start a VNC server and fails with a message like:
  `sudo: /usr/local/bin/start_vnc: command not found`
  use the provided scripts to launch the app directly:
  - ./run.sh (interactive usage)
  - ./entrypoint.sh (for container entrypoint or CI wrappers)
