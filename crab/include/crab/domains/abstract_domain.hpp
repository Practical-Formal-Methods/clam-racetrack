#pragma once

#include <crab/domains/abstract_domain_operators.hpp>
#include <crab/iterators/thresholds.hpp>
#include <vector>

namespace crab {

namespace domains {

template <class Dom> struct abstract_domain_traits;

/**
 * All abstract domains must derive from the abstract_domain class
 * and expose publicly all its public typedef's.
 *
 * This is a sample of how to implement a new abstract domain:
 *
 * template<typename Number, typename VariableName>
 * class my_new_domain final: public
 *abstract_domain<my_new_domain<Number,VariableName>> {
 *   ...
 *   bool is_bottom() {...}
 *   bool is_top() {...}
 *   ...
 * };
 *
 *
 * template<typename Number, typename VariableName>
 * struct abstract_domain_traits<my_new_domain<Number,VariableName>> {
 *   typedef Number number_t;
 *   typedef VariableName varname_t;
 * };
 **/
template <class Dom> class abstract_domain : public ikos::writeable {
public:
  typedef typename abstract_domain_traits<Dom>::number_t number_t;
  typedef typename abstract_domain_traits<Dom>::varname_t varname_t;

  typedef ikos::linear_expression<number_t, varname_t> linear_expression_t;
  typedef ikos::linear_constraint<number_t, varname_t> linear_constraint_t;
  typedef ikos::linear_constraint_system<number_t, varname_t>
      linear_constraint_system_t;
  typedef ikos::disjunctive_linear_constraint_system<number_t, varname_t>
      disjunctive_linear_constraint_system_t;
  typedef ikos::variable<number_t, varname_t> variable_t;
  typedef std::vector<variable_t> variable_vector_t;
  typedef pointer_constraint<variable_t> pointer_constraint_t;

  abstract_domain() : ikos::writeable() {}

  virtual ~abstract_domain(){};

  static Dom top() {
    Dom abs;
    abs.set_to_top();
    return abs;
  }

  static Dom bottom() {
    Dom abs;
    abs.set_to_bottom();
    return abs;
  }

  /**************************** Lattice operations ****************************/

  // set *this to top
  virtual void set_to_top() = 0;
  // set *this to bottom
  virtual void set_to_bottom() = 0;
  // return true if the abstract state is bottom
  virtual bool is_bottom() = 0;
  // return true if the abstract state is top
  virtual bool is_top() = 0;

  // Inclusion operator: return true if *this is equal or more precise than abs
  virtual bool operator<=(Dom abs) = 0;
  // Join operator: join(*this, abs)
  virtual Dom operator|(Dom abs) = 0;
  // *this = join(*this, abs)
  virtual void operator|=(Dom abs) = 0;
  // Meet operator: meet(*this, abs)
  virtual Dom operator&(Dom abs) = 0;
  // Widening operator: widening(*this, abs)
  virtual Dom operator||(Dom abs) = 0;
  // Narrowing operator: narrowing(*this, abs)
  virtual Dom operator&&(Dom abs) = 0;
  // Widening with thresholds: widening_ts(*this, abs)
  virtual Dom
  widening_thresholds(Dom abs,
                      const crab::iterators::thresholds<number_t> &ts) = 0;

  /**************************** Arithmetic operations *************************/
  // x := y op z
  virtual void apply(operation_t op, variable_t x, variable_t y,
                     variable_t z) = 0;
  // x := y op k
  virtual void apply(operation_t op, variable_t x, variable_t y,
                     number_t k) = 0;
  // x := e
  virtual void assign(variable_t x, linear_expression_t e) = 0;
  // forall cst in csts: assume(cst)
  virtual void operator+=(linear_constraint_system_t csts) = 0;
  // x := y op z
  virtual void apply(bitwise_operation_t op, variable_t x, variable_t y,
                     variable_t z) = 0;
  // x := y op k
  virtual void apply(bitwise_operation_t op, variable_t x, variable_t y,
                     number_t k) = 0;
  // dst := src
  virtual void apply(int_conv_operation_t op, variable_t dst,
                     variable_t src) = 0;

  /**************************** Boolean operations ****************************/
  // lhs := rhs
  virtual void assign_bool_cst(variable_t lhs, linear_constraint_t rhs) = 0;
  // lhs := not(rhs) if is_not_rhs
  // lhs := rhs      otherwise
  virtual void assign_bool_var(variable_t lhs, variable_t rhs,
                               bool is_not_rhs) = 0;
  // x := y op z
  virtual void apply_binary_bool(bool_operation_t op, variable_t x,
                                 variable_t y, variable_t z) = 0;
  // assume(not(v)) if is_negated
  // assume(v)      otherwise
  virtual void assume_bool(variable_t v, bool is_negated) = 0;

  /**************************** Array operations *****************************/
  // make a fresh array with contents a[j] initialized to val such that
  // j \in [lb_idx,ub_idx] and j % elem_size == 0.
  // elem_size is in bytes.
  virtual void array_init(variable_t a, linear_expression_t elem_size,
                          linear_expression_t lb_idx,
                          linear_expression_t ub_idx,
                          linear_expression_t val) = 0;
  // lhs := a[i] where elem_size is in bytes
  virtual void array_load(variable_t lhs, variable_t a,
                          linear_expression_t elem_size,
                          linear_expression_t i) = 0;
  // a[i] := v where elem_size is in bytes
  virtual void array_store(variable_t a, linear_expression_t elem_size,
                           linear_expression_t i, linear_expression_t v,
                           bool is_strong_update) = 0;
  // a_new = a_old[i <- v] where elem_size is in bytes
  virtual void array_store(variable_t a_new, variable_t a_old,
                           linear_expression_t elem_size, linear_expression_t i,
                           linear_expression_t v, bool is_strong_update) = 0;
  // forall i<=k<j and k % elem_size == 0 :: a[k] := v.
  // elem_size is in bytes
  virtual void array_store_range(variable_t a, linear_expression_t elem_size,
                                 linear_expression_t i, linear_expression_t j,
                                 linear_expression_t v) = 0;
  // forall i<=k<j and k % elem_size == 0 :: a_new = a_old[k <- v].
  // elem_size is in bytes
  virtual void array_store_range(variable_t a_new, variable_t a_old,
                                 linear_expression_t elem_size,
                                 linear_expression_t i, linear_expression_t j,
                                 linear_expression_t v) = 0;
  // forall i :: a[i] := b[i]
  virtual void array_assign(variable_t a, variable_t b) = 0;

  /**************************** Pointer operations ************************/
  // p := *q
  // elem_size is the number of bytes read from memory
  virtual void pointer_load(variable_t p, variable_t q, linear_expression_t elem_size) = 0;
  // *p := q
  // elem_size is the number of bytes written into memory
  virtual void pointer_store(variable_t p, variable_t q, linear_expression_t elem_size) = 0;
  // p := q + offset
  virtual void pointer_assign(variable_t p, variable_t q, linear_expression_t offset) = 0;
  // p := &a;
  virtual void pointer_mk_obj(variable_t p, ikos::index_t address) = 0;
  // p := &func
  virtual void pointer_function(variable_t p, varname_t func) = 0;
  // p := null
  virtual void pointer_mk_null(variable_t p) = 0;
  // assume(cst);
  virtual void pointer_assume(pointer_constraint_t cst) = 0;
  // assert(cst);
  virtual void pointer_assert(pointer_constraint_t cst) = 0;

  /**************************** Backward arithmetic operations ***************/
  // x = y op z
  // Substitute x with y op z in the abstract value
  // The result is meet with invariant.
  virtual void backward_apply(operation_t op, variable_t x, variable_t y,
                              variable_t z, Dom invariant) = 0;
  // x = y op k
  // Substitute x with y op k in the abstract value
  // The result is meet with invariant.
  virtual void backward_apply(operation_t op, variable_t x, variable_t y,
                              number_t k, Dom invariant) = 0;
  // x = e
  // Substitute x with e in the abstract value
  // The result is meet with invariant.
  virtual void backward_assign(variable_t x, linear_expression_t e,
                               Dom invariant) = 0;

  /**************************** Backward boolean operations ******************/
  virtual void backward_assign_bool_cst(variable_t lhs, linear_constraint_t rhs,
                                        Dom invariant) = 0;
  virtual void backward_assign_bool_var(variable_t lhs, variable_t rhs,
                                        bool is_not_rhs, Dom invariant) = 0;
  virtual void backward_apply_binary_bool(bool_operation_t op, variable_t x,
                                          variable_t y, variable_t z,
                                          Dom invariant) = 0;

  /**************************** Backward array operations ******************/
  virtual void backward_array_init(variable_t a, linear_expression_t elem_size,
                                   linear_expression_t lb_idx,
                                   linear_expression_t ub_idx,
                                   linear_expression_t val, Dom invariant) = 0;
  virtual void backward_array_load(variable_t lhs, variable_t a,
                                   linear_expression_t elem_size,
                                   linear_expression_t i, Dom invariant) = 0;
  virtual void backward_array_store(variable_t a, linear_expression_t elem_size,
                                    linear_expression_t i,
                                    linear_expression_t v,
                                    bool is_strong_update, Dom invariant) = 0;
  virtual void backward_array_store(variable_t a_new, variable_t a_old,
                                    linear_expression_t elem_size,
                                    linear_expression_t i,
                                    linear_expression_t v,
                                    bool is_strong_update, Dom invariant) = 0;
  virtual void
  backward_array_store_range(variable_t a, linear_expression_t elem_size,
                             linear_expression_t i, linear_expression_t j,
                             linear_expression_t v, Dom invariant) = 0;
  virtual void backward_array_store_range(variable_t a_new, variable_t a_old,
                                          linear_expression_t elem_size,
                                          linear_expression_t i,
                                          linear_expression_t j,
                                          linear_expression_t v,
                                          Dom invariant) = 0;
  virtual void backward_array_assign(variable_t a, variable_t b,
                                     Dom invariant) = 0;

  /**************************** Miscellaneous operations ****************/
  // forget v
  virtual void operator-=(variable_t v) = 0;

  // Convert the abstract state into a conjunction of linear constraints
  virtual linear_constraint_system_t to_linear_constraint_system() = 0;

  // Convert the abstract state into a disjunction of conjunction
  // of linear constraints.
  virtual disjunctive_linear_constraint_system_t
  to_disjunctive_linear_constraint_system() = 0;

  // Rename in the abstract state the variables "from" with those from to.
  // 
  // If any variable from "to" exists already in the abstract state
  // then an error will be raised. This might be a bit restrictive and
  // it can be relaxed if needed in the future. 
  virtual void rename(const variable_vector_t &from,
                      const variable_vector_t &to) = 0;

  // Normalize the abstract domain if such notion exists.
  virtual void normalize() = 0;

  // Reduce the size of the abstract domain representation.
  virtual void minimize() = 0;

  // Forget variables form the abstract domain
  virtual void forget(const variable_vector_t &variables) = 0;

  // Project the abstract domain onto variables (dual to forget)
  virtual void project(const variable_vector_t &variables) = 0;

  // Make a new copy of var without relating var with new_var
  virtual void expand(variable_t var, variable_t new_var) = 0;

  // Function whose semantics is defined by the particular abstract
  // domain
  virtual void intrinsic(std::string name,
			 const variable_vector_t &inputs,
			 const variable_vector_t &outputs) = 0;

  virtual void backward_intrinsic(std::string name,
				  const variable_vector_t &inputs,
				  const variable_vector_t &outputs,
				  Dom invariant) = 0;
  
};

} // end namespace domains
} // end namespace crab
