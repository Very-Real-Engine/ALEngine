#include "Scripting/ScriptingEngine.h"
#include "Scripting/ScriptingGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/object.h"
#include "mono/metadata/threads.h"

#include "Core/FileSystem.h"

#include "Project/Project.h"

namespace ale
{

static std::unordered_map<std::string, EScriptFieldType> s_ScriptFieldTypeMap = {

	{"System.Single", EScriptFieldType::FLOAT},		 {"System.Double", EScriptFieldType::DOUBLE},
	{"System.Boolean", EScriptFieldType::BOOL},		 {"System.Char", EScriptFieldType::CHAR},
	{"System.Int16", EScriptFieldType::SHORT},		 {"System.Int32", EScriptFieldType::INT},
	{"System.Int64", EScriptFieldType::LONG},		 {"System.Byte", EScriptFieldType::BYTE},
	{"System.UInt16", EScriptFieldType::USHORT},	 {"System.UInt32", EScriptFieldType::UINT},
	{"System.UInt64", EScriptFieldType::ULONG},		 {"System.String", EScriptFieldType::STRING},

	{"ALEngine.Vector2", EScriptFieldType::VECTOR2}, {"ALEngine.Vector3", EScriptFieldType::VECTOR3},
	{"ALEngine.Vector4", EScriptFieldType::VECTOR4},

	{"ALEngine.Entity", EScriptFieldType::ENTITY},
};

namespace utils
{

static MonoAssembly *loadMonoAssembly(const std::filesystem::path &assemblyPath, bool loadPDB = false)
{
	ScopedBuffer fileData = FileSystem::readFileBinary(assemblyPath);

	MonoImageOpenStatus status;

	// Mono Runtime 중 메모리에 로드된 데이터를 이용해 Assembly Image Open
	// need_copy - Mono가 data를 복사해야 하는지
	// ref_only - Image가 참조 전용으로 열릴지

	// AL_CORE_TRACE("{0}", fileData.size());
	MonoImage *image = mono_image_open_from_data_full(fileData.as<char>(), fileData.size(), 1, &status, 0);

	if (status != MONO_IMAGE_OK)
	{
		const char *errMsg = mono_image_strerror(status);
		AL_CORE_ERROR("{}", errMsg);
		return nullptr;
	}
	if (loadPDB)
	{
		std::filesystem::path pdbPath = assemblyPath;
		pdbPath.replace_extension(".pdb");

		if (std::filesystem::exists(pdbPath))
		{
			ScopedBuffer pdbFileData = FileSystem::readFileBinary(pdbPath);
			mono_debug_open_image_from_memory(image, pdbFileData.as<const mono_byte>(), pdbFileData.size());
			// AL_CORE_INFO("Loaded PDB {}", pdbPath);
		}
	}

	std::string pathString = assemblyPath.string();
	MonoAssembly *assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
	mono_image_close(image);

	return assembly;
}

std::string monoStringToString2(MonoString *string)
{
	char *cStr = mono_string_to_utf8(string);
	std::string str(cStr);
	mono_free(cStr);
	return str;
}

MonoString* stringToMonoString(const std::string& string)
{
	MonoDomain* domain = mono_domain_get();
	return mono_string_new(domain, string.c_str());
}

// Assembly에 대한 정보를 출력하는 함수. Namespace, Function name등을 알 수 있음.
void printAssemblyType(MonoAssembly *assembly)
{
	MonoImage *image = mono_assembly_get_image(assembly);
	const MonoTableInfo *typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
	int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

	for (int32_t i = 0; i < numTypes; i++)
	{
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

		// Assembly 내부에 저장된 string data를 가져올 때 사용
		const char *nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
		const char *name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		AL_CORE_TRACE("{}.{}", nameSpace, name);
	}
}

EScriptFieldType monoTypeToScriptFieldType(MonoType *monoType)
{
	std::string typeName = mono_type_get_name(monoType);

	auto it = s_ScriptFieldTypeMap.find(typeName);
	if (it == s_ScriptFieldTypeMap.end())
	{
		AL_CORE_ERROR("Unknown type: {}", typeName);
		return EScriptFieldType::NONE;
	}

	return it->second;
}

} // namespace utils

struct ScriptingEngineData
{
	// MonoDomain - 프로세스 내에 실행되는 어플리케이션들을 격리시키고, 각 작업을 독립적으로 실행하는 구조체.
	MonoDomain *rootDomain = nullptr;
	MonoDomain *appDomain = nullptr;

	MonoAssembly *coreAssembly = nullptr;
	MonoImage *coreAssemblyImage = nullptr;

	MonoAssembly *appAssembly = nullptr;
	MonoImage *appAssemblyImage = nullptr;

	std::filesystem::path coreAssemblyFilepath;
	std::filesystem::path appAssemblyFilepath;

	ScriptClass entityClass;
	std::unordered_map<std::string, std::shared_ptr<ScriptClass>> entityClasses;
	std::unordered_map<UUID, std::shared_ptr<ScriptInstance>> entityInstances;
	std::unordered_map<UUID, ScriptFieldMap> entityScriptFields;

	// std::unordered_map<std::pair<UUID, MonoType *>, MonoObject *> s_ComponentInstances;

	// filewatch?
	bool assemblyReloadPending = false;

	bool enableDebugging = false; // debugging option - ifdef로 구분 가능.
	Scene *sceneContext = nullptr;
};

static ScriptingEngineData *s_Data = nullptr;

void ScriptingEngine::init()
{
	s_Data = new ScriptingEngineData();

	initMono();
	ScriptGlue::registerFunctions();

	auto scriptCorePath = Project::getAssetDirectory() / Project::getActive()->getConfig().m_ScriptCorePath;
	bool status = loadAssembly(scriptCorePath);
	if (!status)
	{
		AL_CORE_ERROR("[ScriptEngine] Could not load AL-ScriptCore assembly!");
		return;
	}

	// get script module path
	auto scriptModulePath = Project::getAssetDirectory() / Project::getActive()->getConfig().m_ScriptModulePath;
	status = loadAppAssembly(scriptModulePath);
	if (!status)
	{
		AL_CORE_ERROR("[ScriptEngine] Could not load app assembly!");
		return;
	}

	loadAssemblyClasses();

	ScriptGlue::registerComponents();

	s_Data->entityClass = ScriptClass("ALEngine", "Entity", true);
}

void ScriptingEngine::shutDown()
{
	shutDownMono();
	delete s_Data;
}

void ScriptingEngine::initMono()
{
	mono_set_assemblies_path("Sandbox/mono/lib");

	if (s_Data->enableDebugging)
	{
		const char *argv[2] = {"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,"
							   "loglevel=3,logfile=MonoDebugger.log",
							   "--soft-breakpoints"};

		mono_jit_parse_options(2, (char **)argv);
		mono_debug_init(MONO_DEBUG_FORMAT_MONO);
	}

	// Mono runtime 초기화
	MonoDomain *rootDomain = mono_jit_init("ALEJITRuntime"); // Manual delete 필요.

	// ASSERT로 대체.
	if (rootDomain == nullptr)
	{
		AL_CORE_ERROR("RootDomain doesn't exist!");
		return;
	}
	s_Data->rootDomain = rootDomain;

	if (s_Data->enableDebugging)
	{
		mono_debug_domain_create(s_Data->rootDomain);
	}
	mono_thread_set_main(mono_thread_current());
}

void ScriptingEngine::shutDownMono()
{
	mono_domain_set(mono_get_root_domain(), false);

	mono_domain_unload(s_Data->appDomain);
	s_Data->appDomain = nullptr;

	mono_jit_cleanup(s_Data->rootDomain);
	s_Data->rootDomain = nullptr;
}

bool ScriptingEngine::loadAssembly(const std::filesystem::path &filepath)
{
	// app domain create
	s_Data->appDomain = mono_domain_create_appdomain("ALEScriptRuntime", nullptr);
	mono_domain_set(s_Data->appDomain, true);

	s_Data->coreAssemblyFilepath = filepath;
	s_Data->coreAssembly = utils::loadMonoAssembly(filepath, s_Data->enableDebugging);

	if (s_Data->coreAssembly == nullptr)
	{
		return false;
	}

	s_Data->coreAssemblyImage = mono_assembly_get_image(s_Data->coreAssembly);
	return true;
}

bool ScriptingEngine::loadAppAssembly(const std::filesystem::path &filepath)
{
	s_Data->appAssemblyFilepath = filepath;
	s_Data->appAssembly = utils::loadMonoAssembly(filepath, s_Data->enableDebugging);
	if (s_Data->appAssembly == nullptr)
	{
		return false;
	}

	s_Data->appAssemblyImage = mono_assembly_get_image(s_Data->appAssembly);

	// filewatcher
	s_Data->assemblyReloadPending = false;
	return true;
}

void ScriptingEngine::reloadAssembly()
{
}

void ScriptingEngine::onRuntimeStart(Scene *scene)
{
	s_Data->sceneContext = scene;
}

void ScriptingEngine::onRuntimeStop()
{
	s_Data->sceneContext = nullptr;

	s_Data->entityInstances.clear();
}

bool ScriptingEngine::entityClassExists(const std::string &fullClassName)
{
	return s_Data->entityClasses.find(fullClassName) != s_Data->entityClasses.end();
}

void ScriptingEngine::onCreateEntity(Entity entity)
{
	const auto &sc = entity.getComponent<ScriptComponent>();
	if (ScriptingEngine::entityClassExists(sc.m_ClassName))
	{
		UUID entityID = entity.getUUID();

		std::shared_ptr<ScriptInstance> instance =
			std::make_shared<ScriptInstance>(s_Data->entityClasses[sc.m_ClassName], entity);

		s_Data->entityInstances[entityID] = instance;

		// Copy field values
		if (s_Data->entityScriptFields.find(entityID) != s_Data->entityScriptFields.end())
		{
			const ScriptFieldMap &fieldMap = s_Data->entityScriptFields.at(entityID);
			for (const auto &[name, fieldInstance] : fieldMap)
				instance->setFieldValueInternal(name, fieldInstance.m_Buffer);
		}
		instance->invokeOnCreate();
	}
}

void ScriptingEngine::onUpdateEntity(Entity entity, Timestep ts)
{
	UUID entityID = entity.getUUID();

	// Runtime중 삭제될 수 있기 때문에 check needed.
	if (s_Data->entityInstances.find(entityID) != s_Data->entityInstances.end())
	{
		std::shared_ptr<ScriptInstance> instance = s_Data->entityInstances[entityID];
		instance->invokeOnUpdate((float)ts);
	}
	else
	{
		// AL_CORE_ERROR("Could not find ScriptInstance for entity {}", entityID);
	}
}

std::map<std::string, std::function<bool()>> ScriptingEngine::getBooleanMethods(Entity entity)
{
	UUID entityID = entity.getUUID();

	if (s_Data->entityInstances.find(entityID) != s_Data->entityInstances.end())
	{
		std::shared_ptr<ScriptInstance> instance = s_Data->entityInstances[entityID];
		return instance->getAllBooleanMethods();
	}
	return std::map<std::string, std::function<bool()>>();
}

Scene *ScriptingEngine::getSceneContext()
{
	return s_Data->sceneContext;
}

MonoImage *ScriptingEngine::getCoreAssemblyImage()
{
	return s_Data->coreAssemblyImage;
}

MonoObject *ScriptingEngine::getManagedInstance(UUID uuid)
{
	// ASSERT
	return s_Data->entityInstances.at(uuid)->getManagedObject();
}

// MonoObject *ScriptingEngine::createManagedComponentInstance(Entity entity, MonoType *monoType)
// {
// 	// auto key = std::make_pair(entity.getUUID(), monoType);
// 	// if (s_Data->s_ComponentInstances.find(key) != s_Data->s_ComponentInstances.end())
// 	// 	return s_Data->s_ComponentInstances[key];

// 	AL_CORE_TRACE("createManagedComponentInstance");

// 	// 1. MonoType -> MonoClass 변환
// 	MonoClass *monoClass = mono_type_get_class(monoType);
// 	if (!monoClass)
// 	{
// 		AL_CORE_ERROR("[ScriptingEngine] Failed to get MonoClass from MonoType!");
// 		return nullptr;
// 	}

// 	// 2. C# 객체(인스턴스) 생성
// 	MonoObject *instance = instantiateClass(monoClass);
// 	if (!instance)
// 	{
// 		AL_CORE_ERROR("[ScriptingEngine] Failed to instantiate MonoClass!");
// 		return nullptr;
// 	}

// 	// 3. 만약 컴포넌트 쪽에서 생성자로 Entity ID를 받도록 설계했다면, .ctor(...) 호출
// 	//    - 예) C#에서 public MyComponent(ulong entityID) { ... }
// 	//    - parameterCount = 1
// 	MonoMethod *constructor = mono_class_get_method_from_name(monoClass, ".ctor", 1);
// 	if (constructor)
// 	{
// 		UUID entityID = entity.getUUID();
// 		void *param = &entityID;

// 		// 예외 처리( exception )는 필요시 별도 처리
// 		MonoObject *exception = nullptr;
// 		mono_runtime_invoke(constructor, instance, &param, &exception);

// 		if (exception)
// 		{
// 			// 예) 어떤 식으로든 로그 남기기
// 			AL_CORE_ERROR("[ScriptingEngine] Exception while invoking component constructor!");
// 			// 필요 시 반환
// 		}
// 	}
// 	// else
// 	// {
// 	// 	// 혹은, 생성자가 없을 경우 "m_EntityID" 같은 필드가 있는지 찾아 세팅할 수도 있음
// 	// 	MonoClassField *entityField = mono_class_get_field_from_name(monoClass, "m_EntityID");
// 	// 	if (entityField)
// 	// 	{
// 	// 		UUID entityID = entity.getUUID();
// 	// 		mono_field_set_value(instance, entityField, &entityID);
// 	// 	}
// 	// }

// 	// 4. (선택) s_Data->m_EntityComponentInstances[{entity.getUUID(), monoType}] = instance;
// 	//    캐싱/재사용하고 싶다면 이런 식으로 매핑 테이블에 저장 가능

// 	return instance;
// }

std::shared_ptr<ScriptInstance> ScriptingEngine::getEntityScriptInstance(UUID entityID)
{
	auto it = s_Data->entityInstances.find(entityID);
	if (it == s_Data->entityInstances.end())
		return nullptr;
	return it->second;
}

std::shared_ptr<ScriptClass> ScriptingEngine::getEntityClass(const std::string &name)
{
	if (s_Data->entityClasses.find(name) == s_Data->entityClasses.end())
		return nullptr;
	return s_Data->entityClasses.at(name);
}

std::unordered_map<std::string, std::shared_ptr<ScriptClass>> ScriptingEngine::getEntityClasses()
{
	return s_Data->entityClasses;
}

ScriptFieldMap &ScriptingEngine::getScriptFieldMap(Entity entity)
{
	UUID entityID = entity.getUUID();
	return s_Data->entityScriptFields[entityID];
}

void ScriptingEngine::loadAssemblyClasses()
{
	s_Data->entityClasses.clear();

	// Metadata를 나타내는 구조체 - 타입, 메서드, 필드 정보 등을 포함.
	const MonoTableInfo *typeDefsTable = mono_image_get_table_info(s_Data->appAssemblyImage, MONO_TABLE_TYPEDEF);
	int32_t numTypes = mono_table_info_get_rows(typeDefsTable);
	MonoClass *entityClass = mono_class_from_name(s_Data->coreAssemblyImage, "ALEngine", "Entity");

	for (int32_t i = 0; i < numTypes; ++i)
	{
		uint32_t cols[MONO_TYPEDEF_SIZE]; // 6
		mono_metadata_decode_row(typeDefsTable, i, cols, MONO_TYPEDEF_SIZE);

		const char *nameSpace = mono_metadata_string_heap(s_Data->appAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
		const char *className = mono_metadata_string_heap(s_Data->appAssemblyImage, cols[MONO_TYPEDEF_NAME]);
		std::string fullName;

		// C# Style
		if (strlen(nameSpace) != 0)
			fullName = fmt::format("{}.{}", nameSpace, className);
		else
			fullName = className;

		MonoClass *monoClass = mono_class_from_name(s_Data->appAssemblyImage, nameSpace, className);

		// ScriptCore에 정의한 Class와 일치.
		if (monoClass == entityClass)
			continue;

		// Class 계층 구조 검사, 부모-자식 관계 확인
		bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);
		if (!isEntity)
			continue;

		std::shared_ptr<ScriptClass> scriptClass = std::make_shared<ScriptClass>(nameSpace, className);
		s_Data->entityClasses[fullName] = scriptClass;

		// check class fields
		int32_t fieldCount = mono_class_num_fields(monoClass);
		void *iterator = nullptr;
		while (MonoClassField *field = mono_class_get_fields(monoClass, &iterator))
		{
			const char *fieldName = mono_field_get_name(field); // variable name
			uint32_t flags = mono_field_get_flags(field);
			if (flags & MONO_FIELD_ATTR_PUBLIC)
			{
				MonoType *type = mono_field_get_type(field); // variable type
				EScriptFieldType fieldType = utils::monoTypeToScriptFieldType(type);
				scriptClass->m_Fields[fieldName] = {fieldType, fieldName, field};
			}
		}
	}
}

MonoObject *ScriptingEngine::instantiateClass(MonoClass *monoClass)
{
	MonoObject *instance = mono_object_new(s_Data->appDomain, monoClass);

	// MonoObject의 생성자를 호출하여 초기화하는 함수
	mono_runtime_object_init(instance);
	return instance;
}

ScriptClass::ScriptClass(const std::string &classNamespace, const std::string &className, bool isCore)
	: m_ClassNamespace(classNamespace), m_ClassName(className)
{
	m_MonoClass = mono_class_from_name(isCore ? s_Data->coreAssemblyImage : s_Data->appAssemblyImage,
									   m_ClassNamespace.c_str(), m_ClassName.c_str());
}

std::vector<std::string> ScriptClass::getAllMethods()
{

	void* iter = nullptr;
	MonoMethod* method = nullptr;

	std::vector<std::string> methods;

	while ((method = mono_class_get_methods(m_MonoClass, &iter)) != nullptr)
	{
		MonoMethodSignature* sig = mono_method_signature(method);
		MonoType* returnType = mono_signature_get_return_type(sig);

		std::string methodName = mono_method_get_name(method);
		std::string returnTypeName = mono_type_get_name(returnType);
		methods.push_back(returnTypeName + "::" + methodName);
	}
	return methods;
}

MonoObject *ScriptClass::instantiate()
{
	return ScriptingEngine::instantiateClass(m_MonoClass);
}

MonoMethod *ScriptClass::getMethod(const std::string &name, int parameterCount)
{
	return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
}

MonoObject *ScriptClass::invokeMethod(MonoObject *instance, MonoMethod *method, void **params)
{
	MonoObject *exception = nullptr;

	// Runtime에 특정 Method를 호출하기 위해 사용.
	return mono_runtime_invoke(method, instance, params, &exception);
}

ScriptInstance::ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, Entity entity) : m_ScriptClass(scriptClass)
{
	m_Instance = scriptClass->instantiate();
	// GC에 의해 수집되지 않도록 GCHandle로 핀 고정
	m_GCHandle = mono_gchandle_new(m_Instance, false);

	// Mono Runtime에서 생성자는 내부적으로 .ctor라는 이름으로 처리됨.
	m_Constructor = s_Data->entityClass.getMethod(".ctor", 1);
	m_OnCreateMethod = scriptClass->getMethod("onCreate", 0);
	m_OnUpdateMethod = scriptClass->getMethod("onUpdate", 1);

	// 생성자 실행 - UUID Parmaeter needed.
	UUID entityID = entity.getUUID();
	void *param = &entityID;
	MonoObject* currentInstance = mono_gchandle_get_target(m_GCHandle);
	m_ScriptClass->invokeMethod(currentInstance, m_Constructor, &param);
}

ScriptInstance::~ScriptInstance()
{
	mono_gchandle_free(m_GCHandle);
}

void ScriptInstance::invokeOnCreate()
{
	if (m_OnCreateMethod)
	{
		MonoObject* currentInstance = mono_gchandle_get_target(m_GCHandle);
		m_ScriptClass->invokeMethod(currentInstance, m_OnCreateMethod);
	}
}

void ScriptInstance::invokeOnUpdate(float ts)
{
	if (m_OnUpdateMethod)
	{
		void *param = &ts;
		MonoObject* currentInstance = mono_gchandle_get_target(m_GCHandle);
		m_ScriptClass->invokeMethod(currentInstance, m_OnUpdateMethod, &param);
	}
}

bool ScriptInstance::getFieldValueInternal(const std::string &name, void *buffer)
{
	const auto &fields = m_ScriptClass->getFields();
	auto it = fields.find(name);
	if (it == fields.end())
		return false;

	const ScriptField &field = it->second;
	MonoObject* currentInstance = mono_gchandle_get_target(m_GCHandle);
	mono_field_get_value(currentInstance, field.m_ClassField, buffer);
	return true;
}

bool ScriptInstance::setFieldValueInternal(const std::string &name, const void *value)
{
	const auto &fields = m_ScriptClass->getFields();
	auto it = fields.find(name);
	if (it == fields.end())
		return false;

	const ScriptField &field = it->second;
	MonoObject* currentInstance = mono_gchandle_get_target(m_GCHandle);
	mono_field_set_value(currentInstance, field.m_ClassField, (void *)value);
	return true;
}

std::map<std::string, std::function<bool()>> ScriptInstance::getAllBooleanMethods()
{
	std::map<std::string, std::function<bool()>> output;
	std::string signature;
	auto strMethods = m_ScriptClass->getAllMethods();

	MonoMethod* method = nullptr;
	for (auto it = strMethods.begin(); it != strMethods.end(); ++it)
	{
		signature = *it;
		size_t pos = signature.find("::"); // "::"의 위치 찾기 (함수 이름과 앞부분을 구분하는 구분자)
		if (pos == std::string::npos)
			continue;
		std::string leftPart = signature.substr(0, pos); // "::" 앞부분 추출 (예: "System.Void")
		std::string name = signature.substr(pos + 2); // "::" 뒤부분 추출 (함수 이름, 예: "onUpdate")
		
		size_t dotPos = leftPart.rfind('.'); // 왼쪽 부분에서 마지막 '.'의 위치 찾기 (반환형은 마지막 '.' 뒤에 오는 부분)
		if (dotPos == std::string::npos)
			continue;
		
		std::string type = leftPart.substr(dotPos + 1); // 반환형 추출 (예: "Void")
		if (type == "Boolean")
		{
			method = m_ScriptClass->getMethod(name, 0);
			std::function<bool()> func = [this, method]() -> bool {
				MonoObject* currentInstance = mono_gchandle_get_target(m_GCHandle);
				MonoObject* result = m_ScriptClass->invokeMethod(currentInstance, method);
				
				bool value = *(bool *)mono_object_unbox(result);
				return value;
			};

			output[name] = func;
		}
	}
	return output;
}

} // namespace ale