#pragma once

#include "expression.hpp"

#include <vector>
#include <unordered_map>
#include <sstream>

namespace hs {
    const char* g_type_tag_hr_names[] = {
        "none",
        "function",
        "integral",
        "struct",
        "pointer"
    };

    enum type_tag_t {
        TYPE_NONE,
        TYPE_FUNCTION,
        TYPE_INTEGRAL,
        TYPE_STRUCT,
        TYPE_POINTER
    };

    struct hs_type_t {
        std::string signature;

        type_tag_t tag;

        size_t size;

        bool mut, is_static;
    };

    struct definition_t {
        hs_type_t* type;

        std::string name;
    };

    class integral_type_t : public hs_type_t {
    public:
        bool sign;

        integral_type_t(std::string signature, size_t size, bool sign):
            hs_type_t {signature, TYPE_INTEGRAL, size},
            sign(sign) {};
    };

    class none_type_t : public hs_type_t {
    public:
        none_type_t(): hs_type_t {"none", TYPE_NONE, 0} {};
    };

    class struct_type_t : public hs_type_t {
    public:
        std::vector <definition_t> members;
    
        struct_type_t(std::string signature):
            hs_type_t {signature, TYPE_STRUCT} {};
        
        struct_type_t():
            hs_type_t {"", TYPE_STRUCT} {};
    };

    class function_type_t : public hs_type_t {
    public:
        std::vector <definition_t> args;

        hs_type_t* return_type;
    
        function_type_t(std::string signature) :
            hs_type_t {signature, TYPE_FUNCTION} {};
        
        function_type_t():
            hs_type_t {"", TYPE_FUNCTION} {};
    };

    class pointer_type_t : public hs_type_t {
    public:
        hs_type_t* target_type;

        pointer_type_t(std::string signature, hs_type_t* target_type) :
            hs_type_t {signature, TYPE_POINTER, 4},
            target_type(target_type) {};
        
        pointer_type_t(hs_type_t* target_type):
            hs_type_t {"", TYPE_POINTER, 4},
            target_type(target_type) {};
    };

    std::unordered_map <std::string, hs_type_t*> g_base_type_map = {
        { "none", new none_type_t()                     },
        { "u8"  , new integral_type_t("u8", 1, false)   },
        { "u16" , new integral_type_t("u16", 2, false)  },
        { "u32" , new integral_type_t("u32", 4, false)  },
        { "i8"  , new integral_type_t("i8", 1, true)    },
        { "i16" , new integral_type_t("i16", 2, true)   },
        { "i32" , new integral_type_t("i32", 4, true)   }
    };

    std::string get_signature(hs_type_t* t) {
        std::string signature;

        if (t->mut) {
            signature.append("mut ");
        }

        if (t->is_static) {
            signature.append("static ");
        }

        switch (t->tag) {
            case TYPE_FUNCTION: {
                function_type_t* fty = (function_type_t*)t;

                if (!fty->args.size()) {
                    return "()->" + get_signature(fty->return_type);
                } else {
                    signature.push_back('(');

                    for (int i = 0; i < fty->args.size(); i++) {
                        _log(debug, "fty->mut=%u", fty->args[i].type->mut);
                        signature.append(get_signature(fty->args[i].type));
                        signature.push_back(',');
                    }

                    if (signature.back() == ',')
                        signature.pop_back();

                    signature.push_back(')');
                }

                return signature + "->" + get_signature(fty->return_type);
            } break;

            case TYPE_INTEGRAL: {
                return t->signature;
            } break;

            case TYPE_NONE: {
                return "none";
            } break;

            case TYPE_POINTER: {
                pointer_type_t* pty = (pointer_type_t*)t;

                return get_signature(pty->target_type) + "*";
            } break;
        }
    }

    class type_system_t {
        std::unordered_map <std::string, hs_type_t*> type_map;
        std::unordered_map <std::string, std::string> typedef_map;

    public:
#define CONSUME c = stream = get();
#define IGNORE_WHITESPACE \
    while (std::isspace(c)) c = stream.get();

        void add_type(std::string signature, hs_type_t* type) {
            if ((!type_map.contains(signature)) && (!g_base_type_map.contains(signature))) {
                type->signature = signature;
                type_map[signature] = type;
            }
        }

        void type_def(std::string alias, std::string target) {
            if (typedef_map.contains(alias)) {
                // Warning: typedef already exists

                return;
            }

            typedef_map[alias] = target;
        }

        bool type_eq(hs_type_t* t, hs_type_t* u) {
            // Type class not equal
            // ex. int != int*
            if (t->tag != u->tag)
                return false;

            // Representation size not equal
            // ex. int != long
            if (t->size != u->size)
                return false;

            // Mutability not equal
            // ex. int != mut int
            if (t->mut != u->mut)
                return false;
            
            // Storage duration not equal
            // ex. int != static int
            if (t->is_static != u->is_static)
                return false;

            // The code above rules ouf most common types.

            switch (t->tag) {
                case TYPE_FUNCTION: {
                    function_type_t* tc = (function_type_t*)t;
                    function_type_t* uc = (function_type_t*)u;

                    // Functions with unequal return types
                    // are not equal
                    if (!type_eq(tc->return_type, uc->return_type))
                        return false;
                    
                    // Functions with different number of arguments
                    // are not equal
                    if (tc->args.size() != uc->args.size())
                        return false;
                    
                    // Functions are not equal if their argument types
                    // aren't all equal
                    for (int i = 0; i < tc->args.size(); i++) {
                        if (!type_eq(tc->args[i].type, uc->args[i].type))
                            return false;
                    }

                    // Otherwise, the two function types are equal
                    return true;
                } break;

                case TYPE_INTEGRAL: {
                    integral_type_t* tc = (integral_type_t*)t;
                    integral_type_t* uc = (integral_type_t*)u;

                    // Only requirement for two integral types
                    // to be equal other than the base requirements
                    // is the signedness being equal
                    // ex. i8 != u8
                    return tc->sign == uc->sign;
                } break;

                case TYPE_NONE: {
                    // All none types are equal
                    return true;
                } break;

                case TYPE_POINTER: {
                    pointer_type_t* tc = (pointer_type_t*)t;
                    pointer_type_t* uc = (pointer_type_t*)u;

                    // Only requirement for two pointer types
                    // to be be equal is to have equal target
                    // types
                    return type_eq(tc->target_type, uc->target_type);
                } break;

                case TYPE_STRUCT: {
                    struct_type_t* tc = (struct_type_t*)t;
                    struct_type_t* uc = (struct_type_t*)u;

                    if (tc->members.size() != uc->members.size())
                        return false;

                    // Similar to function types
                    for (int i = 0; i < tc->members.size(); i++) {
                        if (!type_eq(tc->members[i].type, uc->members[i].type))
                            return false;
                    }

                    return true;
                } break;

                default: {
                    // Error: Unknown types
                } break;
            }
        }

        bool is_none(hs_type_t* t) {
            return t->tag == TYPE_NONE;
        }

        hs_type_t* get_type(std::string signature) {
            if (typedef_map.contains(signature)) {
                signature = typedef_map[signature];
            }

            if (g_base_type_map.contains(signature)) {
                return g_base_type_map[signature];
            }

            if (type_map.contains(signature)) {
                return type_map[signature];
            }

            return g_base_type_map["none"];
        }

        bool exists(std::string signature) {
            return !is_none(get_type(signature));
        }

#undef CONSUME
#undef IGNORE_WHITESPACE

        void init() {
            struct_type_t* hs_string_type = new struct_type_t;

            hs_string_type->members.push_back({ get_type("u32"), "size" });
            hs_string_type->members.push_back({ get_type("char*"), "ptr" });

            type_map["hs_string"] = new struct_type_t;
        }
    };
}