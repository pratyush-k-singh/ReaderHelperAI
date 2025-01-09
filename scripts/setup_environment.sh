#!/bin/bash

# Exit on error
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Setting up Book Recommender environment...${NC}\n"

# Check for required environment variables
if [ -z "$GROQ_API_KEY" ]; then
    echo -e "${YELLOW}Warning: GROQ_API_KEY is not set${NC}"
    echo "Please set your Groq API key with:"
    echo "export GROQ_API_KEY=your_api_key_here"
fi

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check and install system dependencies
echo -e "\n${GREEN}Checking system dependencies...${NC}"

# List of required packages
PACKAGES=(
    "build-essential"
    "cmake"
    "libfaiss-dev"
    "libcpprest-dev"
    "libspdlog-dev"
    "nlohmann-json3-dev"
    "libssl-dev"
    "libomp-dev"
)

# Check package manager
if command_exists apt-get; then
    echo "Detected Debian/Ubuntu system"
    
    # Update package list
    echo "Updating package list..."
    sudo apt-get update

    # Install packages
    for pkg in "${PACKAGES[@]}"; do
        if ! dpkg -l | grep -q "^ii  $pkg "; then
            echo "Installing $pkg..."
            sudo apt-get install -y "$pkg"
        else
            echo "$pkg is already installed"
        fi
    done
elif command_exists brew; then
    echo "Detected macOS system"
    
    # Install packages using Homebrew
    brew update
    brew install faiss
    brew install cpprestsdk
    brew install spdlog
    brew install nlohmann-json
    brew install openssl
    brew install libomp
else
    echo -e "${RED}Error: Unsupported package manager${NC}"
    echo "Please install the following packages manually:"
    printf '%s\n' "${PACKAGES[@]}"
    exit 1
fi

# Create data directories
echo -e "\n${GREEN}Setting up directory structure...${NC}"
mkdir -p data/raw
mkdir -p data/processed

# Check if books.csv exists
if [ ! -f "data/raw/books.csv" ]; then
    echo -e "${YELLOW}Warning: books.csv not found in data/raw/${NC}"
    echo "Please add your book dataset to data/raw/books.csv"
fi

# Setup build directory
echo -e "\n${GREEN}Setting up build directory...${NC}"
mkdir -p build
cd build

# Configure project
echo -e "\n${GREEN}Configuring project...${NC}"
cmake ..

# Build project
echo -e "\n${GREEN}Building project...${NC}"
make -j$(nproc)

# Run tests
echo -e "\n${GREEN}Running tests...${NC}"
ctest --output-on-failure

echo -e "\n${GREEN}Environment setup complete!${NC}"
echo "You can now use the Book Recommender system."
echo -e "Remember to set your ${YELLOW}GROQ_API_KEY${NC} if you haven't already."
