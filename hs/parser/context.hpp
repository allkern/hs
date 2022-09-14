#pragma once

#include "../error.hpp"

#include "expression.hpp"
#include "parser.hpp"
#include "output.hpp"

#include <string>
#include <stack>

#define WARNING(msg, expr) \
    if (m_logger) m_logger->print_warning( \
        "context", \
        msg, \
        expr->line, expr->offset, expr->len, true \
    );

namespace hs {
    struct contextualizer_t {
        parser_output_t* m_po;
        std::stack <std::string> m_scope;
        error_logger_t* m_logger;

        std::vector <std::string> m_vars_in_global_scope;
        std::vector <std::string> m_vars_in_current_scope;

        void init(parser_t* parser, error_logger_t* logger) {
            m_po = parser->get_output();
            m_logger = logger;

            m_scope.push("<global>");
        }

        std::string get_scope(std::string name) {
            auto dot = name.find_last_of('.');

            if (dot == std::string::npos) {
                return name;
            } else {
                return name.substr(dot);
            }
        }

        std::string get_name_scope(std::string name) {
            auto last_dot = name.find_last_of('.');
            auto prev_dot = name.substr(last_dot - 1).find_last_of('.');

            if (prev_dot == std::string::npos) {
                return name.substr(last_dot);
            } else {
                return name.substr(prev_dot, last_dot - prev_dot);
            }

            return name;
        }

        void contextualize_impl(expression_t* expr) {
            switch (expr->get_type()) {
                case EX_FUNCTION_DEF: {
                    function_def_t* fd = (function_def_t*)expr;

                    if (m_scope.top() == "<global>") {
                        m_vars_in_global_scope.push_back(fd->name);
                    } else {
                        auto global = std::find(
                            std::begin(m_vars_in_global_scope),
                            std::end(m_vars_in_global_scope),
                            fd->name
                        );

                        if (global != std::end(m_vars_in_global_scope))
                            WARNING(
                                fmt("Defining clashing name \"%s\" on scope %s",
                                    fd->name.c_str(),
                                    get_scope(m_scope.top()).c_str()
                                ),
                                fd
                            );

                        m_vars_in_current_scope.push_back(fd->name);
                    }

                    fd->name = m_scope.top() + "." + fd->name;

                    for (function_arg_t& arg : fd->args) {
                        m_vars_in_current_scope.push_back(arg.name);

                        arg.name = fd->name + "." + arg.name;
                    }

                    m_scope.push(fd->name);

                    contextualize_impl(fd->body);

                    m_vars_in_current_scope.clear();
                    m_scope.pop();
                } break;

                case EX_VARIABLE_DEF: {
                    variable_def_t* vd = (variable_def_t*)expr;

                    if (m_scope.top() == "<global>") {
                        m_vars_in_global_scope.push_back(vd->name);
                    } else {
                        auto global = std::find(
                            std::begin(m_vars_in_global_scope),
                            std::end(m_vars_in_global_scope),
                            vd->name
                        );

                        if (global != std::end(m_vars_in_global_scope))
                            WARNING(
                                fmt("Defining clashing name \"%s\" on scope %s",
                                    vd->name.c_str(),
                                    get_scope(m_scope.top()).c_str()
                                ),
                                vd
                            );

                        m_vars_in_current_scope.push_back(vd->name);
                    }

                    vd->name = m_scope.top() + "." + vd->name;
                } break;

                case EX_NAME_REF: {
                    name_ref_t* nr = (name_ref_t*)expr;

                    auto current = std::find(
                        std::begin(m_vars_in_current_scope),
                        std::end(m_vars_in_current_scope),
                        nr->name
                    );

                    bool found_in_current_scope = current != std::end(m_vars_in_current_scope);

                    if (found_in_current_scope) {
                        nr->name = m_scope.top() + "." + nr->name;
                    }

                    auto global = std::find(
                        std::begin(m_vars_in_global_scope),
                        std::end(m_vars_in_global_scope),
                        nr->name
                    );

                    bool found_in_global_scope = global != std::end(m_vars_in_global_scope);

                    if (found_in_current_scope && found_in_global_scope) {
                        WARNING(
                            fmt("Clashing name \"%s\" on scope %s",
                                (*current).c_str(),
                                get_scope(m_scope.top()).c_str()
                            ),
                            nr
                        );

                        break;
                    }

                    if (found_in_current_scope) break;

                    if (found_in_global_scope) {
                        nr->name = "<global>." + nr->name;
                    } else {
                        WARNING(
                            fmt("Using undefined name \"%s\" on scope %s",
                                nr->name.c_str(),
                                get_scope(m_scope.top()).c_str()
                            ),
                            nr
                        );

                        nr->name = "<unknown>." + nr->name;
                    }
                } break;

                case EX_ARRAY_ACCESS: {
                    array_access_t* aa = (array_access_t*)expr;

                    contextualize_impl(aa->type_or_name);
                    contextualize_impl(aa->addr);
                } break;

                case EX_ASSIGNMENT: {
                    assignment_t* as = (assignment_t*)expr;

                    contextualize_impl(as->assignee);
                    contextualize_impl(as->value);
                } break;

                case EX_BINARY_OP: {
                    binary_op_t* bo = (binary_op_t*)expr;

                    contextualize_impl(bo->lhs);
                    contextualize_impl(bo->rhs);
                } break;

                case EX_EXPRESSION_BLOCK: {
                    expression_block_t* block = (expression_block_t*)expr;

                    for (expression_t* expr : block->block) {
                        contextualize_impl(expr);
                    }
                } break;

                case EX_FUNCTION_CALL: {
                    function_call_t* fc = (function_call_t*)expr;

                    contextualize_impl(fc->addr);

                    for (expression_t* expr : fc->args) {
                        contextualize_impl(expr);
                    }
                } break;

                case EX_INVOKE: {
                    invoke_expr_t* ie = (invoke_expr_t*)expr;

                    contextualize_impl(ie->ptr);
                } break;

                case EX_WHILE_LOOP: {
                    while_loop_t* wl = (while_loop_t*)expr;

                    contextualize_impl(wl->condition);
                    contextualize_impl(wl->body);
                } break;

                case EX_NUMERIC_LITERAL: {

                } break;

                case EX_TYPE: {

                } break;

                case EX_NONE: {

                } break;
            }
        }

        void contextualize() {
            for (expression_t* expr : m_po->source) {
                contextualize_impl(expr);

                // _log(debug, "expression:\n%s", expr->print(0).c_str());
            }
        }
    };
}

#undef WARNING