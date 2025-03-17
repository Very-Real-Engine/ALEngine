#pragma once

#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include "Core/Timestep.h"

#include <filesystem>
#include <map>
#include <string>

#include "mono/metadata/mono-gc.h"

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
/**
 * @enum EScriptFieldType
 * @brief 스크립트 필드의 데이터 유형을 정의하는 열거형.
 */
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

/**
 * @struct ScriptField
 * @brief 개별 스크립트 필드 정보를 저장하는 구조체.
 */
struct ScriptField
{
	EScriptFieldType m_Type;
	std::string m_Name;

	MonoClassField *m_ClassField;
};

/**
 * @class ScriptClass
 * @brief App Assembly로부터 파싱해온 Class. C# 스크립트 클래스를 나타내며, 앱 어셈블리에서 로드된 클래스 정보를 관리.
 */
class ScriptClass
{
  public:
	/// @brief ScriptClass 기본 생성자.
	ScriptClass() = default;

	/// @brief ScriptClass 생성자.
	/// @param classNamespace C# 스크립트의 namespace.
	/// @param className C# 스크립트의 Class 이름.
	/// @param isCore Core Assembly인지 확인하는 불리언 변수.
	ScriptClass(const std::string &classNamespace, const std::string &className, bool isCore = false);

	/// @brief ScriptClass를 Mono 구조에 맞게 생성.
	/// @return MonoObject* 
	MonoObject *instantiate();

	/// @brief Mono Script Class의 함수 가져오기.
	/// @param name 함수 이름.
	/// @param parameterCount 함수의 매개변수 개수.
	/// @return MonoMethod*
	MonoMethod *getMethod(const std::string &name, int parameterCount);

	/// @brief MonoMethod를 런타임에 invoke하는 함수.
	/// @param instance Mono Script Class
	/// @param method Mono Method
	/// @param params 인자 포인터
	/// @return MonoObject*
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

/**
 * @class ScriptInstance
 * @brief 개별 엔티티의 C# 스크립트 인스턴스를 나타내는 클래스.
 */
class ScriptInstance
{
  public:
	/// @brief ScriptInstance 생성자.
	/// @param scriptClass 기존에 생성한 ScriptClass 
	/// @param entity ScriptComponent를 가지고 있는 Entity.
	ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, Entity entity);
	~ScriptInstance();

	/// @brief ScriptInstance의 OnCreate 함수 실행.
	void invokeOnCreate();

	/// @brief ScriptInstance의 OnUpdate 함수 실행.
	/// @param ts 프레임 간격
	void invokeOnUpdate(float ts);
	std::map<std::string, std::function<bool()>> getAllBooleanMethods();

	std::shared_ptr<ScriptClass> getScriptClass()
	{
		return m_ScriptClass;
	}

	MonoObject *getManagedObject()
	{
		return mono_gchandle_get_target(m_GCHandle);
	}

	/// @brief ScriptInstance의 변수 값을 가져오는 템플릿 함수.
	/// @tparam T 변수 type.
	/// @param name 변수 이름.
	/// @return 변수 type.
	template <typename T> T getFieldValue(const std::string &name)
	{
		static_assert(sizeof(T) <= 16, "Type too large!");

		bool success = getFieldValueInternal(name, s_FieldValueBuffer);
		if (!success)
			return T();

		return *(T *)s_FieldValueBuffer;
	}

	/// @brief ScriptInstance의 변수 값 설정하는 템플릿 함수.
	/// @tparam T 변수 type.
	/// @param name 변수 이름.
	/// @param value 변수 값.
	template <typename T> void setFieldValue(const std::string &name, T value)
	{
		static_assert(sizeof(T) <= 16, "Type too large!");

		setFieldValueInternal(name, &value);
	}

	template<>
	void setFieldValue(const std::string &name, MonoString* value)
	{
		setFieldValueInternal(name, value);
	}

  private:
	bool getFieldValueInternal(const std::string &name, void *buffer);
	bool setFieldValueInternal(const std::string &name, const void *value);

  private:
	std::shared_ptr<ScriptClass> m_ScriptClass;
	unsigned int m_GCHandle = 0;

	MonoObject *m_Instance = nullptr;
	MonoMethod *m_Constructor = nullptr;
	MonoMethod *m_OnCreateMethod = nullptr;
	MonoMethod *m_OnUpdateMethod = nullptr;

	inline static char s_FieldValueBuffer[16];

	friend class ScriptingEngine;
	friend class ScriptFieldInstance;
};

/**
 * @struct ScriptFieldInstance
 * @brief 특정 엔티티의 개별 스크립트 필드 데이터를 저장하는 구조체.
 */
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
	template<>
	std::string getValue<std::string>()
	{
		return *reinterpret_cast<std::string*>(m_Buffer);
	}

	template <typename T> void setValue(T value)
	{
		static_assert(sizeof(T) <= 16, "Type too large!");
		memcpy(m_Buffer, &value, sizeof(T));
	}
	template <>
	void setValue<std::string>(std::string value)
	{
		*reinterpret_cast<std::string*>(m_Buffer) = std::move(value);
	}
	

  private:
	uint8_t m_Buffer[16];

	friend class ScriptingEngine;
	friend class ScriptInstance;
};

using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

/**
 * @class ScriptingEngine
 * @brief C# 스크립트와 상호작용하는 Mono 런타임을 관리하는 클래스.
 */
class ScriptingEngine
{
  public:
	/// @brief ScriptingEngine 초기화.
	static void init();

	/// @brief ScriptingEngine 종료.
	static void shutDown();

	/// @brief ScriptCore를 읽어 저장하는 함수.
	/// @param filepath ScriptCore file 경로.
	/// @return true 읽는데 성공.
	/// @return false 읽는데 실패.
	static bool loadAssembly(const std::filesystem::path &filepath);

	/// @brief ScriptModule를 읽어 저장하는 함수.
	/// @param filepath ScriptModule file 경로.
	/// @return true 읽는데 성공.
	/// @return false 읽는데 실패.
	static bool loadAppAssembly(const std::filesystem::path &filepath);

	static void reloadAssembly();

	/// @brief ScriptEngine을 Runtime에 시작하는 함수.
	/// @param scene 현재 Scene의 정보.
	static void onRuntimeStart(Scene *scene);

	/// @brief ScriptEngine을 Runtime에 끝내는 함수.
	static void onRuntimeStop();

	/// @brief Entity Class가 존재하는 확인하는 함수.
	/// @param fullClassName Entity Class 이름.
	/// @return true Entity Class가 존재함.
	/// @return false Entity Class가 존재하지 않음.
	static bool entityClassExists(const std::string &fullClassName);

	/// @brief Entity의 ScriptComponent 정보를 읽어와 OnCreate를 실행하는 함수.
	/// @param entity Entity.
	static void onCreateEntity(Entity entity);

	/// @brief Entity의 ScriptComponent 정보를 읽어와 OnUpdate를 실행하는 함수.
	/// @param entity Entity.
	/// @param ts 프레임간 간격. 
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
/**
 * @brief MonoString* 형식을 std::string으로 변환합니다.
 * @param MonoString*
 * @return std::string
 */
std::string monoStringToString2(MonoString *string);
 /**
 * @brief std::string 형식을 MonoString* 으로 변환합니다.
 * @param std::string
 * @return MonoString*
 */
MonoString* stringToMonoString(const std::string& string);

/**
 * @brief 스크립트 필드 유형을 문자열로 변환합니다.
 * @param fieldType 변환할 필드 유형.
 * @return const char* 변환된 문자열.
 */
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

/// @brief 문자열을 스크립트 필드 열거형으로 변환.
/// @param fieldType 필드 타입 문자열.
/// @return 스크립트 필드 열거형.
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