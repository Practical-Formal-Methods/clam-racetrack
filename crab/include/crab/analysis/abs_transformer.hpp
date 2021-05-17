#pragma once

/*
   Implementation of the abstract transfer functions by reducing them
   to abstract domain operations.

   These are the main Crab statements for which we define their abstract
   transfer functions:

   ARITHMETIC and BOOLEAN
     x := y bin_op z;
     x := y;
     assume(cst)
     assert(cst);
     x := select(cond, y, z);

   ARRAYS
     a[l...u] := v (a,b are arrays and v can be bool/integer/pointer)
     a[i] := v;
     v := a[i];
     a := b

   POINTERS
     *p = q;
     p = *q;
     p := q+n
     p := &obj;
     p := &fun
     p := null;

   FUNCTIONS
     x := foo(arg1,...,argn);
     return r;

   havoc(x);

 */

#include <crab/cfg/cfg.hpp>
#include <crab/common/debug.hpp>
#include <crab/common/stats.hpp>
#include <crab/domains/abstract_domain_operators.hpp>
#include <crab/domains/linear_constraints.hpp>

//Includes for DeepSymbol
#include <iostream>
#include <vector>
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <map>
#include <string>
#include <sstream>
#include <ostream>
#include <cstdlib>
#include <utility>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fstream>
#include <time.h>

#define CON(i) for(loopv[i]=input_box_int[i].first; loopv[i]<=input_box_int[i].second;loopv[i]++)

namespace crab {
namespace analyzer {

/**
 * API abstract transformer
 **/
template <typename Number, typename VariableName>
class abs_transformer_api
    : public crab::cfg::statement_visitor<Number, VariableName> {
public:
  typedef Number number_t;
  typedef VariableName varname_t;

  typedef ikos::variable<number_t, VariableName> var_t;
  typedef ikos::linear_expression<number_t, VariableName> lin_exp_t;
  typedef ikos::linear_constraint<number_t, VariableName> lin_cst_t;
  typedef ikos::linear_constraint_system<number_t, VariableName> lin_cst_sys_t;
  typedef ikos::disjunctive_linear_constraint_system<number_t, VariableName> dis_lin_cst_sys_t;

  typedef crab::cfg::havoc_stmt<number_t, VariableName> havoc_t;
  typedef crab::cfg::unreachable_stmt<number_t, VariableName> unreach_t;

  typedef crab::cfg::binary_op<number_t, VariableName> bin_op_t;
  typedef crab::cfg::assignment<number_t, VariableName> assign_t;
  typedef crab::cfg::assume_stmt<number_t, VariableName> assume_t;
  typedef crab::cfg::select_stmt<number_t, VariableName> select_t;
  typedef crab::cfg::assert_stmt<number_t, VariableName> assert_t;
  typedef crab::cfg::int_cast_stmt<number_t, VariableName> int_cast_t;
  
  typedef crab::cfg::callsite_stmt<number_t, VariableName> callsite_t;
  typedef crab::cfg::return_stmt<number_t, VariableName> return_t;
  typedef crab::cfg::intrinsic_stmt<number_t, VariableName> intrinsic_t;  

  typedef crab::cfg::array_init_stmt<number_t, VariableName> arr_init_t;
  typedef crab::cfg::array_store_stmt<number_t, VariableName> arr_store_t;
  typedef crab::cfg::array_load_stmt<number_t, VariableName> arr_load_t;
  typedef crab::cfg::array_assign_stmt<number_t, VariableName> arr_assign_t;
  typedef crab::cfg::ptr_store_stmt<number_t, VariableName> ptr_store_t;
  typedef crab::cfg::ptr_load_stmt<number_t, VariableName> ptr_load_t;

  typedef crab::cfg::ptr_assign_stmt<number_t, VariableName> ptr_assign_t;
  typedef crab::cfg::ptr_object_stmt<number_t, VariableName> ptr_object_t;
  typedef crab::cfg::ptr_function_stmt<number_t, VariableName> ptr_function_t;
  typedef crab::cfg::ptr_null_stmt<number_t, VariableName> ptr_null_t;
  typedef crab::cfg::ptr_assume_stmt<number_t, VariableName> ptr_assume_t;
  typedef crab::cfg::ptr_assert_stmt<number_t, VariableName> ptr_assert_t;

  typedef crab::cfg::bool_binary_op<number_t, VariableName> bool_bin_op_t;
  typedef crab::cfg::bool_assign_cst<number_t, VariableName> bool_assign_cst_t;
  typedef crab::cfg::bool_assign_var<number_t, VariableName> bool_assign_var_t;
  typedef crab::cfg::bool_assume_stmt<number_t, VariableName> bool_assume_t;
  typedef crab::cfg::bool_select_stmt<number_t, VariableName> bool_select_t;
  typedef crab::cfg::bool_assert_stmt<number_t, VariableName> bool_assert_t;

protected:
  virtual void exec(havoc_t &) {}
  virtual void exec(unreach_t &) {}
  virtual void exec(bin_op_t &) {}
  virtual void exec(assign_t &) {}
  virtual void exec(assume_t &) {}
  virtual void exec(select_t &) {}
  virtual void exec(assert_t &) {}
  virtual void exec(int_cast_t &) {}
  virtual void exec(callsite_t &) {}
  virtual void exec(return_t &) {}
  virtual void exec(intrinsic_t &) {}  
  virtual void exec(arr_init_t &) {}
  virtual void exec(arr_store_t &) {}
  virtual void exec(arr_load_t &) {}
  virtual void exec(arr_assign_t &) {}
  virtual void exec(ptr_store_t &) {}
  virtual void exec(ptr_load_t &) {}
  virtual void exec(ptr_assign_t &) {}
  virtual void exec(ptr_object_t &) {}
  virtual void exec(ptr_function_t &) {}
  virtual void exec(ptr_null_t &) {}
  virtual void exec(ptr_assume_t &) {}
  virtual void exec(ptr_assert_t &) {}
  virtual void exec(bool_bin_op_t &) {}
  virtual void exec(bool_assign_cst_t &) {}
  virtual void exec(bool_assign_var_t &) {}
  virtual void exec(bool_assume_t &) {}
  virtual void exec(bool_select_t &) {}
  virtual void exec(bool_assert_t &) {}

public: /* visitor api */
  void visit(havoc_t &s) { exec(s); }
  void visit(unreach_t &s) { exec(s); }
  void visit(bin_op_t &s) { exec(s); }
  void visit(assign_t &s) { exec(s); }
  void visit(assume_t &s) { exec(s); }
  void visit(select_t &s) { exec(s); }
  void visit(assert_t &s) { exec(s); }
  void visit(int_cast_t &s) { exec(s); }
  void visit(callsite_t &s) { exec(s); }
  void visit(return_t &s) { exec(s); }
  void visit(intrinsic_t &s) { exec(s); }  
  void visit(arr_init_t &s) { exec(s); }
  void visit(arr_store_t &s) { exec(s); }
  void visit(arr_load_t &s) { exec(s); }
  void visit(arr_assign_t &s) { exec(s); }
  void visit(ptr_store_t &s) { exec(s); }
  void visit(ptr_load_t &s) { exec(s); }
  void visit(ptr_assign_t &s) { exec(s); }
  void visit(ptr_object_t &s) { exec(s); }
  void visit(ptr_function_t &s) { exec(s); }
  void visit(ptr_null_t &s) { exec(s); }
  void visit(ptr_assume_t &s) { exec(s); }
  void visit(ptr_assert_t &s) { exec(s); }
  void visit(bool_bin_op_t &s) { exec(s); }
  void visit(bool_assign_cst_t &s) { exec(s); }
  void visit(bool_assign_var_t &s) { exec(s); }
  void visit(bool_assume_t &s) { exec(s); }
  void visit(bool_select_t &s) { exec(s); }
  void visit(bool_assert_t &s) { exec(s); }
};

/**
 * Abstract forward transformer for all statements. Function calls
 * can be redefined by derived classes. By default, all function
 * calls are ignored in a sound manner (by havoc'ing all outputs).
 **/
template <class AbsD>
class intra_abs_transformer
    : public abs_transformer_api<typename AbsD::number_t,
                                 typename AbsD::varname_t> {

public:
  typedef AbsD abs_dom_t;
  typedef typename abs_dom_t::number_t number_t;
  typedef typename abs_dom_t::varname_t varname_t;
  typedef typename abs_dom_t::variable_t variable_t;

public:
  typedef abs_transformer_api<number_t, varname_t> abs_transform_api_t;
  using typename abs_transform_api_t::arr_assign_t;
  using typename abs_transform_api_t::arr_init_t;
  using typename abs_transform_api_t::arr_load_t;
  using typename abs_transform_api_t::arr_store_t;
  using typename abs_transform_api_t::assert_t;
  using typename abs_transform_api_t::assign_t;
  using typename abs_transform_api_t::assume_t;
  using typename abs_transform_api_t::bin_op_t;
  using typename abs_transform_api_t::bool_assert_t;
  using typename abs_transform_api_t::bool_assign_cst_t;
  using typename abs_transform_api_t::bool_assign_var_t;
  using typename abs_transform_api_t::bool_assume_t;
  using typename abs_transform_api_t::bool_bin_op_t;
  using typename abs_transform_api_t::bool_select_t;
  using typename abs_transform_api_t::callsite_t;
  using typename abs_transform_api_t::intrinsic_t;  
  using typename abs_transform_api_t::havoc_t;
  using typename abs_transform_api_t::int_cast_t;
  using typename abs_transform_api_t::lin_cst_sys_t;
  using typename abs_transform_api_t::dis_lin_cst_sys_t;
  using typename abs_transform_api_t::lin_cst_t;
  using typename abs_transform_api_t::lin_exp_t;
  using typename abs_transform_api_t::ptr_assert_t;
  using typename abs_transform_api_t::ptr_assign_t;
  using typename abs_transform_api_t::ptr_assume_t;
  using typename abs_transform_api_t::ptr_function_t;
  using typename abs_transform_api_t::ptr_load_t;
  using typename abs_transform_api_t::ptr_null_t;
  using typename abs_transform_api_t::ptr_object_t;
  using typename abs_transform_api_t::ptr_store_t;
  using typename abs_transform_api_t::return_t;
  using typename abs_transform_api_t::select_t;
  using typename abs_transform_api_t::unreach_t;
  using typename abs_transform_api_t::var_t;

protected:
  abs_dom_t m_inv;
  bool m_ignore_assert;

private:
  template <typename NumOrVar>
  void apply(abs_dom_t &inv, binary_operation_t op, variable_t x, variable_t y,
             NumOrVar z) {
    if (auto top = conv_op<ikos::operation_t>(op)) {
      inv.apply(*top, x, y, z);
    } else if (auto top = conv_op<ikos::bitwise_operation_t>(op)) {
      inv.apply(*top, x, y, z);
    } else {
      CRAB_ERROR("unsupported binary operator", op);
    }
  }

public:
  intra_abs_transformer(abs_dom_t inv, bool ignore_assert = false)
      : m_inv(inv), m_ignore_assert(ignore_assert) {}

  virtual ~intra_abs_transformer() {}

  void set_abs_value(abs_dom_t &&inv) { m_inv = std::move(inv); }

  abs_dom_t &get_abs_value() { return m_inv; }

  void exec(bin_op_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag &&
        (!(stmt.op() >= BINOP_SDIV && stmt.op() <= BINOP_UREM))) {
      pre_bot = m_inv.is_bottom();
    }

    auto op1 = stmt.left();
    auto op2 = stmt.right();
    if (op1.get_variable() && op2.get_variable()) {
      apply(m_inv, stmt.op(), stmt.lhs(), (*op1.get_variable()),
            (*op2.get_variable()));
    } else {
      assert(op1.get_variable() && op2.is_constant());
      apply(m_inv, stmt.op(), stmt.lhs(), (*op1.get_variable()),
            op2.constant());
    }

    if (::crab::CrabSanityCheckFlag &&
        (!(stmt.op() >= BINOP_SDIV && stmt.op() <= BINOP_UREM))) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(select_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    abs_dom_t inv1(m_inv);
    abs_dom_t inv2(m_inv);

    inv1 += stmt.cond();
    inv2 += stmt.cond().negate();

    if (::crab::CrabSanityCheckFlag) {
      if (!pre_bot && (inv1.is_bottom() && inv2.is_bottom())) {
        CRAB_ERROR(
            "select condition and its negation cannot be false simultaneously ",
            stmt);
      }
    }

    crab::outs() << "Reached select statement with branch condition " << stmt.cond() << "\n";

    if (inv2.is_bottom()) {
      inv1.assign(stmt.lhs(), stmt.left());
      m_inv = inv1;
    } else if (inv1.is_bottom()) {
      inv2.assign(stmt.lhs(), stmt.right());
      m_inv = inv2;
    } else {
      inv1.assign(stmt.lhs(), stmt.left());
      inv2.assign(stmt.lhs(), stmt.right());
      m_inv = inv1 | inv2;
    }

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(assign_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    m_inv.assign(stmt.lhs(), stmt.rhs());

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(assume_t &stmt) { m_inv.operator+=(stmt.constraint()); }

  void exec(assert_t &stmt) {
    if (m_ignore_assert)
      return;

    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    m_inv.operator+=(stmt.constraint());

    if (::crab::CrabSanityCheckFlag) {
      if (!stmt.constraint().is_contradiction()) {
        bool post_bot = m_inv.is_bottom();
        if (!(pre_bot || !post_bot)) {
          CRAB_WARN("Invariant became bottom after ", stmt, ".",
                    " This might indicate that the assertion is violated");
        }
      }
    }
  }

  void exec(int_cast_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }
    if (auto op = conv_op<crab::domains::int_conv_operation_t>(stmt.op())) {
      m_inv.apply(*op, stmt.dst(), stmt.src());
    } else {
      CRAB_ERROR("unsupported cast operator ", stmt.op());
    }

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(bool_assign_cst_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    m_inv.assign_bool_cst(stmt.lhs(), stmt.rhs());

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(bool_assign_var_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }
    m_inv.assign_bool_var(stmt.lhs(), stmt.rhs(), stmt.is_rhs_negated());

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(bool_bin_op_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    if (auto op = conv_op<domains::bool_operation_t>(stmt.op())) {
      m_inv.apply_binary_bool(*op, stmt.lhs(), stmt.left(), stmt.right());
    } else {
      CRAB_WARN("Unsupported statement ", stmt);
    }

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(bool_assume_t &stmt) {
    m_inv.assume_bool(stmt.cond(), stmt.is_negated());
  }

  void exec(bool_select_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    abs_dom_t inv1(m_inv);
    abs_dom_t inv2(m_inv);
    const bool negate = true;
    inv1.assume_bool(stmt.cond(), !negate);
    inv2.assume_bool(stmt.cond(), negate);
    if (inv2.is_bottom()) {
      inv1.assign_bool_var(stmt.lhs(), stmt.left(), !negate);
      m_inv = inv1;
    } else if (inv1.is_bottom()) {
      inv2.assign_bool_var(stmt.lhs(), stmt.right(), !negate);
      m_inv = inv2;
    } else {
      inv1.assign_bool_var(stmt.lhs(), stmt.left(), !negate);
      inv2.assign_bool_var(stmt.lhs(), stmt.right(), !negate);
      m_inv = inv1 | inv2;
    }

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(bool_assert_t &stmt) {
    if (m_ignore_assert)
      return;

    m_inv.assume_bool(stmt.cond(), false);
  }

  void exec(havoc_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    m_inv.operator-=(stmt.variable());

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(unreach_t &stmt) { m_inv.set_to_bottom(); }

  void exec(arr_init_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    m_inv.array_init(stmt.array(), stmt.elem_size(), stmt.lb_index(),
                     stmt.ub_index(), stmt.val());

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(arr_store_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    auto new_arr_v = stmt.new_array();
    if (stmt.lb_index().equal(stmt.ub_index())) {
      if (new_arr_v) {
        m_inv.array_store(*new_arr_v, stmt.array(), stmt.elem_size(),
                          stmt.lb_index(), stmt.value(),
                          stmt.is_strong_update());
      } else {
        m_inv.array_store(stmt.array(), stmt.elem_size(), stmt.lb_index(),
                          stmt.value(), stmt.is_strong_update());
      }
    } else {
      if (new_arr_v) {
        m_inv.array_store_range(*new_arr_v, stmt.array(), stmt.elem_size(),
                                stmt.lb_index(), stmt.ub_index(), stmt.value());
      } else {
        m_inv.array_store_range(stmt.array(), stmt.elem_size(), stmt.lb_index(),
                                stmt.ub_index(), stmt.value());
      }
    }

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(arr_load_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    m_inv.array_load(stmt.lhs(), stmt.array(), stmt.elem_size(), stmt.index());

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(arr_assign_t &stmt) {
    bool pre_bot = false;
    if (::crab::CrabSanityCheckFlag) {
      pre_bot = m_inv.is_bottom();
    }

    m_inv.array_assign(stmt.lhs(), stmt.rhs());

    if (::crab::CrabSanityCheckFlag) {
      bool post_bot = m_inv.is_bottom();
      if (!(pre_bot || !post_bot)) {
        CRAB_ERROR("Invariant became bottom after ", stmt);
      }
    }
  }

  void exec(ptr_null_t &stmt) {
    m_inv.pointer_mk_null(stmt.lhs());
  }

  void exec(ptr_object_t &stmt) {
    m_inv.pointer_mk_obj(stmt.lhs(), stmt.rhs());
  }

  void exec(ptr_assign_t &stmt) {
    m_inv.pointer_assign(stmt.lhs(), stmt.rhs(), stmt.offset());
  }

  void exec(ptr_function_t &stmt) {
    m_inv.pointer_function(stmt.lhs(), stmt.rhs());
  }

  void exec(ptr_load_t &stmt) {
    m_inv.pointer_load(stmt.lhs(), stmt.rhs(), stmt.elem_size());
  }

  void exec(ptr_store_t &stmt) {
    m_inv.pointer_store(stmt.lhs(), stmt.rhs(), stmt.elem_size());
  }

  void exec(ptr_assume_t &stmt) {
    m_inv.pointer_assume(stmt.constraint());
  }

  void exec(ptr_assert_t &stmt) {
    if (m_ignore_assert)
      return;
    m_inv.pointer_assert(stmt.constraint());
  }

  std::string trim(const std::string &s){
    auto start = s.begin();
    while(start != s.end() && std::isspace(*start)){
      start++;
    }

    auto end = s.end();
    do{
      end--;
    }while(std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end+1);
  }

  std::vector<std::string> parse_disjunct_invariants(std::string s){
    std::vector<std::string> disjuncts;
    int index = 0;
    while(true){
      //Locate "or" to replace
      index = s.find("or", index);
      if(index == std::string::npos) break;

      //Make the replacement
      s.replace(index, 2, "O");

      //Advance index
      index += 1;
    }
    
    std::string item;
    std::istringstream outer(s);
    while(std::getline(outer, item, 'O')){
      item = trim(item);
      item = item.substr(1, item.size()-2);
      disjuncts.push_back(item);
    }

    return disjuncts;
  }

  int access_velocity_traversed_position(int a, int b, int c, int d){
    int velocity_to_traversed_positions[6][6][6][6] =
    {{{{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 1, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 1, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
     {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
     {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
     {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
     {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 0, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0}}},
     {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 0, 0}},
      {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 0}},
      {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 1}}}};

      return velocity_to_traversed_positions[a][b][c][d];
  }

  const char track[35][12] = {
{'x', 'x', 'x', 'x', 'x', 's', 's', 's', 's', 'x', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', 'x', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', 'x', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', 'x', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
{'g', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'},
{'g', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'},
{'g', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'},
};

  int is_valid_position_nointrinsic(int px, int py)
  {
    return (track[px][py]!='x' && px>=0 && px<35 && py>=0 && py<12);
  }

  int is_valid_velocity(int px, int py, int vx, int vy){
    if (vx > 5 || vy > 5){
      return 0;
    }

    int sign_vx = vx >= 0 ? 1 : -1;
    int sign_vy = vy >= 0 ? 1 : -1;
    int vx_abs = vx >= 0 ? vx : -vx;
    int vy_abs = vy >= 0 ? vy : -vy;

    for (int step_vx = 0; step_vx <= vx_abs; step_vx++){
      for (int step_vy = 0; step_vy <= vy_abs; step_vy++){
        if(access_velocity_traversed_position(vx_abs, vy_abs, step_vx, step_vy))
        {
          int tp_x = px + sign_vx * step_vx;
          int tp_y = py + sign_vy * step_vy;
          if (!is_valid_position_nointrinsic(tp_x, tp_y))
          {
            return 0;
          }
        }
      }
    }
    return 1;
  }
  

  int get_wall_distance(int px, int py, int vx, int vy){
    int distance = 0;

    while(is_valid_velocity(px, py, vx, vy))
    {
      px += vx;
      py += vy;
      distance ++;
    }

    return distance;
  }

  int is_valid_acceleration(int px, int py, int vx, int vy, int ax, int ay){
    vx += ax;
    vy += ay;
    return is_valid_velocity(px, py, vx, vy);
  }
  
  void exec(intrinsic_t &cs) {

    //Time logger intrinsic
    if(cs.get_intrinsic_name() == "logger"){
      
      AbsD pre_invs(m_inv);
      std::ofstream logg("/clam/custom_logging/logger.log", std::ofstream::app);
      time_t t;
      time(&t);
      pre_invs.project(cs.get_args());
      std::string c = pre_invs.to_linear_constraint_system().get_string();
      crab::outs() << "This log was generated by logger : " << c << "\n";
      logg << "Running logger intrinsic at : " << ctime(&t) << "\n";
      logg << "This log was generated by logger : " << c << "\n";
      logg.close();
    }
    //Handle deepsymbol call
    else if(cs.get_intrinsic_name() == "deepsymbol"){

      time_t t;
      time(&t);
      crab::outs() << "\nReached crab intrinsic " << cs.get_intrinsic_name() << " at " << ctime(&t);
    
      //Step 1 : Prepare input_box_int
      std::vector<var_t> args_list = cs.get_args();
      std::string call_st = cs.get_string();
      var_t var_ax = args_list[14];
      var_t var_ay = args_list[15];
      //Forget what we know about acceleration
      m_inv -= var_ax;
      m_inv -= var_ay;
      AbsD pre_invs(m_inv);

      std::map<std::string, int> llvmVar_featureIndex_map;
      //Populate the map
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int fv_index = 0;
      std::string item, llvmVar_ax, llvmVar_ay;
      while(std::getline(iss1, item, ',')){
        if(fv_index == 0){
          if(item.substr(0, item.find(":")) != "deepsymbol"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(fv_index <=14){
          llvmVar_featureIndex_map.insert(std::pair<std::string, int>(item.substr(0, item.find(":")), fv_index));
        }
        else if(fv_index==15){
          llvmVar_ax = item.substr(0, item.find(":"));
        }
        else if(fv_index==16){
          llvmVar_ay = item.substr(0, item.find(":"));
        }
        else{
          break;
        }
        fv_index++;
      }

      //Get Invariants
      // crab::outs() << "All invariants at entry : " << pre_invs << "\n";
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string invars = d_ct.get_string(); //get string from pre_invars
        if(invars.size() < 2 ){
          crab::outs() << "Malformed lin_cst string in intrinsic (check variable pre_invars)" << "\n";
          std::exit(1);
        }

        invars = invars.substr(1, invars.size()-2); //Stripped the braces
        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(invars);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }
      
        std::vector<std::pair<int, int>> input_box_int(14, std::make_pair(-999, -999)); // -999 is uninitialized

        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of the llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              if(llvmVar_featureIndex_map.find(item) != llvmVar_featureIndex_map.end()){
                fv_index = llvmVar_featureIndex_map.at(item);
                if(it[1] == "="){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2]);
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2]);
                }
                else if(it[1] == "<"){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2])+1;
                }
                else if(it[1] == "<="){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2]);
                }
                else if(it[1] == ">"){
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2])-1;
                }
                else if(it[1] == ">="){
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2]);
                }
                else{
                  crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                  exit(1);
                }
              }
            }
            else{
              if(llvmVar_featureIndex_map.find(item) != llvmVar_featureIndex_map.end()){
                fv_index = llvmVar_featureIndex_map.at(item);
                if(it[1] == "="){
                  input_box_int[fv_index-1].first = std::stoi(it[2]);
                  input_box_int[fv_index-1].second = std::stoi(it[2]);
                }
                else if(it[1] == "<"){
                  input_box_int[fv_index-1].second = std::stoi(it[2])-1;
                }
                else if(it[1] == "<="){
                  input_box_int[fv_index-1].second = std::stoi(it[2]);
                }
                else if(it[1] == ">"){
                  input_box_int[fv_index-1].first = std::stoi(it[2])+1;
                }
                else if(it[1] == ">="){
                  input_box_int[fv_index-1].first = std::stoi(it[2]);
                }
                else{
                  crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                  exit(1);
                }
              }
            }
          }
        }

        //Sanitize Input Box Int
        for(int i=0;i<14;i++){
          //position
          if(i == 0 || i ==1){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 34;
            }
          }
          else if(i == 2 || i ==3){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = -5;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 5;
            }
          }
          else if(i>3 && i<12){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 47;
            }
          }
          else if(i == 12 || i == 13){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 47;
            }
          }
        }

        //Step 2 : Make the call to DeepSymbol
        int fd1[2];
        int fd2[2];
        pid_t p;

        if(pipe(fd1) == -1 || pipe(fd2) == -1){                                          
          crab::outs() << "Could not create pipes." << "\n";                               
          exit(1);                                                                 
        }

        p = fork();                                                                  

        if(p < 0){                                                                   
          crab::outs() << "Fork failed." << "\n";                                          
          exit(1);                                                                                                                                                                                                                          
        }                                                                            
                                                                                  
        //Parent process - CRAB                                                      
        //Assuming we have formed our input in input_box_int                         
        else if(p > 0){                                                              
          close(fd1[0]); //close reading end of first pipe                         

          int arr[28];                                                             
          for(int i=0;i<14;i++){                                                   
            arr[2*i] = input_box_int[i].first;                                   
            arr[2*i+1] = input_box_int[i].second;                                
          }                                                                        

          //Write the input to the pipe and close writing end                      
          write(fd1[1], (const void* )(arr), 28*sizeof(int));                      
          close(fd1[1]); // close writing end of first pipe                        

          //Wait for child                                                         
          wait(NULL);                                                              

          close(fd2[1]); //close writing end of second pipe                        

          //Read from the input of second pipe                                     
          int out_size;                                                            
          read(fd2[0], (void *)(&out_size), sizeof(int));                          
          int *out_arr = (int *)(calloc(2*out_size, sizeof(int)));                   
          read(fd2[0], (void *)(out_arr), 2*out_size*sizeof(int));                 

          std::vector<std::pair<int, int>> output_box_int;                         
          for(int i=0;i<2*out_size;i+=2){                                            
            output_box_int.push_back(std::pair<int, int>(out_arr[i], out_arr[i+1]));  
          }                                                                        

          close(fd2[0]); //close reading end of second pipe   

          //Step 3 : Get the return value and make disjunctive_constraints                     
          //Print the output - will be converted to disjuncts                      
          int count=1;                                                             
          for(auto it: output_box_int){                                            
            count++;                                                             
          }

          //Create linear_constraint

          var_t pos_x = args_list[0];
          var_t pos_y = args_list[1];
          var_t vel_x = args_list[2];
          var_t vel_y = args_list[3];

          var_t ax = args_list[14];
          var_t ay = args_list[15];
          abs_dom_t boxes = abs_dom_t::bottom();

          for (auto &p: output_box_int) {
            abs_dom_t conjunction = abs_dom_t::top(); 
            lin_cst_t cst1(ax == number_t(p.first));
            lin_cst_t cst2(ay == number_t(p.second));
            lin_cst_t cst3(pos_x >= number_t(input_box_int[0].first));
            lin_cst_t cst4(pos_x <= number_t(input_box_int[0].second));
            lin_cst_t cst5(vel_x >= number_t(input_box_int[2].first));
            lin_cst_t cst6(vel_x <= number_t(input_box_int[2].second));
            lin_cst_t cst7(pos_y >= number_t(input_box_int[1].first));
            lin_cst_t cst8(pos_y <= number_t(input_box_int[1].second));
            lin_cst_t cst9(vel_y >= number_t(input_box_int[3].first));
            lin_cst_t cst10(vel_y >= number_t(input_box_int[3].second));
            conjunction += cst1;
            conjunction += cst2;
            conjunction += cst3;
            conjunction += cst4;
            conjunction += cst5; 
            conjunction += cst6;
            conjunction += cst7;
            conjunction += cst8;
            conjunction += cst9;
            conjunction += cst10;
            boxes |= conjunction;
          }

          //NOISE : we also add no-op if wall and goal distances are more than 3.
          int noise = 0;
          int OD = 3;
          for(int i=4;i<14;i++){
            if(input_box_int[i].first > OD && input_box_int[i].second > OD){
              continue;
            }
            else{
              noise = 0;
            }
          }

          //If velocity is 0 then we should not add noise
          if(input_box_int[2].first == 0 && input_box_int[2].second == 0 && input_box_int[3].first == 0 && input_box_int[3].second == 0){
            noise = 0;
          }

          if(noise){
            abs_dom_t conjunction = abs_dom_t::top(); 
            lin_cst_t cst1(ax == number_t(0));
            lin_cst_t cst2(ay == number_t(0));
            lin_cst_t cst3(pos_x >= number_t(input_box_int[0].first));
            lin_cst_t cst4(pos_x <= number_t(input_box_int[0].second));
            lin_cst_t cst5(vel_x >= number_t(input_box_int[2].first));
            lin_cst_t cst6(vel_x <= number_t(input_box_int[2].second));
            lin_cst_t cst7(pos_y >= number_t(input_box_int[1].first));
            lin_cst_t cst8(pos_y <= number_t(input_box_int[1].second));
            lin_cst_t cst9(vel_y >= number_t(input_box_int[3].first));
            lin_cst_t cst10(vel_y >= number_t(input_box_int[3].second));
            conjunction += cst1;
            conjunction += cst2;
            conjunction += cst3;
            conjunction += cst4;
            conjunction += cst5; 
            conjunction += cst6;
            conjunction += cst7;
            conjunction += cst8;
            conjunction += cst9;
            conjunction += cst10;
            boxes |= conjunction;
          }

          //END Noise

          crab::outs() << "Acceleration disjuncts : " << boxes << "\n\n\n";

          auto tmp1 = new_m_inv.get_content_domain();
          tmp1.project(cs.get_args());
          crab::outs() << "new_m_inv before updaate " << tmp1 << "\n";
          new_m_inv |= m_inv&boxes;
          
          tmp1 = new_m_inv.get_content_domain();
          tmp1.project(cs.get_args());
          crab::outs() << "new_m_inv after updaate " << tmp1 << "\n";

        }                                                                            
                                                                                   
        //Child process - after reqd manipulation, we exec to deepsymbol middleware  
        else{                                                                              
          close(fd1[1]); //close writing end of the first pipe                     
          close(fd2[0]); //close reading end of second pipe                        
                                                                                 
          //Read bound integers from first pipe                                    
          int *bounds_arr = (int *)(malloc(28*sizeof(int)));                       
                                                                                 
          read(fd1[0], (void *)(bounds_arr), 28*sizeof(int));                      
                                                                                 
          //Convert to char * for exec                                             
          //31 args to middleware: 1 path, 1 fd, 28 bounds, NULL                   
          char *args[31];                                                          
          args[0] = (char *)(malloc(43*sizeof(char)));                             
          args[0] = "/deepsymbol/middleware";                   
          args[1] = (char *)(malloc(sizeof(int)+sizeof(char)));                    
          sprintf(args[1], "%d\0", fd2[1]); //File descriptor for child process to write to
          for(int i=2; i<30; i++){                                                 
            args[i] = (char *)(malloc(sizeof(int)+sizeof(char)));                
            sprintf(args[i], "%d\0", bounds_arr[i-2]);                           
          }                                                                        
          args[30] = NULL;                                                         
                                                                                
          close(fd1[0]);                                                           
          execv(args[0], args);                                                    
          crab::outs() << "Failed to execute deepsymbol" << "\n";                          
          exit(1);                                                                                                                      
        }

      }

      m_inv = new_m_inv;
      time(&t);
      crab::outs() << "\nReached crab intrinsic " << cs.get_intrinsic_name() << " end at " << ctime(&t);
    }
    else if(cs.get_intrinsic_name() == "deepsymbol_lookahead"){
      time_t t;
      time(&t);
      crab::outs() << "\nReached crab intrinsic " << cs.get_intrinsic_name() << " at " << ctime(&t);

      //Step 1 : Prepare input_box_int
      std::vector<var_t> args_list = cs.get_args();
      std::string call_st = cs.get_string();
      var_t var_ax = args_list[14];
      var_t var_ay = args_list[15];
      //Forget what we know about acceleration
      m_inv -= var_ax;
      m_inv -= var_ay;
      AbsD pre_invs(m_inv);

      std::map<std::string, int> llvmVar_featureIndex_map;
      //Populate the map
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int fv_index = 0;
      std::string item, llvmVar_ax, llvmVar_ay;
      while(std::getline(iss1, item, ',')){
        if(fv_index == 0){
          if(item.substr(0, item.find(":")) != "deepsymbol_lookahead"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(fv_index <=14){
          llvmVar_featureIndex_map.insert(std::pair<std::string, int>(item.substr(0, item.find(":")), fv_index));
        }
        else if(fv_index==15){
          llvmVar_ax = item.substr(0, item.find(":"));
        }
        else if(fv_index==16){
          llvmVar_ay = item.substr(0, item.find(":"));
        }
        else{
          break;
        }
        fv_index++;
      }

      //Get Invariants
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string invars = d_ct.get_string(); //get string from pre_invars
        if(invars.size() < 2 ){
          crab::outs() << "Malformed lin_cst string in intrinsic (check variable pre_invars)" << "\n";
          std::exit(1);
        }

        invars = invars.substr(1, invars.size()-2); //Stripped the braces
        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(invars);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }
      
        std::vector<std::pair<int, int>> input_box_int(14, std::make_pair(-999, -999)); // -999 is uninitialized

        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of the llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              if(llvmVar_featureIndex_map.find(item) != llvmVar_featureIndex_map.end()){
                fv_index = llvmVar_featureIndex_map.at(item);
                if(it[1] == "="){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2]);
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2]);
                }
                else if(it[1] == "<"){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2])+1;
                }
                else if(it[1] == "<="){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2]);
                }
                else if(it[1] == ">"){
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2])-1;
                }
                else if(it[1] == ">="){
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2]);
                }
                else{
                  crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                  exit(1);
                }
              }
            }
            else{
              if(llvmVar_featureIndex_map.find(item) != llvmVar_featureIndex_map.end()){
                fv_index = llvmVar_featureIndex_map.at(item);
                if(it[1] == "="){
                  input_box_int[fv_index-1].first = std::stoi(it[2]);
                  input_box_int[fv_index-1].second = std::stoi(it[2]);
                }
                else if(it[1] == "<"){
                  input_box_int[fv_index-1].second = std::stoi(it[2])-1;
                }
                else if(it[1] == "<="){
                  input_box_int[fv_index-1].second = std::stoi(it[2]);
                }
                else if(it[1] == ">"){
                  input_box_int[fv_index-1].first = std::stoi(it[2])+1;
                }
                else if(it[1] == ">="){
                  input_box_int[fv_index-1].first = std::stoi(it[2]);
                }
                else{
                  crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                  exit(1);
                }
              }
            }
          }
        }

        //Sanitize Input Box Int
        for(int i=0;i<14;i++){
          //position
          if(i == 0 || i ==1){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 34;
            }
          }
          else if(i == 2 || i ==3){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = -5;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 5;
            }
          }
          else if(i>3 && i<12){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 47;
            }
          }
          else if(i == 12 || i == 13){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 47;
            }
          }
        }

        //Do this for each concretized state from input_box_int (has 14 feature values)
        // int loopv[14];

        //Step 2 : Make the call to DeepSymbol
        int fd1[2];
        int fd2[2];
        pid_t p;

        if(pipe(fd1) == -1 || pipe(fd2) == -1){                                          
          crab::outs() << "Could not create pipes." << "\n";                               
          exit(1);                                                                 
        }

        p = fork();                                                                  

        if(p < 0){                                                                   
          crab::outs() << "Fork failed." << "\n";                                          
          exit(1);                                                                                                                                                                                                                          
        }                                                                            
                                                                                  
        //Parent process - CRAB                                                      
        //Assuming we have formed our input in input_box_int                         
        else if(p > 0){                                                              
          close(fd1[0]); //close reading end of first pipe                         

          int arr[28];                                                             
          for(int i=0;i<14;i++){                                                   
            arr[2*i] = input_box_int[i].first;                                   
            arr[2*i+1] = input_box_int[i].second;                                
          }                                                                        

          //Write the input to the pipe and close writing end                      
          write(fd1[1], (const void* )(arr), 28*sizeof(int));                      
          close(fd1[1]); // close writing end of first pipe                        

          //Wait for child                                                         
          wait(NULL);                                                              

          close(fd2[1]); //close writing end of second pipe                        

          //Read from the input of second pipe                                     
          int out_size;                                                            
          read(fd2[0], (void *)(&out_size), sizeof(int));                          
          int *out_arr = (int *)(calloc(2*out_size, sizeof(int)));                   
          read(fd2[0], (void *)(out_arr), 2*out_size*sizeof(int));                 

          std::vector<std::pair<int, int>> output_box_int;                         
          for(int i=0;i<2*out_size;i+=2){                                            
            output_box_int.push_back(std::pair<int, int>(out_arr[i], out_arr[i+1]));  
          }                                                                        

          close(fd2[0]); //close reading end of second pipe   

          //Step 3 : Get the return value and make disjunctive_constraints                     
          //Print the output - will be converted to disjuncts                      
          int count=1;                                                             
          for(auto it: output_box_int){                                            
            count++;                                                             
          }

          //Create linear_constraint

          var_t pos_x = args_list[0];
          var_t pos_y = args_list[1];
          var_t vel_x = args_list[2];
          var_t vel_y = args_list[3];

          var_t ax = args_list[14];
          var_t ay = args_list[15];
          abs_dom_t boxes = abs_dom_t::bottom();

          for (auto &p: output_box_int) {
            abs_dom_t conjunction = abs_dom_t::top(); 
            lin_cst_t cst1(ax == number_t(p.first));
            lin_cst_t cst2(ay == number_t(p.second));
            lin_cst_t cst3(pos_x >= number_t(input_box_int[0].first));
            lin_cst_t cst4(pos_x <= number_t(input_box_int[0].second));
            lin_cst_t cst5(vel_x >= number_t(input_box_int[2].first));
            lin_cst_t cst6(vel_x <= number_t(input_box_int[2].second));
            lin_cst_t cst7(pos_y >= number_t(input_box_int[1].first));
            lin_cst_t cst8(pos_y <= number_t(input_box_int[1].second));
            lin_cst_t cst9(vel_y >= number_t(input_box_int[3].first));
            lin_cst_t cst10(vel_y >= number_t(input_box_int[3].second));
            conjunction += cst1;
            conjunction += cst2;
            conjunction += cst3;
            conjunction += cst4;
            conjunction += cst5; 
            conjunction += cst6;
            conjunction += cst7;
            conjunction += cst8;
            conjunction += cst9;
            conjunction += cst10;
            boxes |= conjunction;
          }

          new_m_inv |= m_inv&boxes;
        }                                                                            
                                                                                   
        //Child process - after reqd manipulation, we exec to deepsymbol middleware  
        else{                                                                              
          close(fd1[1]); //close writing end of the first pipe                     
          close(fd2[0]); //close reading end of second pipe                        
                                                                                 
          //Read bound integers from first pipe                                    
          int *bounds_arr = (int *)(malloc(28*sizeof(int)));                       
                                                                                 
          read(fd1[0], (void *)(bounds_arr), 28*sizeof(int));                      
                                                                                 
          //Convert to char * for exec                                             
          //31 args to middleware: 1 path, 1 fd, 28 bounds, NULL                   
          char *args[31];                                                          
          args[0] = (char *)(malloc(43*sizeof(char)));                             
          args[0] = "/deepsymbol/middleware";                   
          args[1] = (char *)(malloc(sizeof(int)+sizeof(char)));                    
          sprintf(args[1], "%d\0", fd2[1]); //File descriptor for child process to write to
          for(int i=2; i<30; i++){                                                 
            args[i] = (char *)(malloc(sizeof(int)+sizeof(char)));                
            sprintf(args[i], "%d\0", bounds_arr[i-2]);                           
          }                                                                        
          args[30] = NULL;                                                         
                                                                                
          close(fd1[0]);                                                           
          execv(args[0], args);                                                    
          crab::outs() << "Failed to execute deepsymbol" << "\n";                          
          exit(1);                                                                                                                      
        }


      }

      m_inv = new_m_inv;
      time(&t);
      crab::outs() << "\nReached crab intrinsic " << cs.get_intrinsic_name() << " end at " << ctime(&t);
    }
    else if(cs.get_intrinsic_name() == "eran"){

      time_t t;
      time(&t);
      crab::outs() << "\nReached crab intrinsic " << cs.get_intrinsic_name() << " at " << ctime(&t);
      //Handle eran call
      //Step 1 : Prepare input_box_int
      std::vector<var_t> args_list = cs.get_args();
      std::string call_st = cs.get_string();
      var_t var_ax = args_list[14];
      var_t var_ay = args_list[15];
      //Forget what we know about acceleration
      m_inv -= var_ax;
      m_inv -= var_ay;
      AbsD pre_invs(m_inv);

      std::map<std::string, int> llvmVar_featureIndex_map;
      //Populate the map
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int fv_index = 0;
      std::string item, llvmVar_ax, llvmVar_ay;
      while(std::getline(iss1, item, ',')){
        if(fv_index == 0){
          if(item.substr(0, item.find(":")) != "eran"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(fv_index <=14){
          llvmVar_featureIndex_map.insert(std::pair<std::string, int>(item.substr(0, item.find(":")), fv_index));
        }
        else if(fv_index==15){
          llvmVar_ax = item.substr(0, item.find(":"));
        }
        else if(fv_index==16){
          llvmVar_ay = item.substr(0, item.find(":"));
        }
        else{
          break;
        }
        fv_index++;
      }

      //Get Invariants
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string invars = d_ct.get_string(); //get string from pre_invars
        if(invars.size() < 2 ){
          crab::outs() << "Malformed lin_cst string in intrinsic (check variable pre_invars)" << "\n";
          std::exit(1);
        }

        invars = invars.substr(1, invars.size()-2); //Stripped the braces
        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(invars);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }
      
        std::vector<std::pair<int, int>> input_box_int(14, std::make_pair(-999, -999)); // -999 is uninitiazlied

        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of the llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              if(llvmVar_featureIndex_map.find(item) != llvmVar_featureIndex_map.end()){
                fv_index = llvmVar_featureIndex_map.at(item);
                if(it[1] == "="){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2]);
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2]);
                }
                else if(it[1] == "<"){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2])+1;
                }
                else if(it[1] == "<="){
                  input_box_int[fv_index-1].first = -1*std::stoi(it[2]);
                }
                else if(it[1] == ">"){
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2])-1;
                }
                else if(it[1] == ">="){
                  input_box_int[fv_index-1].second = -1*std::stoi(it[2]);
                }
                else{
                  crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                  exit(1);
                }
              }
            }
            else{
              if(llvmVar_featureIndex_map.find(item) != llvmVar_featureIndex_map.end()){
                fv_index = llvmVar_featureIndex_map.at(item);
                if(it[1] == "="){
                  input_box_int[fv_index-1].first = std::stoi(it[2]);
                  input_box_int[fv_index-1].second = std::stoi(it[2]);
                }
                else if(it[1] == "<"){
                  input_box_int[fv_index-1].second = std::stoi(it[2])-1;
                }
                else if(it[1] == "<="){
                  input_box_int[fv_index-1].second = std::stoi(it[2]);
                }
                else if(it[1] == ">"){
                  input_box_int[fv_index-1].first = std::stoi(it[2])+1;
                }
                else if(it[1] == ">="){
                  input_box_int[fv_index-1].first = std::stoi(it[2]);
                }
                else{
                  crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                  exit(1);
                }
              }
            }
          }
        }

        //Sanitize Input Box Int
        for(int i=0;i<14;i++){
          //position
          if(i == 0 || i ==1){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 34;
            }
          }
          else if(i == 2 || i ==3){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = -5;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 5;
            }
          }
          else if(i>3 && i<12){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 47;
            }
          }
          else if(i == 12 || i == 13){
            if(input_box_int[i].first == -999){
              input_box_int[i].first = 0;
            }
            if(input_box_int[i].second == -999){
              input_box_int[i].second = 47;
            }
          }
        }

        //Do this for each concretized state from input_box_int (has 14 feature values)
        int loopv[14];

        //Step 2 : Make the call to ERAN
        int fd1[2];
        int fd2[2];
        pid_t p;

        if(pipe(fd1) == -1 || pipe(fd2) == -1){                                          
          crab::outs() << "Could not create pipes." << "\n";                               
          exit(1);                                                                 
        }

        p = fork();                                                                  

        if(p < 0){                                                                   
          crab::outs() << "Fork failed." << "\n";                                          
          exit(1);                                                                                                                                                                                                                          
        }                                                                            
                                                                                  
        //Parent process - CRAB                                                      
        //Assuming we have formed our input in input_box_int                         
        else if(p > 0){                                                              
          close(fd1[0]); //close reading end of first pipe                         

          int arr[28];                                                             
          for(int i=0;i<14;i++){                                                   
            arr[2*i] = input_box_int[i].first;                                   
            arr[2*i+1] = input_box_int[i].second;                                
          }                                                                        

          //Write the input to the pipe and close writing end                      
          write(fd1[1], (const void* )(arr), 28*sizeof(int));                      
          close(fd1[1]); // close writing end of first pipe                        

          //Wait for child                                                         
          wait(NULL);                                                              

          close(fd2[1]); //close writing end of second pipe                        

          //Read from the input of second pipe                                     
          int out_size;                                                            
          read(fd2[0], (void *)(&out_size), sizeof(int));                          
          int *out_arr = (int *)(calloc(2*out_size, sizeof(int)));                   
          read(fd2[0], (void *)(out_arr), 2*out_size*sizeof(int));                 

          std::vector<std::pair<int, int>> output_box_int;                         
          for(int i=0;i<2*out_size;i+=2){                                            
            output_box_int.push_back(std::pair<int, int>(out_arr[i], out_arr[i+1]));  
          }                                                                        

          close(fd2[0]); //close reading end of second pipe   

          //Step 3 : Get the return value and make disjunctive_constraints                     
          //Print the output - will be converted to disjuncts                      
          int count=1;                                                             
          for(auto it: output_box_int){                                            
            count++;                                                             
          }

          //Create linear_constraint

          var_t pos_x = args_list[0];
          var_t pos_y = args_list[1];
          var_t vel_x = args_list[2];
          var_t vel_y = args_list[3];

          var_t ax = args_list[14];
          var_t ay = args_list[15];
          abs_dom_t boxes = abs_dom_t::bottom();

          for (auto &p: output_box_int) {
            abs_dom_t conjunction = abs_dom_t::top(); 
            lin_cst_t cst1(ax == number_t(p.first));
            lin_cst_t cst2(ay == number_t(p.second));
            lin_cst_t cst3(pos_x >= number_t(input_box_int[0].first));
            lin_cst_t cst4(vel_x >= number_t(input_box_int[2].first));
            lin_cst_t cst5(pos_y >= number_t(input_box_int[1].first));
            lin_cst_t cst6(vel_y >= number_t(input_box_int[3].first));
            lin_cst_t cst7(pos_x <= number_t(input_box_int[0].second));
            lin_cst_t cst8(vel_x <= number_t(input_box_int[2].second));
            lin_cst_t cst9(pos_y <= number_t(input_box_int[1].second));
            lin_cst_t cst10(vel_y <= number_t(input_box_int[3].second));
            conjunction += cst1;
            conjunction += cst2;
            conjunction += cst3;
            conjunction += cst4;
            conjunction += cst5; 
            conjunction += cst6;
            conjunction += cst7;
            conjunction += cst8;
            conjunction += cst9;
            conjunction += cst10;
            boxes |= conjunction;
          }

          //NOISE : we also add no-op if wall and goal distances are more than 3.
          int noise = 0;
          for(int i=4;i<14;i++){
            if(input_box_int[i].first >= 4 && input_box_int[i].second >= 4){
              continue;
            }
            else{
              noise = 0;
            }
          }

          //If velocity is 0 then we should not add noise
          if(input_box_int[2].first == 0 && input_box_int[2].second == 0 && input_box_int[3].first == 0 && input_box_int[3].second == 0){
            noise = 0;
          }

          if(noise){
            abs_dom_t conjunction = abs_dom_t::top(); 
            lin_cst_t cst1(ax == number_t(0));
            lin_cst_t cst2(ay == number_t(0));
            lin_cst_t cst3(pos_x >= number_t(input_box_int[0].first));
            lin_cst_t cst4(pos_x <= number_t(input_box_int[0].second));
            lin_cst_t cst5(vel_x >= number_t(input_box_int[2].first));
            lin_cst_t cst6(vel_x <= number_t(input_box_int[2].second));
            lin_cst_t cst7(pos_y >= number_t(input_box_int[1].first));
            lin_cst_t cst8(pos_y <= number_t(input_box_int[1].second));
            lin_cst_t cst9(vel_y >= number_t(input_box_int[3].first));
            lin_cst_t cst10(vel_y >= number_t(input_box_int[3].second));
            conjunction += cst1;
            conjunction += cst2;
            conjunction += cst3;
            conjunction += cst4;
            conjunction += cst5; 
            conjunction += cst6;
            conjunction += cst7;
            conjunction += cst8;
            conjunction += cst9;
            conjunction += cst10;
            boxes |= conjunction;
          }

          //END Noise
          auto tmp1 = new_m_inv.get_content_domain();
          tmp1.project(cs.get_args());
          new_m_inv |= m_inv&boxes;
          
          tmp1 = new_m_inv.get_content_domain();
          tmp1.project(cs.get_args());
        }                                                                            
                                                                                   
        //Child process - after reqd manipulation, we exec to deepsymbol middleware  
        else{                                                                              
          close(fd1[1]); //close writing end of the first pipe                     
          close(fd2[0]); //close reading end of second pipe                        
                                                                                 
          //Read bound integers from first pipe                                    
          int *bounds_arr = (int *)(malloc(28*sizeof(int)));                       
                                                                                 
          read(fd1[0], (void *)(bounds_arr), 28*sizeof(int));                      
                                                                                 
          //Convert to char * for exec                                             
          //31 args to middleware: 1 path, 1 fd, 28 bounds, NULL                   
          char *args[31];                                                          
          args[0] = (char *)(malloc(43*sizeof(char)));                             
          args[0] = "/eran-middleware/middleware";                   
          args[1] = (char *)(malloc(sizeof(int)+sizeof(char)));                    
          sprintf(args[1], "%d\0", fd2[1]); //File descriptor for child process to write to
          for(int i=2; i<30; i++){                                                 
            args[i] = (char *)(malloc(sizeof(int)+sizeof(char)));                
            sprintf(args[i], "%d\0", bounds_arr[i-2]);                           
          }                                                                        
          args[30] = NULL;                                                         
                                                                                
          close(fd1[0]);                                                           
          execv(args[0], args);                                                    
          crab::outs() << "Failed to execute eran" << "\n";                          
          exit(1);                                                                                                                      
        }
      }

      m_inv = new_m_inv;
      time(&t);
      crab::outs() << "\nReached crab intrinsic " << cs.get_intrinsic_name() << " end at " << ctime(&t);
    }
    else if(cs.get_intrinsic_name() == "print_invariants"){
      AbsD pre_invs(m_inv);
      crab::outs() << "Invariants at this point : " << pre_invs << "\n";
      crab::outs() << "This is disjunctive lin cst " << m_inv.to_disjunctive_linear_constraint_system() << "\n";
      crab::outs() << "This is lin cst " << m_inv.to_linear_constraint_system() << "\n\n";

    }
    else if(cs.get_intrinsic_name() == "print_invariants_to_variables"){
      auto pre_invs = m_inv.get_content_domain();
      pre_invs.project(cs.get_args());
      crab::outs() << "Call statement for var_map " << cs.get_string() << "\n";
      crab::outs() << "Invariants projected to variables " << pre_invs << "\n";
    }
    else if(cs.get_intrinsic_name() == "print_state_invariants"){
      auto pre_invs = m_inv.get_content_domain();
      pre_invs.project(cs.get_args());
      crab::outs() << "Call statement for var_map " << cs.get_string() << "\n";
      crab::outs() << "Invariants projected to state " << pre_invs << "\n";
    }
    else if(cs.get_intrinsic_name().find("print_invariants_generic") != std::string::npos){
      auto pre_invs = m_inv.get_content_domain();
      pre_invs.project(cs.get_args());
      crab::outs() << "Call statement for var_map " << cs.get_string() << "\n";
      crab::outs() << "Invariants projected " << pre_invs << "\n";
    }
    else if(cs.get_intrinsic_name() == "access_map"){
      //Map
      int BS_MAP[35][12] = {
        {'x', 'x', 'x', 'x', 'x', 's', 's', 's', 's', 'x', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', 'x', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', 'x', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', 'x', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', 'x', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', 'x'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'x', 'x', 'x', 'x', 'x', '.', '.', '.', '.', '.', '.', '.'},
        {'g', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'},
        {'g', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'},
        {'g', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.'},
      };

      std::vector<var_t> args_list = cs.get_args();
      //Forget anything we know about z
      var_t pos_x = args_list[0];
      var_t pos_y = args_list[1];
      var_t z = args_list[2];
      m_inv -= z;

      AbsD pre_invs(m_inv);
      std::vector<var_t> position_vars;
      position_vars.push_back(args_list[0]);
      position_vars.push_back(args_list[1]);

      //get string name of position x,y llvmVars
      std::string call_st = cs.get_string();
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int index=0;
      std::string llvmVar_posx, llvmVar_posy, llvmVar_return;
      std::string item;
      while(std::getline(iss1, item, ',')){
        if(index == 0){
          if(item.substr(0, item.find(":")) != "access_map"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(index == 1){
          llvmVar_posx = item.substr(0, item.find(":"));
        }
        else if(index == 2){
          llvmVar_posy = item.substr(0, item.find(":"));
        }
        else if(index == 3){
          llvmVar_return = item.substr(0, item.find(":"));
        }
        else{
          crab::outs() << "More than expected arguements passed" << "\n";
          std::exit(1);
        }
        index++;
      }


      //Get linear constraints on the position_vars
      //Get a list of the disjuncts separately
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();
      
      //Iterate over each disjunct in pre_invs
      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string position_bounds = d_ct.get_string();
        position_bounds = position_bounds.substr(1, position_bounds.size()-2);
        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(position_bounds);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }

        std::vector<std::pair<int, int>> input_box_int(2, std::make_pair(0, 0));
        int position_bound_exist[4] = {0,0,0,0};
        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of the llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              if(item == llvmVar_posx){
                index = 0;
              }
              else if(item == llvmVar_posy){
                index = 1;
              }
              else{
                continue;
              }

              if(it[1] == "="){
                input_box_int[index].first = -1*std::stoi(it[2]);
                input_box_int[index].second = -1*std::stoi(it[2]);
                position_bound_exist[2*index] = 1;
                position_bound_exist[2*index+1] = 1;
              }
              else if(it[1] == "<"){
                input_box_int[index].first = -1*std::stoi(it[2])+1;
                position_bound_exist[2*index] = 1;
              }
              else if(it[1] == "<="){
                input_box_int[index].first = -1*std::stoi(it[2]);
                position_bound_exist[2*index] = 1;
              }
              else if(it[1] == ">"){
                input_box_int[index].second = -1*std::stoi(it[2])-1;
                position_bound_exist[2*index+1] = 1;
              }
              else if(it[1] == ">="){
                input_box_int[index].second = -1*std::stoi(it[2]);
                position_bound_exist[2*index+1] = 1;
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
            else{ //This is a positive constraint
              if(item == llvmVar_posx){
                index = 0;
              }
              else if(item == llvmVar_posy){
                index = 1;
              }
              else{
                continue;
              }

              if(it[1] == "="){
                input_box_int[index].first = std::stoi(it[2]);
                input_box_int[index].second = std::stoi(it[2]);
                position_bound_exist[2*index] = 1;
                position_bound_exist[2*index+1] = 1;
              }
              else if(it[1] == "<"){
                input_box_int[index].second = std::stoi(it[2])-1;
                position_bound_exist[2*index+1] = 1;
              }
              else if(it[1] == "<="){
                input_box_int[index].second = std::stoi(it[2]);
                position_bound_exist[2*index+1] = 1;
              }
              else if(it[1] == ">"){
                input_box_int[index].first = std::stoi(it[2])+1;
                position_bound_exist[2*index] = 1;
              }
              else if(it[1] == ">="){
                input_box_int[index].first = std::stoi(it[2]);
                position_bound_exist[2*index] = 1;
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
          }
        }
        //TODO:sanity check

        //Access the map to check what are possible values
        std::vector<int> possible_map_locations;
        for(int i=input_box_int[0].first; i<=input_box_int[0].second; i++){
          if(i<0){
            continue;
          }
          else if(i>34){
            break;
          }
          
          for(int j=input_box_int[1].first; j<=input_box_int[1].second; j++){
            //if (i,j) are allowed in the invariants
            if( j<0 ){
              continue;
            }
            else if(j>11){
              break;
            }
            else if(std::find(possible_map_locations.begin(), possible_map_locations.end(), BS_MAP[i][j]) != possible_map_locations.end()){
              continue;
            }
            else{
              possible_map_locations.push_back(BS_MAP[i][j]);
            }
          }
        }
      
        //Push llvmVar_return into invariants
        abs_dom_t boxes = abs_dom_t::bottom();

        for (auto p: possible_map_locations) { 
          abs_dom_t conjunction = abs_dom_t::top(); 
          lin_cst_t cst(z == number_t(p));
          conjunction += cst;
          if(position_bound_exist[0]){
            lin_cst_t cst3(pos_x >= number_t(input_box_int[0].first));
            conjunction += cst3;
          }
          if(position_bound_exist[1]){
            lin_cst_t cst4(pos_x <= number_t(input_box_int[0].second));
            conjunction += cst4;
          }
          if(position_bound_exist[2]){
            lin_cst_t cst5(pos_y >= number_t(input_box_int[1].first));
            conjunction += cst5;
          }
          if(position_bound_exist[3]){
            lin_cst_t cst6(pos_y <= number_t(input_box_int[1].second));
            conjunction += cst6;
          }
          
          boxes |= conjunction;
        }

        new_m_inv |= m_inv&boxes;
      }

      m_inv = new_m_inv;

      AbsD tmp(m_inv);
      std::vector<var_t> tempo;
      tempo.push_back(args_list[2]);
      tmp.project(cs.get_args());

    }
    else if(cs.get_intrinsic_name() == "execute_acceleration"){
      AbsD pre_inv(m_inv);

      std::vector<var_t> args_list = cs.get_args();
      //Forget what we know about the next state
      for(int i=6;i<10;i++){
        m_inv -= args_list[i];
      }

      // get string name of input variables
      std::string call_st = cs.get_string();
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int index = 0;
      std::vector<std::string> llvm_Vars;
      std::string item;
      while(std::getline(iss1, item, ',')){
        if(index == 0){
          if(item.substr(0, item.find(":")) != "execute_acceleration"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(index >=1 && index <=10){
          llvm_Vars.push_back(item.substr(0, item.find(":")));
        }
        else{
          crab::outs() << "More than expected " << index << " arguements passed" << "\n";
          std::exit(1);
        }
        index++;
      }

      //Get the Invariants
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      //Iterate over each disjunct in djct_csts
      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string invars = d_ct.get_string(); //get string of preconds invariants
        if(invars.size() < 2){
          crab::outs() << "Malformed lin_cst string in intrinsic (check variable pre_invars)" << "\n";
          std::exit(1);
        }

        invars = invars.substr(1, invars.size()-2); //Stripped braces

        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(invars);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }

        std::vector<std::pair<int, int>> input_bounds(6, std::make_pair(-999,-999)); // -999 is uninitialized
        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting\n";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = -1*std::stoi(it[2]);
                input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].first = -1*std::stoi(it[2])+1;
              }
              else if(it[1] == "<="){
                input_bounds[index].first = -1*std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].second = -1*std::stoi(it[2])-1;
              }
              else if(it[1] == ">="){
               input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
            else{
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = std::stoi(it[2]);
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].second = std::stoi(it[2])-1;
              }
              else if(it[1] == "<="){
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].first = std::stoi(it[2])+1;
              }
              else if(it[1] == ">="){
               input_bounds[index].first = std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
          }
        }

        //Sanitize input_bounds if uninitialized
        for(int i=0;i<6;i++){
          if(i==0||i==1){
            if(input_bounds[i].first == -999){
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -999){
              input_bounds[i].second = 34;
            }
          }
          else if(i==2||i==3){
            if(input_bounds[i].first == -999){
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -999){
              input_bounds[i].second = 5;
            }
          }
          else{
            if(input_bounds[i].first == -999){
              input_bounds[i].first = -1;
            }
            if(input_bounds[i].second == -999){
              input_bounds[i].second = 1;
            }
          }   
        }

        //Create "shadow" invariant to get concerned invariant object from m_inv
        abs_dom_t shadow = abs_dom_t::top();
        var_t pos_x = args_list[0];
        var_t pos_y = args_list[1];
        var_t vel_x = args_list[2];
        var_t vel_y = args_list[3];
        var_t acc_x = args_list[4];
        var_t acc_y = args_list[5];
        lin_cst_t cst[12];
        for(int i=0;i <6; i++){
          var_t v = args_list[i];
          cst[2*i] = lin_cst_t(v >= number_t(input_bounds[i].first));
          cst[2*i+1] = lin_cst_t(v <= number_t(input_bounds[i].second));
        }
        for(int i=0;i<12;i++){
          shadow += cst[i];
        }

        abs_dom_t disjunct = abs_dom_t::bottom();
        abs_dom_t conjunct = abs_dom_t::bottom();
        disjunct = m_inv&shadow;

        //Concretize input_bounds in loops and for each concrete value, compute concrete output
        for(int px=input_bounds[0].first; px<=input_bounds[0].second;px++){
          for(int py=input_bounds[1].first; py<=input_bounds[1].second;py++){
            for(int vx=input_bounds[2].first; vx<=input_bounds[2].second;vx++){
              for(int vy=input_bounds[3].first; vy<=input_bounds[3].second;vy++){
                for(int ax=input_bounds[4].first; ax<=input_bounds[4].second;ax++){
                  for(int ay=input_bounds[5].first; ay<=input_bounds[5].second;ay++){

                    int new_vals[6];
                    new_vals[0] = px;
                    new_vals[1] = py;
                    new_vals[2] = vx;
                    new_vals[3] = vy;
                    new_vals[4] = ax;
                    new_vals[5] = ay;

                    //Now I have to execute the acceleration here
                    if(!is_valid_acceleration(px, py, vx, vy, ax, ay)){
                      crab::outs() << "Invalid State : " << px << " " << py << " " << vx << " " << vy << " " << ax << " " << ay <<"\n";
                      new_vals[0] = -100;
                      new_vals[1] = -100;
                      new_vals[2] = -100;
                      new_vals[3] = -100;
                    }
                    else{
                      new_vals[2] += ax;
                      new_vals[3] += ay;
                      new_vals[0] += new_vals[2];
                      new_vals[1] += new_vals[3];
                    }

                    lin_cst_t new_cst[4];
                    for(int i=0;i <4; i++){
                      var_t v = args_list[i+6];
                      new_cst[i] = lin_cst_t(v == number_t(new_vals[i]));
                    }

                    abs_dom_t conjunction = abs_dom_t::top();
                    for(int i=0;i<4;i++){
                      conjunction += new_cst[i];
                    }

                    conjunct |= conjunction;
                  }
                }
              }
            }
          }
        }

        new_m_inv |= disjunct&conjunct;
      }
      
      m_inv = new_m_inv;
      auto tmp1 = m_inv.get_content_domain();
      tmp1.project(cs.get_args());
    }
    else if(cs.get_intrinsic_name() == "access_velocity_traversed_position"){
      int velocity_to_traversed_positions[6][6][6][6] ={
        {{{1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 1, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 1, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 1, 1, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
        {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
        {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
        {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}}},
        {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 0, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 0}, {0, 0, 0, 0, 0, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 0}}},
        {{{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 0, 0}},
        {{1, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 0}},
        {{1, 1, 0, 0, 0, 0}, {0, 1, 1, 0, 0, 0}, {0, 0, 1, 1, 0, 0}, {0, 0, 0, 1, 1, 0}, {0, 0, 0, 0, 1, 1}, {0, 0, 0, 0, 0, 1}}}
      };

      AbsD pre_invs(m_inv);
      std::vector<var_t> args_list = cs.get_args();
      std::vector<var_t> velocity_vars;
      velocity_vars.push_back(args_list[0]);
      velocity_vars.push_back(args_list[1]);
      velocity_vars.push_back(args_list[2]);
      velocity_vars.push_back(args_list[3]);

      //get string name of vx, vy, steps
      std::string call_st = cs.get_string();
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int index=0;
      std::string llvmVar_vx, llvmVar_vy, llvmVar_stepvx, llvmVar_stepvy,llvmVar_return;
      std::string item;
      while(std::getline(iss1, item, ',')){
        if(index == 0){
          if(item.substr(0, item.find(":")) != "access_velocity_traversed_position"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(index == 1){
          llvmVar_vx = item.substr(0, item.find(":"));
        }
        else if(index == 2){
          llvmVar_vy = item.substr(0, item.find(":"));
        }
        else if(index == 3){
          llvmVar_stepvx = item.substr(0, item.find(":"));
        }
        else if(index == 4){
          llvmVar_stepvy = item.substr(0, item.find(":"));
        }
        else if(index == 5){
          llvmVar_return = item.substr(0, item.find(":"));
        }
        else{
          crab::outs() << "More than expected arguements passed" << "\n";
          std::exit(1);
        }
        index++;
      }

      //Get linear constraints on the position_vars
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string velocity_bounds = d_ct.get_string();
        velocity_bounds = velocity_bounds.substr(1, velocity_bounds.size()-2);
        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(velocity_bounds);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }

        std::vector<std::pair<int, int>> input_box_int(4, std::make_pair(0, 5));
        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of the llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              if(item == llvmVar_vx){
                index = 0;
              }
              else if(item == llvmVar_vy){
                index = 1;
              }
              else if(item == llvmVar_stepvx){
                index = 2;
              }
              else if(item == llvmVar_stepvy){
                index = 3;
              }
              else{
                continue;
              }

              if(it[1] == "="){
                input_box_int[index].first = -1*std::stoi(it[2]);
                input_box_int[index].second = -1*std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_box_int[index].first = -1*std::stoi(it[2])+1;
              }
              else if(it[1] == "<="){
                input_box_int[index].first = -1*std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_box_int[index].second = -1*std::stoi(it[2])-1;
              }
              else if(it[1] == ">="){
                input_box_int[index].second = -1*std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
            else{ //This is a positive constraint
              if(item == llvmVar_vx){
                index = 0;
              }
              else if(item == llvmVar_vy){
                index = 1;
              }
              else if(item == llvmVar_stepvx){
                index = 2;
              }
              else if(item == llvmVar_stepvy){
                index = 3;
              }
              else{
                continue;
              }

              if(it[1] == "="){
                input_box_int[index].first = std::stoi(it[2]);
                input_box_int[index].second = std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_box_int[index].second = std::stoi(it[2])-1;
              }
              else if(it[1] == "<="){
                input_box_int[index].second = std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_box_int[index].first = std::stoi(it[2])+1;
              }
              else if(it[1] == ">="){
                input_box_int[index].first = std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
          }
        }

        //TODO : sanity check
        //Access the array to check what are possible return values
        std::vector<int> possible_returns;
        int ret;
        int yes=0, no=0;
        for(int i=input_box_int[0].first; i<=input_box_int[0].second; i++){
          for(int j=input_box_int[1].first; j<=input_box_int[1].second; j++){
            for(int k=input_box_int[2].first; k<= input_box_int[2].second; k++){
              for(int l=input_box_int[3].first; l<= input_box_int[3].second; l++){
                if(velocity_to_traversed_positions[i][j][k][l]){
                  yes++;
                }
                else{
                  no++;
                }
              }
            }
          }
        }

        if(yes){
          possible_returns.push_back(1);
        }
        if(no){
          possible_returns.push_back(0);
        }

        //Push llvmVar_return into invariants
        abs_dom_t boxes = abs_dom_t::bottom();
        var_t ret_value = args_list[4];

        for (auto p: possible_returns) { 
          abs_dom_t conjunction = abs_dom_t::top(); 
          lin_cst_t cst(ret_value == number_t(p));
          conjunction += cst; 
          boxes |= conjunction;
        }

        abs_dom_t dct = abs_dom_t::top();
        dct += d_ct;
        new_m_inv |= dct&boxes;

      }

      m_inv = new_m_inv;
      AbsD tmp(m_inv);
      std::vector<var_t> tempo;
      tempo.push_back(args_list[4]);
      tmp.project(tempo);
    }
    else if(cs.get_intrinsic_name() == "dummy_one"){
      AbsD pre_invs(m_inv);
      crab::outs() << "Invariants at entry : " << pre_invs.to_linear_constraint_system().get_string() << "\n";
      std::vector<var_t> args_list = cs.get_args();
      var_t z = args_list[2];
      int p = 'x';
      abs_dom_t boxes = abs_dom_t::bottom();
      abs_dom_t conjunction = abs_dom_t::top();
      lin_cst_t cst(z == number_t(p));
      conjunction += cst;
      boxes |= conjunction;
      crab::outs() << "This is boxes : " << boxes.to_linear_constraint_system().get_string() << "\n";
      m_inv |= boxes;
      crab::outs() << "This is m_inv : " << m_inv.to_linear_constraint_system().get_string() << "\n"; 
    }
    else if(cs.get_intrinsic_name() == "dummy_two"){
      AbsD pre_invs(m_inv);
      crab::outs() << "Invariants at entry : " << pre_invs.to_linear_constraint_system().get_string() << "\n";
      std::vector<var_t> args_list = cs.get_args();
      var_t z = args_list[2];
      int p = 'x';
      m_inv.assign(z, number_t(p));
      crab::outs() << "This is m_inv : " << m_inv.to_linear_constraint_system().get_string() << "\n"; 
    }
    else if(cs.get_intrinsic_name() == "dummy_three"){
      AbsD pre_invs(m_inv);
      crab::outs() << "Invariants at entry : " << pre_invs.to_linear_constraint_system().get_string() << "\n";
      std::vector<var_t> args_list = cs.get_args();
      var_t z = args_list[2];
      int p = 'x';
      abs_dom_t boxes = abs_dom_t::top();
      boxes.assign(z, number_t(p));
      crab::outs() << "This is boxes : " << boxes.to_linear_constraint_system().get_string() << "\n";
      m_inv |= boxes;
      crab::outs() << "This is m_inv : " << m_inv.to_linear_constraint_system().get_string() << "\n"; 
    }
    else if(cs.get_intrinsic_name() == "to_one"){
      AbsD pre_inv(m_inv);
      std::vector<var_t> args_list = cs.get_args();
      var_t v = args_list[0];

      //Forget what we know about v
      m_inv -= v;
      lin_cst_t cst(v == number_t(1));
      abs_dom_t boxes = abs_dom_t::top();
      boxes += cst;
      m_inv = m_inv&boxes;
    }
    else if(cs.get_intrinsic_name() == "get_wall_distance"){
      AbsD pre_inv(m_inv);

      std::vector<var_t> args_list = cs.get_args();
      var_t var_distance = args_list[6];
      //Forget what we know about distance
      m_inv -= var_distance;

      //get string name of input variables
      std::string call_st = cs.get_string();
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int index = 0;
      std::vector<std::string> llvm_Vars;
      std::string item;
      while(std::getline(iss1, item, ',')){
        if(index == 0){
          if(item.substr(0, item.find(":")) != "get_wall_distance"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(index >=1 && index <=7){
          llvm_Vars.push_back(item.substr(0, item.find(":")));
        }
        else{
          crab::outs() << "More than expected arguements passed" << "\n";
          std::exit(1);
        }
        index++;
      }

      //Get the Invariants
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      //Iterate over each disjunct in djct_csts
      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string invars = d_ct.get_string(); //get string of preconds invariants
        if(invars.size() < 2){
          crab::outs() << "Malformed lin_cst string in intrinsic (check variable pre_invars)" << "\n";
          std::exit(1);
        }

        invars = invars.substr(1, invars.size()-2); //Stripped braces

        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(invars);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }

        std::vector<std::pair<int, int>> input_bounds(6, std::make_pair(-999,-999)); // -999 is uninitialized
        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting\n";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = -1*std::stoi(it[2]);
                input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].first = -1*std::stoi(it[2])+1;
              }
              else if(it[1] == "<="){
                input_bounds[index].first = -1*std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].second = -1*std::stoi(it[2])-1;
              }
              else if(it[1] == ">="){
               input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
            else{
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = std::stoi(it[2]);
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].second = std::stoi(it[2])-1;
              }
              else if(it[1] == "<="){
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].first = std::stoi(it[2])+1;
              }
              else if(it[1] == ">="){
               input_bounds[index].first = std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
          }
        }

        //Sanitize input_bounds if uninitialized
        for(int i=0;i<4;i++){
          if(i==0){
            if(input_bounds[i].first == -999){
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -999){
              input_bounds[i].second = 34;
            }
          }
          else if(i==1){
            if(input_bounds[i].first == -999){
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -999){
              input_bounds[i].second = 11;
            }
          }
          else if(i==2||i==3){
            if(input_bounds[i].first == -999)
            {
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -999){
              input_bounds[i].second = 5;
            }
          }   
        }

        //Use Input_bounds to compute wall distance
        int distance_lb = 1000, distance_ub = 0, distance;
        for(int px = input_bounds[0].first; px <= input_bounds[0].second; px++){
          for(int py = input_bounds[1].first; py <= input_bounds[1].second; py++){
            for(int vx = input_bounds[2].first; vx <= input_bounds[2].second; vx++){
              for(int vy = input_bounds[3].first; vy <= input_bounds[3].second; vy++){
                for(int dx = input_bounds[4].first; dx <= input_bounds[4].second; dx++){
                  for(int dy = input_bounds[5].first; dy <= input_bounds[5].second; dy++){
                
                if(dx==0 && dy==0){
                  continue;
                }

                distance = get_wall_distance(px, py, dx, dy);

                var_t pos_x = args_list[0];
                var_t pos_y = args_list[1];
                var_t vel_x = args_list[2];
                var_t vel_y = args_list[3];
                var_t dir_x = args_list[4];
                var_t dir_y = args_list[5];

                abs_dom_t boxes = abs_dom_t::bottom();
                abs_dom_t conjunction = abs_dom_t::top();
                lin_cst_t cst1(var_distance == number_t(distance));
                lin_cst_t cst2(pos_x == number_t(px));
                lin_cst_t cst3(pos_y == number_t(py));
                lin_cst_t cst4(vel_x == number_t(vx));
                lin_cst_t cst5(vel_y == number_t(vy));
                lin_cst_t cst6(dir_x == number_t(dx));
                lin_cst_t cst7(dir_y == number_t(dy));
                conjunction += cst1;
                conjunction += cst2;
                conjunction += cst3;
                conjunction += cst4;
                conjunction += cst5;
                conjunction += cst6;
                conjunction += cst7;
                boxes |= conjunction;

                new_m_inv |= m_inv&boxes;
                  }
                }
              }
            }
          }
        }
      }
      
      m_inv = new_m_inv;
      auto tmp1 = m_inv.get_content_domain();
      tmp1.project(cs.get_args());

    }
    else if(cs.get_intrinsic_name() == "get_goal_distance"){
      AbsD pre_inv;

      std::vector<var_t> args_list = cs.get_args();
      var_t distance_x = args_list[4];
      var_t distance_y = args_list[5];
      //Forget anything we know about these
      m_inv -= distance_x;
      m_inv -= distance_y;

      //get string name of input variables
      std::string call_st = cs.get_string();
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int index = 0;
      std::vector<std::string> llvm_Vars;
      std::string item;
      while(std::getline(iss1, item, ',')){
        if(index == 0){
          if(item.substr(0, item.find(":")) != "get_goal_distance"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(index >=1 && index <=6){
          llvm_Vars.push_back(item.substr(0, item.find(":")));
        }
        else{
          crab::outs() << "More than expected arguements passed" << "\n";
          std::exit(1);
        }
        index++;
      }

      //Get the invariants
      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      //Iterate over each disjunct in djct_csts
      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string invars = d_ct.get_string(); //get string of preconds invariants
        if(invars.size() < 2){
          crab::outs() << "Malformed lin_cst string in intrinsic (check variable pre_invars)" << "\n";
          std::exit(1);
        }

        invars = invars.substr(1, invars.size()-2); //Stripped braces

        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(invars);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }

        std::vector<std::pair<int, int>> input_bounds(4, std::make_pair(0,0));
        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting\n";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = -1*std::stoi(it[2]);
                input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].first = -1*std::stoi(it[2])+1;
              }
              else if(it[1] == "<="){
                input_bounds[index].first = -1*std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].second = -1*std::stoi(it[2])-1;
              }
              else if(it[1] == ">="){
               input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
            else{
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = std::stoi(it[2]);
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].second = std::stoi(it[2])-1;
              }
              else if(it[1] == "<="){
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].first = std::stoi(it[2])+1;
              }
              else if(it[1] == ">="){
               input_bounds[index].first = std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
          }
        }

        //Use Input_bounds to compute goal distance
        for(int px = input_bounds[0].first; px <= input_bounds[0].second; px++){
          for(int py = input_bounds[1].first; py <= input_bounds[1].second; py++){
            for(int vx = input_bounds[2].first; vx <= input_bounds[2].second; vx++){
              for(int vy = input_bounds[3].first; vy <= input_bounds[3].second; vy++){
            
            int goal_x, goal_y, g_l1 = 47, g_x = 35, g_y = 12;
            for(int i=0; i<3; i++){
              goal_x = i+32;
              goal_y = 0;
              int x = std::abs(px - goal_x);
              int y = std::abs(py - goal_y);
              int l1 = x+y;
              if(l1 < g_l1){
                g_x = x;
                g_y = y;
                g_l1 = l1;
              }
            }
            //g_x and g_y computed for particular (px, py, vx, vy)
            //Create linear_cst
            var_t pos_x = args_list[0];
            var_t pos_y = args_list[1];
            var_t vel_x = args_list[2];
            var_t vel_y = args_list[3];

            abs_dom_t boxes = abs_dom_t::bottom();
            abs_dom_t conjunction = abs_dom_t::top();
            lin_cst_t cst1(distance_x == number_t(g_x));
            lin_cst_t cst2(distance_y == number_t(g_y));
            lin_cst_t cst3(pos_x == number_t(px));
            lin_cst_t cst4(pos_y == number_t(py));
            lin_cst_t cst5(vel_x == number_t(vx));
            lin_cst_t cst6(vel_y == number_t(vy));
            conjunction += cst1;
            conjunction += cst2;
            conjunction += cst3;
            conjunction += cst4;
            conjunction += cst5;
            conjunction += cst6;
            boxes |= conjunction;

            new_m_inv |= m_inv&boxes;
              }
            }
            
          }
        }
      }

      m_inv = new_m_inv;
      auto tmp1 = m_inv.get_content_domain();
      tmp1.project(cs.get_args());
    }
    else if(cs.get_intrinsic_name() == "is_goal_state"){

      std::vector<var_t> args_list = cs.get_args();
      var_t ret = args_list[4];
      //Forget what we know about ret
      m_inv -= ret;

      //Create goal state constraints
      abs_dom_t goal_check = abs_dom_t::top();
      lin_cst_t cst1(args_list[0] >= number_t(32));
      lin_cst_t cst2(args_list[0] <= number_t(34));
      lin_cst_t cst3(args_list[1] == number_t(0));
      // lin_cst_t cst4(args_list[2] == number_t(0));
      // lin_cst_t cst5(args_list[3] == number_t(0));
      goal_check += cst1;
      goal_check += cst2;
      goal_check += cst3;
      // goal_check += cst4;
      // goal_check += cst5;

      //Create null constraints
      abs_dom_t null_check = abs_dom_t::top();
      lin_cst_t cst6(args_list[0] == number_t(-100));
      lin_cst_t cst8(args_list[1] == number_t(-100));
      lin_cst_t cst9(args_list[2] == number_t(-100));
      lin_cst_t cst10(args_list[3] == number_t(-100));
      null_check += cst6;
      null_check += cst8;
      null_check += cst9;
      null_check += cst10;

      abs_dom_t result = abs_dom_t::top();
      auto check_g = m_inv&goal_check;
      auto check_n = m_inv&null_check;

      if(check_g.is_bottom() && check_n.is_bottom()){
        lin_cst_t r(ret == number_t(2)); //Neither goal nor crash
        result += r;
        m_inv = m_inv&result;
      }
      else if(check_g.is_bottom() && !check_n.is_bottom()){
        lin_cst_t r(ret == number_t(0)); //Crash
        result += r;
        m_inv = m_inv&result;
      }
      else if(!check_g.is_bottom() && check_n.is_bottom()){
        lin_cst_t r(ret == number_t(1)); //Safe
        result += r;
        m_inv = m_inv&result;
      }
      else if(!check_g.is_bottom() && !check_n.is_bottom()){
        lin_cst_t r(ret == number_t(0)); //we treat this as Crash
        result += r;
        m_inv = m_inv&result;
      }
      else{
        crab::outs() << "Law of excluded middle violated\n";
      }

      AbsD pre_inv(m_inv);
      pre_inv.project(cs.get_args());

    }
    else if(cs.get_intrinsic_name() == "check_status"){

      std::vector<var_t> args_list = cs.get_args();
      var_t ret = args_list[4];
      //Forget what we know about ret
      m_inv -= ret;

      //Create goal state constraints
      abs_dom_t goal_check = abs_dom_t::top();
      lin_cst_t cst1(args_list[0] >= number_t(32));
      lin_cst_t cst2(args_list[0] <= number_t(34));
      lin_cst_t cst3(args_list[1] == number_t(0));
      // lin_cst_t cst4(args_list[2] == number_t(0));
      // lin_cst_t cst5(args_list[3] == number_t(0));
      goal_check += cst1;
      goal_check += cst2;
      goal_check += cst3;
      // goal_check += cst4;
      // goal_check += cst5;

      //Create null constraints
      abs_dom_t null_check = abs_dom_t::top();
      lin_cst_t cst6(args_list[0] == number_t(-100));
      lin_cst_t cst8(args_list[1] == number_t(-100));
      lin_cst_t cst9(args_list[2] == number_t(-100));
      lin_cst_t cst10(args_list[3] == number_t(-100));
      null_check += cst6;
      null_check += cst8;
      null_check += cst9;
      null_check += cst10;

      abs_dom_t result = abs_dom_t::top();
      auto check_g = m_inv&goal_check;
      auto check_n = m_inv&null_check;

      if(check_g.is_bottom() && check_n.is_bottom()){
        lin_cst_t r(ret == number_t(2)); //Neither goal nor crash
        result += r;
        m_inv = result;
      }
      else if(check_g.is_bottom() && !check_n.is_bottom()){
        lin_cst_t r(ret == number_t(0)); //Crash
        result += r;
        m_inv = result;
      }
      else if(!check_g.is_bottom() && check_n.is_bottom()){
        lin_cst_t r(ret == number_t(1)); //Safe
        result += r;
        m_inv = result;
      }
      else if(!check_g.is_bottom() && !check_n.is_bottom()){
        lin_cst_t r(ret == number_t(0)); //we treat this as Crash
        result += r;
        m_inv = result;
      } //These 4 are exhaustive
      else{
        auto nc = m_inv&null_check;
        if(nc.is_bottom()){
          lin_cst_t r(ret == number_t(1));
          abs_dom_t res = abs_dom_t::top();
          res += r;
          m_inv = m_inv&res;
        }
        else
        {
          lin_cst_t r(ret == number_t(0));
          result += r;
          m_inv = m_inv&result;
          m_inv = abs_dom_t::bottom();
        }
        
      }

      AbsD pre_inv(m_inv);
      pre_inv.project(cs.get_args());
    }
    else if(cs.get_intrinsic_name() == "is_valid_velocity"){
      AbsD pre_inv(m_inv);

      std::vector<var_t> args_list = cs.get_args();
      var_t var_isvalid = args_list[4];
      //Forget what we know about distance
      m_inv -= var_isvalid;

      //get string name of input variables
      std::string call_st = cs.get_string();
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      int index = 0;
      std::vector<std::string> llvm_Vars;
      std::string item;
      while(std::getline(iss1, item, ',')){
        if(index == 0){
          if(item.substr(0, item.find(":")) != "is_valid_velocity"){
            crab::outs() << "Malformed instrinsic statement call" << "\n";
            std::exit(1);
          }
        }
        else if(index >=1 && index <=5){
          llvm_Vars.push_back(item.substr(0, item.find(":")));
        }
        else{
          crab::outs() << "More than expected arguements passed" << "\n";
          std::exit(1);
        }
        index++;
      }

      auto pre_inv_boxes = m_inv.get_content_domain();
      pre_inv_boxes.project(cs.get_args());
      auto djct_csts = pre_inv_boxes.to_disjunctive_linear_constraint_system();

      //Iterate over each disjunct in djct_csts
      abs_dom_t new_m_inv = abs_dom_t::bottom();
      for(auto &d_ct: djct_csts){
        std::string invars = d_ct.get_string(); //get string of preconds invariants
        if(invars.size() < 2){
          crab::outs() << "Malformed lin_cst string in intrinsic (check variable pre_invars)" << "\n";
          std::exit(1);
        }

        invars = invars.substr(1, invars.size()-2); //Stripped braces

        std::vector<std::string> lin_cst;
        std::vector<std::vector<std::string>> tokens;
        std::istringstream iss2(invars);
        while(std::getline(iss2, item, ';')){
          item = trim(item);
          std::stringstream ss(item);  //String of individual lin_cst
          std::istream_iterator<std::string> begin(ss);
          std::istream_iterator<std::string> end;
          std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
          tokens.push_back(lin_cst);
        }

        std::vector<std::pair<int, int>> input_bounds(4, std::make_pair(-100,-100));
        for(auto it: tokens){
          if((it.size()!=3) && (it[0]!= "true") && (it[0]!= "false")){
            crab::outs() << "Malformed lin_cst token. Exitting" << "\n";
            std::exit(1);
          }
          else if(it.size()==3){
            item = it[0]; //String of llvm variable
            if(item.at(0)=='-'){
              item = item.substr(1, item.size()-1);
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting\n";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = -1*std::stoi(it[2]);
                input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].first = -1*std::stoi(it[2])+1;
              }
              else if(it[1] == "<="){
                input_bounds[index].first = -1*std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].second = -1*std::stoi(it[2])-1;
              }
              else if(it[1] == ">="){
               input_bounds[index].second = -1*std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
            else{
              auto index_it = std::find(llvm_Vars.begin(), llvm_Vars.end(), item);
              if(index_it == llvm_Vars.end()){
                crab::outs() << "Variable not found. Exitting";
                std::exit(1);
              }

              int index = std::distance(llvm_Vars.begin(), index_it);
              if(it[1] == "="){
                input_bounds[index].first = std::stoi(it[2]);
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == "<"){
                input_bounds[index].second = std::stoi(it[2])-1;
              }
              else if(it[1] == "<="){
                input_bounds[index].second = std::stoi(it[2]);
              }
              else if(it[1] == ">"){
                input_bounds[index].first = std::stoi(it[2])+1;
              }
              else if(it[1] == ">="){
               input_bounds[index].first = std::stoi(it[2]);
              }
              else{
                crab::outs() << "LIN CST OPERATOR INVALID. EXITTING" << "\n";
                exit(1);
              }
            }
          }
        }

        //Sanitize input_bounds if uninitialized
        for(int i=0;i<4;i++){
          if(i==0){
            if(input_bounds[i].first == -100){
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -100){
              input_bounds[i].second = 34;
            }
          }
          else if(i==1){
            if(input_bounds[i].first == -100){
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -100){
              input_bounds[i].second = 11;
            }
          }
          else{
            if(input_bounds[i].first == -100){
              input_bounds[i].first = 0;
            }
            if(input_bounds[i].second == -100){
              input_bounds[i].second = 5;
            }
          }   
        }

        std::unordered_set<int> validity; 

        //Use Input_bounds to check if velocity is valid
        for(int px = input_bounds[0].first; px <= input_bounds[0].second; px++){
          for(int py = input_bounds[1].first; py <= input_bounds[1].second; py++){
            for(int vx = input_bounds[2].first; vx <= input_bounds[2].second; vx++){
              for(int vy = input_bounds[3].first; vy <= input_bounds[3].second; vy++){
                int is_valid = is_valid_velocity(px, py, vx, vy);

                validity.insert(is_valid);
              }
            }
          }
        }

        //Create linear_cst
        var_t pos_x = args_list[0];
        var_t pos_y = args_list[1];

        abs_dom_t boxes = abs_dom_t::bottom();
        abs_dom_t conjunction = abs_dom_t::top();
        if(validity.find(0)!=validity.end() && validity.find(1)!=validity.end()){
          lin_cst_t cst1(var_isvalid >= number_t(0));
          lin_cst_t cst2(var_isvalid <= number_t(1));
          conjunction += cst1;
          conjunction += cst2;
        }
        else if(validity.find(0)!=validity.end()){
          lin_cst_t cst1(var_isvalid == number_t(0));
          conjunction += cst1;
        }
        else if(validity.find(1)!=validity.end()){
          lin_cst_t cst2(var_isvalid == number_t(1));
          conjunction += cst2;
        }
        else{
          lin_cst_t cst2(var_isvalid == number_t(1));
          conjunction += cst2;
        }
        lin_cst_t cst3(pos_x >= number_t(input_bounds[0].first));
        lin_cst_t cst4(pos_x <= number_t(input_bounds[0].second));
        lin_cst_t cst5(pos_y >= number_t(input_bounds[1].first));
        lin_cst_t cst6(pos_y <= number_t(input_bounds[1].second));
        conjunction += cst3;
        conjunction += cst4;
        conjunction += cst5; 
        conjunction += cst6;
        boxes |= conjunction;

        new_m_inv |= m_inv&boxes;
      }
      
      m_inv = new_m_inv;
      auto tmp1 = m_inv.get_content_domain();
      tmp1.project(cs.get_args());
    }
    else{
      // This is the default intrinsic behaviour, here we will test the behaviour of an intrinsic

      std::string call_st = cs.get_string();
      crab::outs() << "*****INTRINSIC*******\n";
      crab::outs() << "This is the call statement- " << call_st << "\n";
      call_st = call_st.substr(call_st.find("(") + 1, call_st.find(")") - call_st.find("(") - 1);
      std::istringstream iss1(call_st);
      std::string item;
      int index = 0;
      while(std::getline(iss1, item, ',')){
        crab::outs() << "Entry at index " << index << " : " << item.substr(0, item.find(":")) << "\n";
        index++;
      }

      AbsD pre_invs(m_inv);
      pre_invs.project(cs.get_args()); 
      auto pre_invars = pre_invs.to_linear_constraint_system(); //These are linear constraints projected to llvm variables
      std::string invars = pre_invars.get_string(); //get string from pre_invars

      crab::outs() << "Linear constraints at this point- " << invars << "\n";
      std::vector<std::string> lin_cst;
      std::vector<std::vector<std::string>> tokens;
      invars = invars.substr(1, invars.size()-2); //Stripped the braces
      std::istringstream iss2(invars);
      while(std::getline(iss2, item, ';')){
        item = trim(item);
        std::stringstream ss(item);  //String of individual lin_cst
        std::istream_iterator<std::string> begin(ss);
        std::istream_iterator<std::string> end;
        std::vector<std::string> lin_cst(begin, end); //Convert each linear_cst to its tokens
        tokens.push_back(lin_cst);
      }

      for(auto it: tokens){
        if(it.size()!=3){
          crab::outs() << "Token does not have size 3 : " << it[0] << "\n";
        }
      }

      crab::outs() << "*******************\n";

      m_inv.intrinsic(cs.get_intrinsic_name(), cs.get_args(), cs.get_lhs());
    }
  }
  
  virtual void exec(callsite_t &cs) {
    for (auto vt : cs.get_lhs()) {
      m_inv.operator-=(vt); // havoc
    }
  }
  
  virtual void exec(return_t &ret) {}
  
};

///////////////////////////////////////
/// For inter-procedural analysis
///////////////////////////////////////

template <typename AbsDom> class inter_transformer_helpers {
public:
  typedef typename AbsDom::linear_expression_t linear_expression_t;
  typedef typename AbsDom::variable_t variable_t;
  typedef typename AbsDom::number_t number_t;

  static void unify(AbsDom &inv, variable_t lhs, variable_t rhs) {
    assert(lhs.get_type() == rhs.get_type());
    switch (lhs.get_type()) {
    case BOOL_TYPE:
      inv.assign_bool_var(lhs, rhs, false);
      break;
    case INT_TYPE:
    case REAL_TYPE:
      inv.assign(lhs, rhs);
      break;
    case PTR_TYPE:
      inv.pointer_assign(lhs, rhs, number_t(0));
      break;
    case ARR_BOOL_TYPE:
    case ARR_INT_TYPE:
    case ARR_REAL_TYPE:
    case ARR_PTR_TYPE:
      inv.array_assign(lhs, rhs);
      break;
    default:
      CRAB_ERROR("unsuported type");
    }
  }
};

/////////////////////////////////
/// For backward analysis
/////////////////////////////////

/**
 * Abstract transformer to compute necessary preconditions.
 **/
template <class AbsD, class InvT>
class intra_necessary_preconditions_abs_transformer final
    : public abs_transformer_api<typename AbsD::number_t,
                                 typename AbsD::varname_t> {
public:
  typedef AbsD abs_dom_t;
  typedef typename abs_dom_t::number_t number_t;
  typedef typename abs_dom_t::varname_t varname_t;
  typedef typename abs_dom_t::variable_t variable_t;
  typedef crab::cfg::statement<number_t, varname_t> statement_t;
  typedef abs_transformer_api<number_t, varname_t> abs_transform_api_t;
  using typename abs_transform_api_t::arr_assign_t;
  using typename abs_transform_api_t::arr_init_t;
  using typename abs_transform_api_t::arr_load_t;
  using typename abs_transform_api_t::arr_store_t;
  using typename abs_transform_api_t::assert_t;
  using typename abs_transform_api_t::assign_t;
  using typename abs_transform_api_t::assume_t;
  using typename abs_transform_api_t::bin_op_t;
  using typename abs_transform_api_t::bool_assert_t;
  using typename abs_transform_api_t::bool_assign_cst_t;
  using typename abs_transform_api_t::bool_assign_var_t;
  using typename abs_transform_api_t::bool_assume_t;
  using typename abs_transform_api_t::bool_bin_op_t;
  using typename abs_transform_api_t::bool_select_t;
  using typename abs_transform_api_t::callsite_t;
  using typename abs_transform_api_t::intrinsic_t;  
  using typename abs_transform_api_t::havoc_t;
  using typename abs_transform_api_t::int_cast_t;
  using typename abs_transform_api_t::lin_cst_sys_t;
  using typename abs_transform_api_t::lin_cst_t;
  using typename abs_transform_api_t::lin_exp_t;
  using typename abs_transform_api_t::ptr_assert_t;
  using typename abs_transform_api_t::ptr_assign_t;
  using typename abs_transform_api_t::ptr_assume_t;
  using typename abs_transform_api_t::ptr_function_t;
  using typename abs_transform_api_t::ptr_load_t;
  using typename abs_transform_api_t::ptr_null_t;
  using typename abs_transform_api_t::ptr_object_t;
  using typename abs_transform_api_t::ptr_store_t;
  using typename abs_transform_api_t::return_t;
  using typename abs_transform_api_t::select_t;
  using typename abs_transform_api_t::unreach_t;
  using typename abs_transform_api_t::var_t;

private:
  // used to compute the (necessary) preconditions
  abs_dom_t m_pre;
  // used to refine the preconditions: map from statement_t to abs_dom_t.
  InvT *m_invariants;
  // ignore assertions
  bool m_ignore_assert;
  // if m_ignore_assert is false then enable compute preconditions
  // from good states, otherwise from bad states (by negating the
  // conditions of the assert statements).
  bool m_good_states;

public:
  intra_necessary_preconditions_abs_transformer(abs_dom_t post, InvT *invars,
                                                bool good_states,
                                                bool ignore_assert = false)
      : m_pre(post), m_invariants(invars), m_ignore_assert(ignore_assert),
        m_good_states(good_states) {

    if (!m_invariants) {
      CRAB_ERROR("Invariant table cannot be null");
    }
  }

  ~intra_necessary_preconditions_abs_transformer() = default;

  abs_dom_t preconditions() { return m_pre; }

  void exec(bin_op_t &stmt) {
    auto op = conv_op<ikos::operation_t>(stmt.op());
    if (!op || op >= ikos::OP_UDIV) {
      // ignore UDIV, SREM, UREM
      // CRAB_WARN("backward operation ", stmt.op(), " not implemented");
      m_pre -= stmt.lhs();
      return;
    }

    auto op1 = stmt.left();
    auto op2 = stmt.right();
    abs_dom_t invariant = (*m_invariants)[&stmt];

    CRAB_LOG("backward-tr", crab::outs()
                                << "** " << stmt.lhs() << " := " << op1 << " "
                                << *op << " " << op2 << "\n"
                                << "\tFORWARD INV=" << invariant << "\n"
                                << "\tPOST=" << m_pre << "\n");

    if (op1.get_variable() && op2.get_variable()) {
      m_pre.backward_apply(*op, stmt.lhs(), (*op1.get_variable()),
                           (*op2.get_variable()), std::move(invariant));
    } else {
      assert(op1.get_variable() && op2.is_constant());
      m_pre.backward_apply(*op, stmt.lhs(), (*op1.get_variable()),
                           op2.constant(), std::move(invariant));
    }
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  // select(x := cond ? e1: e2, post) can be reduced to
  //   pre: goto b_then;
  //   pre: goto b_else;
  //   b_then:
  //     assume(cond);
  //     x := e1;
  //     goto post;
  //   b_else:
  //     assume(not(cond));
  //     x := e2;
  //     goto post;
  //   post: ....
  void exec(select_t &stmt) {
    abs_dom_t old_pre = (*m_invariants)[&stmt];

    // -- one of the two branches is false
    abs_dom_t then_inv(old_pre);
    then_inv += stmt.cond();
    if (then_inv.is_bottom()) {
      m_pre.backward_assign(stmt.lhs(), stmt.right(), std::move(old_pre));
      m_pre += stmt.cond().negate();
      return;
    }

    abs_dom_t else_inv(old_pre);
    else_inv += stmt.cond().negate();
    if (else_inv.is_bottom()) {
      m_pre.backward_assign(stmt.lhs(), stmt.left(), std::move(old_pre));
      m_pre += stmt.cond();
      return;
    }

    // -- both branches can be possible so we join them
    abs_dom_t pre_then(m_pre);
    pre_then.backward_assign(stmt.lhs(), stmt.left(), old_pre);
    pre_then += stmt.cond();

    abs_dom_t pre_else(m_pre);
    pre_else.backward_assign(stmt.lhs(), stmt.right(), old_pre);
    pre_else += stmt.cond().negate();

    m_pre = pre_then | pre_else;
  }

  // x := e
  void exec(assign_t &stmt) {
    abs_dom_t invariant = (*m_invariants)[&stmt];

    CRAB_LOG("backward-tr", auto rhs = stmt.rhs();
             crab::outs() << "** " << stmt.lhs() << " := " << rhs << "\n"
                          << "\tFORWARD INV=" << invariant << "\n"
                          << "\tPOST=" << m_pre << "\n");

    m_pre.backward_assign(stmt.lhs(), stmt.rhs(), std::move(invariant));
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  // assume(c)
  // the precondition must contain c so forward and backward are the same.
  void exec(assume_t &stmt) {
    CRAB_LOG("backward-tr", crab::outs() << "** " << stmt << "\n"
                                         << "\tPOST=" << m_pre << "\n");
    m_pre += stmt.constraint();
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  // assert(c)
  void exec(assert_t &stmt) {
    if (!m_ignore_assert) {
      CRAB_LOG("backward-tr", crab::outs() << "** " << stmt << "\n"
                                           << "\tPOST=" << m_pre << "\n");
      if (m_good_states) {
        // similar to assume(c)
        m_pre += stmt.constraint();
      } else {
        // here we are interested in computing preconditions of the
        // error states. Thus, we propagate backwards "not c" which
        // represents the error states.
        abs_dom_t error;
        error += stmt.constraint().negate();
        // we need to join to consider all possible preconditions to
        // error. Otherwise, if we would have two assertions
        // "assert(x >= -2); assert(x <= 2);" we would have
        // incorrectly contradictory constraints.
        m_pre |= error;
      }

      CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
    }
  }

  // similar to assume(false)
  void exec(unreach_t &stmt) { m_pre.set_to_bottom(); }

  // x := *
  // x can be anything before the assignment
  void exec(havoc_t &stmt) { m_pre -= stmt.variable(); }

  void exec(int_cast_t &stmt) {
    abs_dom_t invariant = (*m_invariants)[&stmt];
    CRAB_LOG("backward-tr", crab::outs() << "** " << stmt << "\n"
                                         << "\tPOST=" << m_pre << "\n");
    m_pre.backward_assign(stmt.dst(), stmt.src(), std::move(invariant));
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  void exec(bool_assign_cst_t &stmt) { m_pre -= stmt.lhs(); }
  void exec(bool_assign_var_t &stmt) { m_pre -= stmt.lhs(); }
  void exec(bool_bin_op_t &stmt) { m_pre -= stmt.lhs(); }
  void exec(bool_select_t &stmt) { m_pre -= stmt.lhs(); }

  void exec(bool_assume_t &stmt) {
    // same as forward
    CRAB_LOG("backward-tr", crab::outs() << "** " << stmt << "\n"
                                         << "\tPOST=" << m_pre << "\n");
    m_pre.assume_bool(stmt.cond(), stmt.is_negated());
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  void exec(bool_assert_t &stmt) {
    if (!m_ignore_assert) {
      CRAB_LOG("backward-tr", crab::outs() << "** " << stmt << "\n"
                                           << "\tPOST=" << m_pre << "\n");
      if (m_good_states) {
        // similar to bool_assume(c)
        m_pre.assume_bool(stmt.cond(), false /*non-negated*/);
      } else {
        abs_dom_t error;
        error.assume_bool(stmt.cond(), true /*negated*/);
        m_pre |= error;
      }
      CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
    }
  }

  void exec(arr_init_t &stmt) {
    abs_dom_t invariant = (*m_invariants)[&stmt];

    CRAB_LOG("backward-tr", crab::outs()
                                << "** " << stmt << "\n"
                                << "\tFORWARD INV=" << invariant << "\n"
                                << "\tPOST=" << m_pre << "\n");
    m_pre.backward_array_init(stmt.array(), stmt.elem_size(), stmt.lb_index(),
                              stmt.ub_index(), stmt.val(),
                              std::move(invariant));
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  void exec(arr_load_t &stmt) {
    abs_dom_t invariant = (*m_invariants)[&stmt];

    CRAB_LOG("backward-tr", crab::outs()
                                << "** " << stmt << "\n"
                                << "\tFORWARD INV=" << invariant << "\n"
                                << "\tPOST=" << m_pre << "\n");
    m_pre.backward_array_load(stmt.lhs(), stmt.array(), stmt.elem_size(),
                              stmt.index(), std::move(invariant));
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  void exec(arr_store_t &stmt) {
    abs_dom_t invariant = (*m_invariants)[&stmt];
    CRAB_LOG("backward-tr", crab::outs()
                                << "** " << stmt << "\n"
                                << "\tFORWARD INV=" << invariant << "\n"
                                << "\tPOST=" << m_pre << "\n");

    auto new_arr_v = stmt.new_array();
    if (stmt.lb_index().equal(stmt.ub_index())) {
      if (new_arr_v) {
        m_pre.backward_array_store(
            *new_arr_v, stmt.array(), stmt.elem_size(), stmt.lb_index(),
            stmt.value(), stmt.is_strong_update(), std::move(invariant));
      } else {
        m_pre.backward_array_store(
            stmt.array(), stmt.elem_size(), stmt.lb_index(), stmt.value(),
            stmt.is_strong_update(), std::move(invariant));
      }
    } else {
      if (new_arr_v) {
        m_pre.backward_array_store_range(
            *new_arr_v, stmt.array(), stmt.elem_size(), stmt.lb_index(),
            stmt.ub_index(), stmt.value(), std::move(invariant));
      } else {
        m_pre.backward_array_store_range(stmt.array(), stmt.elem_size(),
                                         stmt.lb_index(), stmt.ub_index(),
                                         stmt.value(), std::move(invariant));
      }
    }
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  void exec(arr_assign_t &stmt) {
    abs_dom_t invariant = (*m_invariants)[&stmt];
    CRAB_LOG("backward-tr", crab::outs()
                                << "** " << stmt << "\n"
                                << "\tFORWARD INV=" << invariant << "\n"
                                << "\tPOST=" << m_pre << "\n");
    m_pre.backward_array_assign(stmt.lhs(), stmt.rhs(), std::move(invariant));
    CRAB_LOG("backward-tr", crab::outs() << "\tPRE=" << m_pre << "\n");
  }

  // NOT IMPLEMENTED
  void exec(ptr_null_t &stmt) {}
  void exec(ptr_object_t &stmt) {}
  void exec(ptr_assign_t &stmt) {}
  void exec(ptr_function_t &stmt) {}
  void exec(ptr_load_t &stmt) {}
  void exec(ptr_store_t &stmt) {}
  void exec(ptr_assume_t &stmt) {}
  void exec(ptr_assert_t &stmt) {}

  /// -- Call and return can be redefined by derived classes

  virtual void exec(callsite_t &cs) {
    for (auto vt : cs.get_lhs()) {
      m_pre -= vt;
    }
  }
  virtual void exec(return_t &stmt) {}

  void exec(intrinsic_t &cs) {
    abs_dom_t invariant = (*m_invariants)[&cs];    
    m_pre.backward_intrinsic(cs.get_intrinsic_name(), cs.get_args(), cs.get_lhs(),
			     std::move(invariant));    
  }
  
};

} // namespace analyzer
} // namespace crab
