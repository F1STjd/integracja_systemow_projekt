# Steam Price Comparison - Docker Setup

This application compares Steam game prices across different regions with affordability analysis.

## Prerequisites

- Docker
- Docker Compose (optional, for easier deployment)

## Building and Running with Docker

### Option 1: Using Docker Compose (Recommended)

```bash
# Build and run
docker-compose up --build

# Run in background
docker-compose up -d --build

# Stop the application
docker-compose down
```

### Option 2: Using Docker directly

```bash
# Build the image
docker build -t steam-price-app .

# Run the container
docker run -p 8080:8080 --name steam-price-comparison steam-price-app

# Run in background
docker run -d -p 8080:8080 --name steam-price-comparison steam-price-app
```

## Accessing the Application

Once running, open your browser and navigate to:
- http://localhost:8080

## Features

- Search for Steam games by name
- Compare prices across different regions
- View affordability analysis based on GDP per capita
- Interactive charts showing price comparisons
- Responsive web interface

## API Endpoints

- `GET /` - Main search page
- `GET /search?query=<game_name>` - Search for games
- `GET /prices?appid=<steam_app_id>` - Get price comparison for a specific game
- `GET /styles.css` - Application styles

## Technical Stack

- **Backend**: C++26 with httplib
- **Database**: SQLite3
- **JSON**: Glaze library
- **Frontend**: HTML5, CSS3, JavaScript (Chart.js, DataTables)
- **Build System**: CMake + Ninja

## Container Details

- **Base Image**: Ubuntu 24.04
- **Compiler**: GCC-14 with C++26 support
- **Port**: 8080
- **Health Check**: Included
- **Resource Limits**: 512MB RAM, 1 CPU (configurable)

## Troubleshooting

### If the container fails to start:

1. Check if port 8080 is available:
   ```bash
   netstat -tulpn | grep 8080
   ```

2. View container logs:
   ```bash
   docker logs steam-price-comparison
   ```

3. Check container status:
   ```bash
   docker ps -a
   ```

### If the database is not found:

The SQLite database should be automatically copied during the container startup. If you encounter database-related errors, ensure the `src/data.sqlite` file exists in your project directory.

## Development

To modify the application:

1. Make changes to the source code
2. Rebuild the Docker image:
   ```bash
   docker-compose up --build
   ```

The Dockerfile uses layer caching to speed up rebuilds when only source code changes.
