#!/usr/bin/echo

project_script_path="${header_internal_script}"
temp_path="/tmp/gr-engine"
current_path="${PWD}"

if [ ! -d "${project_script_path}/build" ]; then
    mkdir "${project_script_path}/build"

    cd "${project_script_path}/build"

    cmake ..

    cd current_path
fi

if [ ! -f "${project_script_path}/CMakeLists.txt" ]; then
    echo "CMakeLists.txt do not found: ${project_script_path}/CMakeLists.txt"
    exit
else
    # sed -i '/#{start}/,/#{file_source_input}/d' "${project_script_path}/CMakeLists.txt"
    sed -i '/#{start}/,/#{file_source_input}/ {
            /#{start}/!{
                /#{file_source_input}/!d
            }
        }' "${project_script_path}/CMakeLists.txt"
fi

if [ ! -d "${temp_path}" ]; then
    mkdir "${temp_path}"
fi

# .cpp files
find "${PWD}" -type f -name "*.cpp" > "${temp_path}/script_source.txt"

if [ -f "${temp_path}/script_source.txt" ]; then
    while IFS= read -r line; do
        if [ -f "$line" ]; then
            cp "$line" "${project_script_path}/src/script"

            filename=$(basename "$line")

            sed -i "/#{file_source_input}/i ${project_script_path}/src/script/$filename" "${project_script_path}/CMakeLists.txt"
        fi
    done < "${temp_path}/script_source.txt"
fi

find "${PWD}" -type f -name "*.hpp" -exec cp {} "${project_script_path}/src/script/" \;

cmake --build "${project_script_path}/build"
