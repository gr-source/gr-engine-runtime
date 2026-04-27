#!/usr/bin/bash

last_path=${PWD}

if ! [ -d "${DENGINE_COMPONENT_PATH}" ]; then
    echo "Component director do not found: ${DENGINE_COMPONENT_PATH}"
    exit 1
fi

if [ -d  "${DENGINE_COMPONENT_PATH}/build" ]; then
    rm -rf  "${DENGINE_COMPONENT_PATH}/build"
    echo "Build cleanread..."
fi

if [ -d  "${DENGINE_COMPONENT_PATH}/lib" ]; then
    rm -rf  "${DENGINE_COMPONENT_PATH}/lib"
fi

echo "Initializing cmake..."

mkdir "${DENGINE_COMPONENT_PATH}/build"

cd "${DENGINE_COMPONENT_PATH}/build"

if ! cmake .. > /dev/null 2>&1; then
    echo "CMake do not condigured."
    
    cd "${last_path}"

    exit 2
fi

cd "${last_path}"

echo "Building CMake..."

if cmake --build "${DENGINE_COMPONENT_PATH}/build" > /dev/null 2>&1; then
    echo "CMake build sucessfull."
else
    echo "CMake build error."
    exit 3
fi




