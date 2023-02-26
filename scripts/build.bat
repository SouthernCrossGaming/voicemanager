rmdir .\build /s /q
docker compose build
docker compose run extension-build --remove-orphans