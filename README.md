# Porównanie Cen Steam - Konfiguracja Docker

Ta aplikacja porównuje ceny gier Steam w różnych regionach z analizą przystępności cenowej.

## Wymagania

- Docker
- Docker Compose (opcjonalnie, dla łatwiejszego wdrożenia)

## Budowanie i Uruchamianie z Docker

### Opcja 1: Używanie Docker Compose (Zalecane)

```bash
# Buduj i uruchom
docker-compose up --build

# Uruchom w tle
docker-compose up -d --build

# Zatrzymaj aplikację
docker-compose down
```

### Opcja 2: Używanie Docker bezpośrednio

```bash
# Zbuduj obraz
docker build -t steam-price-app .

# Uruchom kontener
docker run -p 8080:8080 --name steam-price-comparison steam-price-app

# Uruchom w tle
docker run -d -p 8080:8080 --name steam-price-comparison steam-price-app
```

## Dostęp do Aplikacji

Po uruchomieniu otwórz przeglądarkę i przejdź do:
- http://localhost:8080

## Funkcje

- Wyszukiwanie gier Steam po nazwie
- Porównywanie cen w różnych regionach
- Podgląd analizy przystępności opartej na PKB per capita
- Interaktywne wykresy pokazujące porównania cen
- Responsywny interfejs internetowy

## Punkty końcowe API

- `GET /` - Główna strona wyszukiwania
- `GET /search?query=<nazwa_gry>` - Wyszukiwanie gier
- `GET /prices?appid=<steam_app_id>` - Pobierz porównanie cen dla konkretnej gry
- `GET /styles.css` - Style aplikacji

## Stos Technologiczny

- **Backend**: C++26 z httplib
- **Baza danych**: SQLite3
- **JSON**: Biblioteka Glaze
- **Frontend**: HTML5, CSS3, JavaScript (Chart.js, DataTables)
- **System budowania**: CMake + Ninja

## Szczegóły Kontenera

- **Obraz bazowy**: Ubuntu 24.04
- **Kompilator**: GCC-14 z obsługą C++26
- **Port**: 8080
- **Sprawdzanie zdrowia**: Włączone
- **Limity zasobów**: 512MB RAM, 1 CPU (konfigurowalne)

## Rozwiązywanie problemów

### Jeśli kontener nie może się uruchomić:

1. Sprawdź czy port 8080 jest dostępny:
   ```bash
   netstat -tulpn | grep 8080
   ```

2. Wyświetl logi kontenera:
   ```bash
   docker logs steam-price-comparison
   ```

3. Sprawdź status kontenera:
   ```bash
   docker ps -a
   ```

### Jeśli baza danych nie została znaleziona:

Baza danych SQLite powinna być automatycznie skopiowana podczas uruchamiania kontenera. Jeśli napotkasz błędy związane z bazą danych, upewnij się, że plik `src/data.sqlite` istnieje w katalogu projektu.

## Rozwój

Aby zmodyfikować aplikację:

1. Wprowadź zmiany w kodzie źródłowym
2. Przebuduj obraz Docker:
   ```bash
   docker-compose up --build
   ```

Dockerfile używa buforowania warstw, aby przyspieszyć ponowne budowanie gdy zmieniają się tylko pliki źródłowe.
