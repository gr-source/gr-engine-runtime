
/*
    Criado: seg 13 nov 2023 11∶47∶36
    Atualizado: qua 15 nov 2023 11∶23∶54
    By: ghosth-br
*/


#pragma once

#include <map>
#include <array>
#include <mutex>
#include <limits>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <future>
#include <fstream>
#include <ostream>
#include <istream>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include <GL/glew.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "Color.h"
#include "log.h"

#include "core/exception/Exception.h"

#include "types.h"

#include <gr-math/gmath.h>

/* pasta: src */
class Game;

/* pasta: src/engine */
class Mesh;
class Light;
class Sprite;
class Shader;
class Skybox;
class Terrain;
class Material;
class Animation;
class Texture2D;
class GameObject;
class ScreenBuffer;
class ScreenRender;
class BufferManager;
class ObjectManager;

/* pasta: src/engine/component */
class EditorController;
class BoxCollider;
class MeshRender;
class LineRender;
class Transform;
class Rigidbody;
class Collider;
class Animator;
class Camera;

class ObjectManager;

class ImGuiRender;

/* pasta: src/engine/ecs */
class Entity;
class EntityManager;

/* pasta: src/engine/core/scenes */
class SceneManager;
class Scene;

/* pasta: src/engine/core/wavefront*/
class Wavefront;
class Object;

class FbxScene;
class FbxNode;

/* Framebuffer */
class Renderbuffer;
class Framebuffer;

enum class FileType : int {
	NONE = 0,
	OBJ  = 1,
	MTL  = 2,
	PNG  = 3,
	JPG  = 4,
	GR   = 5,
	MT   = 6,
	GOT  = 7,
	SCN  = 8,
	FBX  = 9
};

enum class DragDrop : int {
	TEXTURE      = 0,
	GAMEOBJECT   = 1,
	SCENES_CHILD = 2,
	MATERIAL     = 3,
	NONE         = -1
};

enum class DirectoryType : int {
	NONE      = 0,
	MODEL     = 1,
	MATERIAL  = 2,
	TEXTURE   = 3,
	SHADER    = 4,
	SCENE     = 5,
	ICON      = 6,
	RESOURCES = 7
};

/* objetm o tipo de objeto pra dragdrop */
const char* getDragDrop(const DragDrop& type);

/* pega o tipo do arquivo pela estensao */
FileType getFileType(const std::string& extension);

/* pega o a externsao do arquivo pelo tipo */
const char* getFileTypeName(const FileType& type);

struct {
	bool equals(const std::string& text, const std::string& last) {
		return text == last;
	};

	bool contains(const std::string& text, const std::string& last) {
		if (text.empty() || last.empty()) return false;

		std::size_t npos = text.find(last);

		return npos != std::string::npos;
	}

	bool endWith(const std::string& text, const std::string& last) {
		if (text.empty() || last.empty()) return false;

		int len = last.length();

		std::string result = text.substr(text.length() - len, len);

		return result == last;
	}
} String;

enum class Enviropment {
	DOWNLOADS,
	DOCUMENTS
};

struct {

	bool copy(const std::string& in, const std::string& out) {
		std::fstream istream(in, std::ios::in);
		if (!istream.is_open()) {
			return false;
		}

		std::string source((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
		
		std::fstream ostream(out, std::ios::out);
		if (!(ostream << source)) {
			istream.close();
			return false;
		}
		ostream.close();

		return true;
	}

	bool move(const std::string& in, const std::string& out) {
		std::fstream istream(in, std::ios::in);
		if (!istream.is_open()) {
			return false;
		}

		std::string source((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());
		
		std::fstream ostream(out, std::ios::out);
		if (!(ostream << source)) {
			istream.close();
			return false;
		}
		ostream.close();
		
		istream.close();

		std::filesystem::remove(in);
		return true;
	}

	std::string read(const std::string& path) {
		std::fstream istream(path, std::ios::in);
		if (!istream.is_open()) {
			return "";
		}

		std::string source = std::string((std::istreambuf_iterator<char>(istream)), std::istreambuf_iterator<char>());

		istream.close();

		return source;
	}
} File;

struct GameStatics {
	u32 vertices;
	u32 indices;

	float updateMs;
	float renderMs;
	float physicsMs;
	float scriptMS;
};

extern GameStatics gameStatics;



