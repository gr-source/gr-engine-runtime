#!/usr/bin/echo

current_path=${PWD}

# ===== Check Project Exists =====
if ! [ -d "${GR_PROJECT_PATH}" ]; then
    echo "Project do not exist."
    exit 1
fi

if ! [ -d "${GR_PROJECT_PATH}/script" ]; then
    mkdir "${GR_PROJECT_PATH}/script"
fi

# ===== Check Cmake exist and clear files in cmake =====
echo "Clearing data..."
if [ ! -f "${GR_PROJECT_PATH}/CMakeLists.txt" ]; then
    echo "CMakeLists.txt do not found: ${GR_PROJECT_PATH}/CMakeLists.txt"
    exit 2
else
    sed -i '/#{GR_START}/,/#{GR_END}/ {
            /#{GR_START}/!{
                /#{GR_END}/!d
            }
        }' "${GR_PROJECT_PATH}/CMakeLists.txt"
fi

# ===== Create temp file =====
find "${GR_ROOT_PROJECT_PATH}" -type f -name "*.cpp" > "script_source.txt"
find "${GR_ROOT_PROJECT_PATH}" -type f -name "*.hpp" > "script_header.txt"

# ===== Read temp file and copy from project =====
if [ -f "script_source.txt" ]; then
    while IFS= read -r line; do
        if [ -f "$line" ]; then
            cp "$line" "${GR_PROJECT_PATH}/script"

            filename=$(basename "$line")

            sed -i "/#{GR_END}/i ${GR_PROJECT_PATH}/script/$filename" "${GR_PROJECT_PATH}/CMakeLists.txt"
        fi
    done < "script_source.txt"
fi

if [ -f "script_header.txt" ]; then
    while IFS= read -r line; do
        if [ -f "$line" ]; then
            cp "$line" "${GR_PROJECT_PATH}/script"
        fi
    done < "script_header.txt"
fi

# clean twmp files
rm script_source.txt
rm script_header.txt

if ! [ -d "${GR_PROJECT_PATH}/build" ]; then
    echo "Initialize configure project..."

    mkdir "${GR_PROJECT_PATH}/build"

    cd "${GR_PROJECT_PATH}/build"

    if cmake .. > /dev/null 2>&1; then
        echo "Project configured."
    else
        echo "Falied to configure project."
        exit 3
    fi

    cd "${current_path}"
fi

echo "Initialize build project..."
if cmake --build "${GR_PROJECT_PATH}/build" > /dev/null 2>&1; then
    echo "Build project sucess full."
else
    echo "Falied to build project."
    exit 2
fi
