import json
import os
import urllib.request

# Configurações de caminhos
SCHEMA_URL = "https://comicbookjson.github.io/schema/versions/cbjz-v1.schema.json"
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
OUTPUT_HEADER = os.path.join(PROJECT_ROOT, "core", "include", "CbjSchema.hpp")

def get_cpp_type(prop_dict, is_required):
    """Mapeia os tipos do JSON Schema para C++17"""
    cpp_type = "std::string"
    
    if "enum" in prop_dict:
        cpp_type = "std::string"
    elif "$ref" in prop_dict:
        ref_name = prop_dict["$ref"].split("/")[-1]
        cpp_type = ref_name[3:] if ref_name.startswith("Cbj") else ref_name
    elif prop_dict.get("type") == "array" or "items" in prop_dict:
        item_type = get_cpp_type(prop_dict["items"], True)["base"]
        cpp_type = f"std::vector<{item_type}>"
    else:
        types = prop_dict.get("type", "string")
        if isinstance(types, list):
            types = [t for t in types if t != "null"][0]
            
        if types == "string": cpp_type = "std::string"
        elif types == "integer": cpp_type = "int"
        elif types == "number": cpp_type = "double"
        elif types == "boolean": cpp_type = "bool"

    allows_null = isinstance(prop_dict.get("type"), list) and "null" in prop_dict.get("type")
    
    if allows_null or not is_required:
        return {"base": cpp_type, "full": f"std::optional<{cpp_type}>", "is_opt": True}
    return {"base": cpp_type, "full": cpp_type, "is_opt": False}

def capitalize_first(s):
    return s[0].upper() + s[1:] if s else ""

def generate_class(name, schema_obj):
    """Gera uma Classe C++ POCO completa com Getters/Setters"""
    clean_name = name[3:] if name.startswith("Cbj") else name
    lines = [
        f"    class {clean_name} {{",
        "    public:",
        f"        {clean_name}() = default;",
        f"        virtual ~{clean_name}() = default;",
        ""
    ]
    
    properties = schema_obj.get("properties", {})
    required = schema_obj.get("required", [])
    
    # 1. Gerar os campos privados
    lines.append("    private:")
    for prop_name, prop_details in properties.items():
        is_req = prop_name in required
        type_info = get_cpp_type(prop_details, is_req)
        
        # Valor padrão, se houver
        default_val = ""
        if "default" in prop_details and type_info["base"] == "std::string":
            default_val = f' = "{prop_details["default"]}"'
        elif type_info["base"] == "std::string" and not type_info["is_opt"]:
            default_val = ' = ""'
            
        description = prop_details.get("description", "Schema property: " + prop_name)
        lines.append(f"        /** {description} */")
        lines.append(f"        {type_info['full']} _{prop_name}{default_val};")
    
    lines.append("")
    
    # 2. Gerar Getters e Setters Públicos
    lines.append("    public:")
    for prop_name, prop_details in properties.items():
        is_req = prop_name in required
        type_info = get_cpp_type(prop_details, is_req)
        pascal_case_name = capitalize_first(prop_name)
        
        # Comentário da documentação
        if "description" in prop_details:
            lines.append(f"        /** {prop_details['description']} */")
        
        # Getter const (retorna cópia para tipos simples e const ref para complexos)
        if type_info["is_opt"] or type_info["base"] in ["int", "double", "bool"]:
             lines.append(f"        {type_info['full']} Get{pascal_case_name}() const {{ return _{prop_name}; }}")
        else:
             lines.append(f"        const {type_info['full']}& Get{pascal_case_name}() const {{ return _{prop_name}; }}")
        
        # Setter (Usa const ref para strings/vectors)
        if type_info["is_opt"] or type_info["base"] in ["int", "double", "bool"]:
            lines.append(f"        void Set{pascal_case_name}(const {type_info['full']}& value) {{ _{prop_name} = value; }}")
        else:
            lines.append(f"        void Set{pascal_case_name}(const {type_info['full']}& value) {{ _{prop_name} = value; }}")
            
        lines.append(f"        /** Gets a mutable reference to the {prop_name} schema property. */")
        lines.append(f"        {type_info['full']}& Mutable{pascal_case_name}() {{ return _{prop_name}; }}")

        lines.append("") # Linha em branco entre as propriedades
        
    lines.append("    };\n")
    return "\n".join(lines)

def main():
    print("Baixando JSON Schema...")
    try:
        req = urllib.request.Request(SCHEMA_URL, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response:
            schema = json.loads(response.read().decode())
    except Exception as e:
        print(f"Erro ao baixar: {e}. Usando schema local...")
        with open(os.path.join(PROJECT_ROOT, "schema", "src", "wwwroot", "versions", "cbjz-v1.schema.json"), "r", encoding="utf-8") as f:
            schema = json.load(f)

    cpp_code = [
        "#pragma once",
        "",
        "#include <string>",
        "#include <vector>",
        "#include <optional>",
        "",
        "namespace cbj {",
        ""
    ]

    defs = schema.get("$defs", {})
    # Ordem correta de declaração
    order = ["CbjTag", "CbjCreator", "CbjChapter", "CbjTagPosition", "CbjMetadata", "CbjPage"]
    
    for def_name in order:
        if def_name in defs:
            cpp_code.append(generate_class(def_name, defs[def_name]))

    # Root Document
    cpp_code.append(generate_class("CbjDocument", schema))

    cpp_code.append("} // namespace cbj\n")

    os.makedirs(os.path.dirname(OUTPUT_HEADER), exist_ok=True)
    with open(OUTPUT_HEADER, "w", encoding="utf-8") as f:
        f.write("\n".join(cpp_code))
        
    print(f"✅ Arquivo C++ DTO gerado com sucesso em: {OUTPUT_HEADER}")

if __name__ == "__main__":
    main()