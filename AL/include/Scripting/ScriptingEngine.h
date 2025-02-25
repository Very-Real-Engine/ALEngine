#pragma once

#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include "Core/Timestep.h"

#include <filesystem>
#include <map>
#include <string>

// C 스타일의 name mangling을 사용하지 않도록 지시하는 키워드
// name mangling - 컴파일러가 변수나 함수의 고유 식별자를 생성하기 위해 사용되는 프로세스
// MyNamespace::MyClass::myFunction(int) → _ZN11MyNamespace7MyClass10myFunctionEi

extern "C"
{
	typedef struct _MonoType MonoType;
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
	typedef struct _MonoString MonoString;
}

namespace ale
{
enum class EScriptFieldType
{
	NONE = 0,
	FLOAT,
	DOUBLE,
	BOOL,
	CHAR,
	BYTE,
	SHORT,
	INT,
	LONG,
	UBYTE,
	USHORT,
	UINT,
	ULONG,
	VECTOR2,
	VECTOR3,
	VECTOR4,
	ENTITY,
	STRING
};

struct ScriptField
{
	EScriptFieldType m_Type;
	std::string m_Name;

	MonoClassField *m_ClassField;
};

// App Assembly로부터 파싱해온 Class
class ScriptClass
{
  public:
	ScriptClass() = default;
	ScriptClass(const std::string &classNamespace, const std::string &className, bool isCore = false);

	MonoObject *instantiate();
	MonoMethod *getMethod(const std::string &name, int parameterCount);
	MonoObject *invokeMethod(MonoObject *instance, MonoMethod *method, void **params = nullptr);
	std::vector<std::string> getAllMethods();

	const std::map<std::string, ScriptField> &getFields() const
	{
		return m_Fields;
	}

  private:
	std::string m_ClassNamespace;
	std::string m_ClassName;

	std::map<std::string, ScriptField> m_Fields;

	MonoClass *m_MonoClass = nullptr;
	friend class ScriptingEngine;
};

class ScriptInstance
{
  public:
	ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, Entity entity);

	void invokeOnCreate();
	void invokeOnUpdate(float ts);
	std::map<std::string, std::function<bool()>> getAllBooleanMethods();

	std::shared_ptr<ScriptClass> getScriptClass()
	{
		return m_ScriptClass;
	}

	MonoObject *getManagedObject()
	{
		return m_Instance;
	}

	template <typename T> T getFieldValue(const std::string &name)
	{
		static_assert(sizeof(T) <= 16, "Type too large!");

		bool success = getFieldValueInternal(name, s_FieldValueBuffer);
		if (!success)
			return T();

		return *(T *)s_FieldValueBuffer;
	}

	template <typename T> void setFieldValue(const std::string &name, T value)
	{
		static_assert(sizeof(T) <= 16, "Type too large!");

		setFieldValueInternal(name, &value);
	}

  private:
	bool getFieldValueInternal(const std::string &name, void *buffer);
	bool setFieldValueInternal(const std::string &name, const void *value);

  private:
	std::shared_ptr<ScriptClass> m_ScriptClass;

	MonoObject *m_Instance = nullptr;
	MonoMethod *m_Constructor = nullptr;
	MonoMethod *m_OnCreateMethod = nullptr;
	MonoMethod *m_OnUpdateMethod = nullptr;

	inline static char s_FieldValueBuffer[16];

	friend class ScriptingEngine;
	friend class ScriptFieldInstance;
};

struct ScriptFieldInstance
{
	ScriptField m_Field;

	ScriptFieldInstance()
	{
		memset(m_Buffer, 0, sizeof(m_Buffer));
	}

	template <typename T> T getValue()
	{
		static_assert(sizeof(T) <= 16, "Type too large!");
		return *(T *)m_Buffer;
	}

	template <typename T> void setValue(T value)
	{
		static_assert(sizeof(T) <= 16, "Type too large!");
		memcpy(m_Buffer, &value, sizeof(T));
	}

  private:
	uint8_t m_Buffer[16];

	friend class ScriptingEngine;
	friend class ScriptInstance;
};

using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

class ScriptingEngine
{
  public:
	static void init();
	static void shutDown();

	static bool loadAssembly(const std::filesystem::path &filepath);
	static bool loadAppAssembly(const std::filesystem::path &filepath);

	static void reloadAssembly();

	static void onRuntimeStart(Scene *scene);
	static void onRuntimeStop();

	static bool entityClassExists(const std::string &fullClassName);
	static void onCreateEntity(Entity entity);
	static void onUpdateEntity(Entity entity, Timestep ts);
	static std::map<std::string, std::function<bool()>> getBooleanMethods(Entity entity);

	static Scene *getSceneContext();
	static MonoImage *getCoreAssemblyImage();
	static MonoObject *getManagedInstance(UUID uuid);
	// static MonoObject *createManagedComponentInstance(Entity entity, MonoType *monoType);

	static std::shared_ptr<ScriptInstance> getEntityScriptInstance(UUID entityID);
	static std::shared_ptr<ScriptClass> getEntityClass(const std::string &name);
	static std::unordered_map<std::string, std::shared_ptr<ScriptClass>> getEntityClasses();
	static ScriptFieldMap &getScriptFieldMap(Entity entity);

  private:
	static void initMono();
	static void shutDownMono();

	static MonoObject *instantiateClass(MonoClass *monoClass);
	static void loadAssemblyClasses();

	friend class ScriptClass;
	friend class ScriptGlue;
};

namespace utils
{

std::string monoStringToString2(MonoString *string);
MonoString* stringToMonoString(std::string& string);

inline const char *scriptFieldTypeToString(EScriptFieldType fieldType)
{
	switch (fieldType)
	{
	case EScriptFieldType::NONE:
		return "None";
	case EScriptFieldType::FLOAT:
		return "Float";
	case EScriptFieldType::DOUBLE:
		return "Double";
	case EScriptFieldType::BOOL:
		return "Bool";
	case EScriptFieldType::CHAR:
		return "Char";
	case EScriptFieldType::BYTE:
		return "Byte";
	case EScriptFieldType::SHORT:
		return "Short";
	case EScriptFieldType::INT:
		return "Int";
	case EScriptFieldType::LONG:
		return "Long";
	case EScriptFieldType::UBYTE:
		return "UByte";
	case EScriptFieldType::USHORT:
		return "UShort";
	case EScriptFieldType::UINT:
		return "UInt";
	case EScriptFieldType::ULONG:
		return "ULong";
	case EScriptFieldType::VECTOR2:
		return "Vector2";
	case EScriptFieldType::VECTOR3:
		return "Vector3";
	case EScriptFieldType::VECTOR4:
		return "Vector4";
	case EScriptFieldType::ENTITY:
		return "Entity";
	case EScriptFieldType::STRING:
		return "String";
	}
	return "None";
}

inline EScriptFieldType scriptFieldTypeFromString(std::string_view fieldType)
{
	if (fieldType == "None")
		return EScriptFieldType::NONE;
	if (fieldType == "Float")
		return EScriptFieldType::FLOAT;
	if (fieldType == "Double")
		return EScriptFieldType::DOUBLE;
	if (fieldType == "Bool")
		return EScriptFieldType::BOOL;
	if (fieldType == "Char")
		return EScriptFieldType::CHAR;
	if (fieldType == "Byte")
		return EScriptFieldType::BYTE;
	if (fieldType == "Short")
		return EScriptFieldType::SHORT;
	if (fieldType == "Int")
		return EScriptFieldType::INT;
	if (fieldType == "Long")
		return EScriptFieldType::LONG;
	if (fieldType == "UByte")
		return EScriptFieldType::UBYTE;
	if (fieldType == "UShort")
		return EScriptFieldType::USHORT;
	if (fieldType == "UInt")
		return EScriptFieldType::UINT;
	if (fieldType == "ULong")
		return EScriptFieldType::ULONG;
	if (fieldType == "Vector2")
		return EScriptFieldType::VECTOR2;
	if (fieldType == "Vector3")
		return EScriptFieldType::VECTOR3;
	if (fieldType == "Vector4")
		return EScriptFieldType::VECTOR4;
	if (fieldType == "Entity")
		return EScriptFieldType::ENTITY;
	if (fieldType == "String")
		return EScriptFieldType::STRING;

	return EScriptFieldType::NONE;
}

} // namespace utils
} // namespace ale

// namespace std
// {
// // UUID + MonoType* 조합을 위한 해시 함수 특수화
// template <> struct hash<std::pair<ale::UUID, MonoType *>>
// {
// 	std::size_t operator()(const std::pair<ale::UUID, MonoType *> &pair) const noexcept
// 	{
// 		std::size_t h1 = std::hash<ale::UUID>{}(pair.first);
// 		std::size_t h2 = std::hash<MonoType *>{}(pair.second);
// 		return h1 ^ (h2 << 1); // 비트 연산을 사용하여 해시 값 결합
// 	}
// };
// } // namespace std