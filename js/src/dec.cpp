#include "jsapi.h"
#include <iostream>
#include <fstream>
#include <vector>
#include "jsdbgapi.h"
#include "jsopcode.h"

using namespace JS;

template<typename T>
inline T *
checkPtr(T *ptr)
{
  if (! ptr)
    abort();
  return ptr;
}
void
checkBool(JSBool success)
{
  if (! success)
    abort();
}

// Função para ler um arquivo binário inteiro em buffer
bool readBinaryFile(const char* filename, std::vector<uint8_t>& out) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    out.resize(size);
    file.read(reinterpret_cast<char*>(out.data()), size);
    return true;
}

// Classe global mínima
JSClass global_class = {
    "global",
    JSCLASS_GLOBAL_FLAGS,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub
};

// Tratador de erro
void reportError(JSContext* cx, const char* message, JSErrorReport* report) {
    fprintf(stderr, "%s:%u: %s\n",
        report->filename ? report->filename : "<sem nome>",
        (unsigned int) report->lineno,
        message);
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./dec script.jsc\n";
        return 1;
    }

    const char* jscFile = argv[1];
    std::vector<uint8_t> data;
    if (!readBinaryFile(jscFile, data)) {
        std::cerr << "Erro ao ler bytecode.\n";
        return 1;
    }

    JSRuntime *runtime = checkPtr(JS_NewRuntime(1024 * 1024, JS_USE_HELPER_THREADS));
    JS_SetGCParameter(runtime, JSGC_MAX_BYTES, 0xffffffff);
    JS_SetNativeStackQuota(runtime, 5000000);

    JSContext *cx = checkPtr(JS_NewContext(runtime, 8192));
    JS_SetErrorReporter(cx, reportError);

    JS_SetErrorReporter(cx, reportError);
    JS_BeginRequest(cx);

    JS::CompileOptions options(cx);
    options.setVersion(JSVERSION_LATEST);
    RootedObject global(cx, checkPtr(JS_NewGlobalObject(cx, &global_class, NULL)));
    JS_SetGlobalObject(cx, global);

    JSAutoCompartment ac(cx, global);

    /* Populate the global object with the standard globals,
       like Object and Array. */
    checkBool(JS_InitStandardClasses(cx, global));


    JSScript* script = JS_DecodeScript(cx, data.data(), data.size(), NULL, NULL);
    if (!script) {
        std::cerr << "Erro ao decodificar bytecode.\n";
        return 1;
    }

    std::cout << JS_GetScriptLineExtent(cx, script) << std::endl;
    // JS_DumpBytecode(cx, script);

    //JS_DumpBytecode(cx, script);
    DumpScriptWithInnerFunctions(cx, script);
    //js_DumpScript(cx, script);
    // JSString* str = JS_DecompileScript(cx, script, "/Users/wubin/myspace/workspace/client_build/cocos2d-x-2.2.6/projects/MyGame/Resources/UI/main.js", 4);

    // // JSString* str = JS_DecompileScript(cx, script, "main.js", 4);
    // if (!str) {
    //     std::cerr << "Erro ao decompilar script.\n";
    //     return 1;
    // }

    // char* decompiled = JS_EncodeString(cx, str);
    // std::cout << decompiled << std::endl;

    return 0;
}
