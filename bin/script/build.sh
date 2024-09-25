#!/usr/bin/bash

# Save current path
current_path=${PWD}

temp_file_source="script_source.txt"
temp_file_header="script_header.txt"

# verification of project existence
if ! [ -d "${GR_ROOT_PROJECT_PATH}" ]; then
    echo "Project do not exist."
    exit 1
fi

if ! [ -d "${GR_ROOT_ASSETS_PATH}" ]; then
    echo "Assets data do not exist."
    exit 2
fi

# verification of scripts existece
if ! [ -d "${GR_ROOT_PROJECT_PATH}/script" ]; then
    echo "Script data do not found."
    exit 3
fi

# ===== Check Cmake exist and clear files in cmake =====
echo "Clearing data..."
if [ ! -f "${GR_ROOT_PROJECT_PATH}/script/CMakeLists.txt" ]; then
    echo "CMakeLists.txt do not found: ${GR_ROOT_PROJECT_PATH}/script/CMakeLists.txt"
    exit 4
else
    # clean source files
    sed -i '/#{GR_SRC_START}/,/#{GR_SRC_END}/ {
            /#{GR_SRC_START}/!{
                /#{GR_SRC_END}/!d
            }
        }' "${GR_ROOT_PROJECT_PATH}/script/CMakeLists.txt"
    
    # clear header files
    sed -i '/#{GR_HDR_START}/,/#{GR_HDR_END}/ {
            /#{GR_HDR_START}/!{
                /#{GR_HDR_END}/!d
            }
        }' "${GR_ROOT_PROJECT_PATH}/script/CMakeLists.txt"
fi

# ===== Create temp file =====
find "${GR_ROOT_ASSETS_PATH}" -type f -name "*.cpp" > ${temp_file_source}
find "${GR_ROOT_ASSETS_PATH}" -type f -name "*.hpp" > ${temp_file_header}

# ===== Read temp file and copy from project =====
if [ -f ${temp_file_source} ]; then
    while IFS= read -r line; do
        if [ -f "$line" ]; then
            cp "$line" "${GR_ROOT_PROJECT_PATH}/script"

            filename=$(basename "$line")

            sed -i "/#{GR_SRC_END}/i ${GR_ROOT_PROJECT_PATH}/script/$filename" "${GR_ROOT_PROJECT_PATH}/script/CMakeLists.txt"
        fi
    done < ${temp_file_source}
fi

# ===== Read temp file and copy from project =====
if [ -f ${temp_file_header} ]; then
    while IFS= read -r line; do
        if [ -f "$line" ]; then
            cp "$line" "${GR_ROOT_PROJECT_PATH}/script"

            filename=$(basename "$line")

            sed -i "/#{GR_HDR_END}/i ${GR_ROOT_PROJECT_PATH}/script/$filename" "${GR_ROOT_PROJECT_PATH}/script/CMakeLists.txt"
        fi
    done < ${temp_file_header}
fi

# If the project is not configured, configure it
if ! [ -d "${GR_ROOT_PROJECT_PATH}/build" ]; then
    echo "configuring project..."

    mkdir "${GR_ROOT_PROJECT_PATH}/build"

    cd "${GR_ROOT_PROJECT_PATH}/build"

    cmake ..
    
    cd "${current_path}"
fi

# # building project
cmake --build "${GR_ROOT_PROJECT_PATH}/build"
