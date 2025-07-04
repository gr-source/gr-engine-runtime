#!/bin/bash

declare -A scriptpriority

# TODO: adicionar o .hpp no cmake e inutil, precisa ser removido?

if ! [ -d "$1" ]; then
	echo "Use: $0 \"/dir/my project\""
	exit 1
fi

# TODO: Precisa fazer um sistema para encontrar CMakeLists.txt
if ! [ -f "$1/.build/component/CMakeLists.txt" ]; then
	echo "CMake do not found."
	exit 2
fi

echo "Reading project: $1"

find "$1" -type d -path "$1/.build" -prune -o -type f -name "*.cpp" -exec sha1sum {} \; > "$1/.build/source.txt"
find "$1" -type d -path "$1/.build" -prune -o -type f -name "*.hpp" -exec sha1sum {} \; > "$1/.build/header.txt"

if ! [ -f "$1/.build/source.txt" ]; then
	echo "Fail fo read source file."
	exit 2
fi

if ! [ -f "$1/.build/header.txt" ]; then
	echo "Fail ti read header file."
	exit 3
fi

if [ -f "$1/.build/sourcex.txt" ]; then
	diff -b "$1/.build/source.txt" "$1/.build/sourcex.txt" > /dev/null
	if [ $? -ne 0 ]; then
		diff -b "$1/.build/source.txt" "$1/.build/sourcex.txt" | grep '^<' | sed 's/^< //' > "$1/.build/source_add.txt"
		diff -b "$1/.build/source.txt" "$1/.build/sourcex.txt" | grep '^>' | sed 's/^> //' > "$1/.build/source_remove.txt"
	fi
else
	cp "$1/.build/source.txt" "$1/.build/source_add.txt"
fi

if [ -f "$1/.build/headerx.txt" ]; then
	diff -d "$1/.build/header.txt" "$1/.build/headerx.txt" > /dev/null
	if [ $? -ne 0 ]; then
		diff -b "$1/.build/header.txt" "$1/.build/headerx.txt" | grep '^<' | sed 's/^< //' > "$1/.build/header_add.txt"
		diff -b "$1/.build/header.txt" "$1/.build/headerx.txt" | grep '^>' | sed 's/^> //' > "$1/.build/header_remove.txt"
	fi
else
	cp "$1/.build/header.txt" "$1/.build/header_add.txt"
fi

if [ -f "$1/.build/source_remove.txt" ]; then
	while IFS=' ' read -r hash file; do
		filename=$(basename "$file")
		if [ -f "$1/.build/component/source/$filename" ]; then			
			echo "Remove script: $filename"
			rm "$1/.build/component/source/$filename"
		fi
	done < "$1/.build/source_remove.txt"
	rm "$1/.build/source_remove.txt"
fi

if [ -f "$1/.build/header_remove.txt" ]; then
	while IFS=' ' read -r hash file; do
		filename=$(basename "$file")
		if [ -f "$1/.build/component/source/$filename" ]; then
			rm "$1/.build/component/source/$filename"
			echo "Remove script: $filename"
		fi
	done < "$1/.build/header_remove.txt"
	rm "$1/.build/header_remove.txt"
fi

# Adiciona novos scripts(cpp)
# TODO: Esta remomento todos os scripts antes já adicionado, e adiciona o que foi atualizad.
if [ -f "$1/.build/source_add.txt" ]; then
	echo "Update cmake sources."

	# sed -i '/#{GR_SRC_START}/,/#{GR_SRC_END}/ {
    #        /#{GR_SRC_START}/!{
    #            /#{GR_SRC_END}/!d
    #        }
    #    }' "$1/.build/component/CMakeLists.txt"

	while IFS=' ' read -r hash file; do
		filename=$(basename "$file")
		if ! [ -f "$file" ]; then
			continue;
		fi
		if  [ -f "$1/.build/component/source/$filename" ]; then
			echo "Update file: $filename"
		else
			echo "Adding file: $filename"
		fi

		cp "$file"  "$1/.build/component/source"

		cat "$1/.build/component/CMakeLists.txt" | grep "source/$filename" > /dev/null
		if [ $? -ne 0 ]; then
			sed -i "/#{GR_SRC_END}/i source/$filename" "$1/.build/component/CMakeLists.txt"
		fi

	done < "$1/.build/source_add.txt"

	rm "$1/.build/source_add.txt"
fi

# Adiciona os novos scripts(hpp)
if [ -f "$1/.build/header_add.txt" ]; then
	echo "Update cmake header."
	while IFS=' ' read -r hash file; do
		filename=$(basename "$file")
		if ! [ -f "$file" ]; then
			continue;
		fi
		if  [ -f "$1/.build/component/source/$filename" ]; then
			echo "Update file: $filename"
		else
			echo "Adding file: $filename"
		fi
		cp "$file" "$1/.build/component/source"
	done < "$1/.build/header_add.txt"
	rm "$1/.build/header_add.txt"
fi

cp "$1/.build/source.txt" "$1/.build/sourcex.txt"
cp "$1/.build/header.txt" "$1/.build/headerx.txt"


sed -i '/\/\*/,/\*\// {
    /\//!d
}' "$1/.build/source.txt"

while IFS=' ' read -r hash file; do
    while IFS= read -r line; do
        line="${line%%//*}"

        if [[ "$line" =~ (getComponent|GetComponent)[[:space:]]*\<[[:space:]]*([a-zA-Z0-9_]+)[[:space:]]*\> ]]; then
            component_name="${BASH_REMATCH[2]}"

            ((scriptpriority["$component_name"]++))
        fi
    done < "$file"

done < "$1/.build/source.txt"

if [ -f "$1/.build/priority.txt" ]; then
    rm "$1/.build/priority.txt"
fi

for component in "${!scriptpriority[@]}"; do
    echo "$component ${scriptpriority[$component]}" >> "$1/.build/priority.txt"
done

last_path=$(pwd)

if ! [ -d "$1/.build/component/build" ]; then
	echo "Configuring project..."

	mkdir "$1/.build/component/build"

	cd "$1/.build/component/build"

	cmake -DCMAKE_BUILD_TYPE=Debug .. >  /dev/null
	if [ $? -ne 0 ]; then
		echo "Fail to configure project."
		exit 4
	fi
fi

cd "$last_path"

echo "Building project..."

cmake --build "$1/.build/component/build" -j4 > /dev/null
if [ $? -ne 0 ]; then
	echo "Fail to build project."
	exit 5
fi

echo "Project build successfully."



