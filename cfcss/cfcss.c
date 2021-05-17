#include <stdlib.h>
#include <gcc-plugin.h>
#include <coretypes.h>
#include <gimple.h>
#include <tree.h>
#include <tree-flow.h>
#include <tree-pass.h>
#include <basic-block.h>

int plugin_is_GPL_compatible = 1;
int *signatures, *differences, *adjusting_signatures;
tree cfcss_error_handler = NULL;
bool first_function = true;

static struct plugin_info cfcss_info =
{
    .version = "34",
    .help = "This is my first plugin",
};


static struct plugin_gcc_version cfcss_ver =
{
    .basever = "4.6",
};

static bool cfcss_gate(void)
{
    return true;
}

static unsigned dumpcfg(void)
{
    brief_dump_cfg(stderr);
    return 0;
}

void ALL_BB_print(void)
{
    basic_block bb;
    gimple_stmt_iterator gsi;
    FOR_ALL_BB(bb)
    {
      gimple_dump_bb (bb, stderr, 0, TDF_SLIM);
    }
}

int index_bb(basic_block bb) {
  return bb->index - 2;
}

bool is_branch_fan_in_node(basic_block bb) {
  return !single_pred_p(bb);
}

basic_block get_bb_by_index(int index) {
  basic_block bb;
  basic_block bb_by_index;

  FOR_ALL_BB(bb) {
    if (bb->index == index) {
      bb_by_index = bb;
    }
  }
  return bb_by_index;
}

basic_block get_last_bb() {
  return get_bb_by_index(last_basic_block - 1);
}

basic_block get_first_bb() {
  return get_bb_by_index(2);
}

void assign_signatures() {
  basic_block bb;
  signatures = (int *) xmalloc(n_basic_blocks * sizeof(int));

  FOR_EACH_BB(bb) {
    int bb_index = index_bb(bb);
    signatures[bb_index] = bb_index;
  }
}

void compute_difference_single(basic_block bb) {
  basic_block pred_bb = single_pred(bb);
  differences[index_bb(bb)] = signatures[index_bb(bb)] ^ signatures[index_bb(pred_bb)];
}

void compute_difference_multi(basic_block bb) {
  int pred_count = EDGE_COUNT (bb->preds);
  int pred_indexes[pred_count];
  int i = 0, j;
  edge e;
	edge_iterator ei;
	FOR_EACH_EDGE(e, ei, bb->preds){
		pred_indexes[i] = index_bb(e->src);
    i++;
	}

  differences[index_bb(bb)] = signatures[index_bb(bb)] ^ signatures[pred_indexes[0]];
  for (j = 0; j < pred_count; j++) {
    adjusting_signatures[pred_indexes[j]] = signatures[pred_indexes[j]] ^ signatures[pred_indexes[0]];
  }
}

void compute_differences() {
  basic_block bb;
  int i;
  differences = (int *) xmalloc(n_basic_blocks * sizeof(int));
  adjusting_signatures = (int *) xmalloc(n_basic_blocks * sizeof(int));
  for (i = 0; i < n_basic_blocks; i++) {
    adjusting_signatures[i] = -1;
  }

  FOR_EACH_BB(bb) {
    if (index_bb(bb) == 0) {
      differences[0] = signatures[0];
    }
    else if (!is_branch_fan_in_node(bb)) {
      compute_difference_single(bb);
    }
    else {
      compute_difference_multi(bb);
    }
  }
}

void handle_aliasing()
{
  // TODO
}

void insert_instructions()
{
  basic_block bb, bb_first, bb_correct, bb_correct_first, bb_err;
  tree G, D, dj, sj, zero, adj_sig, lbl_correct, lbl_err;
  gimple stmt_assign, stmt_nop, stmt_cond, stmt_call;
  gimple_stmt_iterator gsi, gsi_err;
  edge e, e_end, e_correct, e_err;
  int bb_index, i;
  int n_original_bbs;

  n_original_bbs = n_basic_blocks;

  // create runtime variables for inserting
  G = create_tmp_var (integer_type_node, "G");
  D = create_tmp_var (integer_type_node, "D");
  mark_sym_for_renaming(G);
  mark_sym_for_renaming(D);

  // create constants for inserting
  dj = build_int_cst (integer_type_node, differences[0]);
  sj = build_int_cst (integer_type_node, signatures[0]);
  zero = build_int_cst (integer_type_node, 0);

  // create error handling block and handle first block
  bb_first = get_bb_by_index(2);
  gsi = gsi_start_bb(bb_first);

  stmt_assign = gimple_build_assign_with_ops(BIT_XOR_EXPR, G, zero, zero);
  update_stmt(stmt_assign);
  gsi_insert_before (&gsi, stmt_assign, GSI_NEW_STMT);

  stmt_assign = gimple_build_assign_with_ops(BIT_XOR_EXPR, D, zero, zero);
  update_stmt(stmt_assign);
  gsi_insert_after (&gsi, stmt_assign, GSI_NEW_STMT);

  stmt_assign = gimple_build_assign_with_ops(BIT_XOR_EXPR, G, G, dj);
  update_stmt(stmt_assign);
  gsi_insert_after (&gsi, stmt_assign, GSI_NEW_STMT);

  if (is_branch_fan_in_node(bb_first)) {
    stmt_assign = gimple_build_assign_with_ops(BIT_XOR_EXPR, G, G, D);
    update_stmt(stmt_assign);
    gsi_insert_after (&gsi, stmt_assign, GSI_NEW_STMT);
  }

  if (adjusting_signatures[index_bb(bb_first)] != -1) {
    adj_sig = build_int_cst (integer_type_node, adjusting_signatures[index_bb(bb_first)]);
    stmt_assign = gimple_build_assign(D, adj_sig);
    update_stmt(stmt_assign);
    gsi_insert_after (&gsi, stmt_assign, GSI_NEW_STMT);
  }

  e_correct = split_block (bb_first, gsi_stmt (gsi));
  execute_on_growing_pred (e_correct);
  bb_correct_first = e_correct->dest;

  e_err = split_block (bb_first, gsi_stmt (gsi));
  execute_on_growing_pred (e_err);
  bb_err = e_err->dest;

  gsi_err = gsi_start_bb(bb_err);
  stmt_call = gimple_build_call (cfcss_error_handler, 0);
  gsi_insert_after (&gsi_err, stmt_call, GSI_NEW_STMT);

  execute_on_shrinking_pred (e_correct);
  remove_edge(e_correct);

  e_correct = make_edge (bb_first, bb_correct_first, EDGE_TRUE_VALUE);

  if (e_err->flags & EDGE_FALLTHRU)
  {
    e_err->flags = e_err->flags - EDGE_FALLTHRU;
  }
  if (!(e_err->flags & EDGE_FALSE_VALUE))
  {
    e_err->flags = e_err->flags + EDGE_FALSE_VALUE;
  }

  lbl_correct = gimple_block_label(bb_correct_first);
  lbl_err = gimple_block_label(bb_err);
  stmt_cond = gimple_build_cond (EQ_EXPR, G, sj, lbl_correct, lbl_err);
  gsi_insert_after (&gsi, stmt_cond, GSI_NEW_STMT);


  if (dom_info_available_p (CDI_DOMINATORS))
  {
    set_immediate_dominator (CDI_DOMINATORS, bb_first, recompute_dominator (CDI_DOMINATORS, bb_first));
    set_immediate_dominator (CDI_DOMINATORS, bb_correct_first, recompute_dominator (CDI_DOMINATORS, bb_correct_first));
    set_immediate_dominator (CDI_DOMINATORS, bb_err, recompute_dominator (CDI_DOMINATORS, bb_err));
  }
  if (dom_info_available_p (CDI_POST_DOMINATORS))
  {
	  set_immediate_dominator (CDI_POST_DOMINATORS, bb_first, recompute_dominator (CDI_POST_DOMINATORS, bb_first));
    set_immediate_dominator (CDI_POST_DOMINATORS, bb_correct_first, recompute_dominator (CDI_POST_DOMINATORS, bb_correct_first));
    set_immediate_dominator (CDI_POST_DOMINATORS, bb_err, recompute_dominator (CDI_POST_DOMINATORS, bb_err));
  }

  for (i = 3; i < n_original_bbs; i++)
  {
    bb = get_bb_by_index(i);
    if (bb->index == bb_first->index || bb->index == bb_correct_first->index || bb->index == bb_err->index) {
      continue;
    }

    bb_index = index_bb(bb);
    dj = build_int_cst (integer_type_node, differences[bb_index]);
    sj = build_int_cst (integer_type_node, signatures[bb_index]);

    gsi = gsi_start_bb(bb);

    stmt_assign = gimple_build_assign_with_ops(BIT_XOR_EXPR, G, G, dj);
    update_stmt(stmt_assign);
    gsi_insert_before (&gsi, stmt_assign, GSI_NEW_STMT);

    if (is_branch_fan_in_node(bb)) {
      stmt_assign = gimple_build_assign_with_ops(BIT_XOR_EXPR, G, G, D);
      update_stmt(stmt_assign);
      gsi_insert_after (&gsi, stmt_assign, GSI_NEW_STMT);
    }

    if (adjusting_signatures[bb_index] != -1) {
      adj_sig = build_int_cst (integer_type_node, adjusting_signatures[bb_index]);
      stmt_assign = gimple_build_assign(D, adj_sig);
      update_stmt(stmt_assign);
      gsi_insert_after (&gsi, stmt_assign, GSI_NEW_STMT);
    }

    e_correct = split_block (bb, gsi_stmt (gsi));
    execute_on_growing_pred (e_correct);
    bb_correct = e_correct->dest;

    e_err = make_edge (bb, bb_err, EDGE_FALSE_VALUE);

    if (e_correct->flags & EDGE_FALLTHRU)
    {
      e_correct->flags = e_correct->flags - EDGE_FALLTHRU;
    }
    if (!(e_correct->flags & EDGE_FALSE_VALUE))
    {
      e_correct->flags = e_correct->flags + EDGE_TRUE_VALUE;
    }

    lbl_correct = gimple_block_label(bb_correct);
    stmt_cond = gimple_build_cond (EQ_EXPR, G, sj, lbl_correct, lbl_err);
    gsi_insert_after (&gsi, stmt_cond, GSI_NEW_STMT);


    if (dom_info_available_p (CDI_DOMINATORS))
    {
      set_immediate_dominator (CDI_DOMINATORS, bb, recompute_dominator (CDI_DOMINATORS, bb));
      set_immediate_dominator (CDI_DOMINATORS, bb_correct, recompute_dominator (CDI_DOMINATORS, bb_correct));
      set_immediate_dominator (CDI_DOMINATORS, bb_err, recompute_dominator (CDI_DOMINATORS, bb_err));
    }
    if (dom_info_available_p (CDI_POST_DOMINATORS))
    {
	    set_immediate_dominator (CDI_POST_DOMINATORS, bb, recompute_dominator (CDI_POST_DOMINATORS, bb));
      set_immediate_dominator (CDI_POST_DOMINATORS, bb_correct, recompute_dominator (CDI_POST_DOMINATORS, bb_correct));
      set_immediate_dominator (CDI_POST_DOMINATORS, bb_err, recompute_dominator (CDI_POST_DOMINATORS, bb_err));
    }
  }
}

static unsigned cfcss_exec(void)
{
    unsigned i;
    const_tree str, op;
    basic_block bb, bb_after, bbs, bbend;
    gimple stmt;
    gimple_stmt_iterator gsi;
    edge e_after, eend;
    int str_count = 0;

    // first compiled function should be cfcss_error_handler
    if (first_function) {
      if (get_identifier(current_function_name()) == get_identifier("cfcss_error_handler"))
      {
        if (cfcss_error_handler == NULL)
        {
          // setting cfcss_error_handler function from compiled code to tree variable
          cfcss_error_handler = current_function_decl;
        }
        first_function = false;
        return 0;
      }
      // cfcss_error_handler is not first function in compiled code
      else {
        printf("Error: cfcss_error_handler not declared as first function in compiled source code, exiting compilation...\n");
        exit(1);
      }
    }

    handle_aliasing();
    
    assign_signatures();
    compute_differences();

    insert_instructions();

    free(signatures);
    free(differences);
    free(adjusting_signatures);
    return 0;
}

static struct gimple_opt_pass cfcss_pass = 
{
    .pass.type = GIMPLE_PASS,
    .pass.name = "cfcss",
    .pass.gate = cfcss_gate,
    .pass.execute = cfcss_exec,
};

int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *ver)
{
    struct register_pass_info pass;

    if (strncmp(ver->basever, cfcss_ver.basever, strlen("4.6")))
      return -1;

    pass.pass = &cfcss_pass.pass;
    pass.reference_pass_name = "ssa";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    register_callback("cfcss", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("cfcss", PLUGIN_INFO, NULL, &cfcss_info);

    return 0;
}
