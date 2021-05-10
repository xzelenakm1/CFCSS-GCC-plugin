#include <stdlib.h>
#include <gcc-plugin.h>
#include <coretypes.h>
// #include <diagnostic.h>
#include <gimple.h>
#include <tree.h>
#include <tree-flow.h>
#include <tree-pass.h>
#include <basic-block.h>

int plugin_is_GPL_compatible = 1;
int *signatures, *differences, *adjusting_signatures;
tree cfcss_error_handler = NULL;

static struct plugin_info plugin_info =
{
    .version = "34",
    .help = "This is my first plugin",
};


static struct plugin_gcc_version plugin_ver =
{
    .basever = "4.6",
};

static bool plugin_gate(void)
{
    return true;
}

static unsigned dumpcfg(void)
{
    brief_dump_cfg(stderr); //**** NEPOZNA VER 9
    return 0;
}

void ALL_BB_print(void)
{
    basic_block bb;
    gimple_stmt_iterator gsi;
    //gimple_seq seq;
    //FOR_EACH_BB(bb)
    FOR_ALL_BB(bb)
    {
      gimple_dump_bb (bb, stderr, 0, TDF_SLIM); // **** NEFUNGUJE VO VER 9
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

gimple get_last_statement_in_bb (basic_block bb) {
  // gimple_stmt_iterator gsi;
  // gimple stmt;
  // int i;

  // for (si = gsi_start_bb (bb); !gsi_end_p (si); gsi_next (&si)) {

  // }

  // stmt = gsi_stmt (gsi);
  // print_gimple_stmt (stderr, gsi_stmt(gsi), 0, TDF_SLIM);
  printf("---------------- before split --------- \n");
  // e = split_block (bbs, stmts); 
  // e = split_block_after_labels (e->dest);

  printf("---------------- after split --------- \n");

  printf("%d\n", bb->index);

  gimple_stmt_iterator gsi = gsi_last_bb(bb);
  gsi_prev(&gsi);
  gimple stmt = gsi_stmt(gsi);
  return stmt;
}

// void split_block_err(basic_block bb, gimple stmt, basic_block err_bb) {

//   e_after = split_block (bb, gsi_stmt (gsi));
//   execute_on_growing_pred (e_after);
//   bb_after = e_after->dest;

//   if (eafter->flags & EDGE_FALLTHRU)
//   {
//     eafter->flags = eafter->flags - EDGE_FALLTHRU;
//   }
//   if (!(eafter->flags & EDGE_TRUE_VALUE))
//   {
//     eafter->flags = eafter->flags + EDGE_TRUE_VALUE;
//   }

//   enew = make_edge (bb, bbend, EDGE_FALSE_VALUE);

//   if (dom_info_available_p (CDI_DOMINATORS))
//   {
//     set_immediate_dominator (CDI_DOMINATORS, bbend, recompute_dominator (CDI_DOMINATORS, bbend));
// 	  set_immediate_dominator (CDI_DOMINATORS, bb, recompute_dominator (CDI_DOMINATORS, bb));
//   }
//   if (dom_info_available_p (CDI_POST_DOMINATORS))
//   {
// 	  set_immediate_dominator (CDI_POST_DOMINATORS, bbend, recompute_dominator (CDI_POST_DOMINATORS, bbend));
// 	  set_immediate_dominator (CDI_POST_DOMINATORS, bb, recompute_dominator (CDI_POST_DOMINATORS, bb));
//   }
// }

void assign_signatures() {
  basic_block bb;
  printf("Number of basic blocks: %d\n", n_basic_blocks - 2);
  signatures = (int *) xmalloc(n_basic_blocks * sizeof(int));

  FOR_EACH_BB(bb) {
    int bb_index = index_bb(bb);
    signatures[bb_index] = bb_index;
    printf("bb index: %d, predecessor count: %d\n", signatures[bb_index], EDGE_COUNT (bb->preds));
  }
}

// for vj whose pred(vj) = vi
// difference dj = si ⊕ sj
// insert G = G ⊕ dj
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
  // TODO - handle circular loopback to block
  printf("For BB index %d, pred_ids are: ", index_bb(bb));
  for (j = 0; j < pred_count; j++) {
    printf("%d ", pred_indexes[j]);
  }
  printf("\n");

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

    // int bb_index = index_bb(bb);
    // signatures[bb_index] = bb_index;
    // printf("bb index: %d, predecessor count: %d\n", signatures[bb_index], EDGE_COUNT (bb->preds));
  }
}

void handle_aliasing()
{
  // TODO
}

void insert_instructions()
{
  basic_block bb, bb_first, bb_correct, bb_correct_first, bb_err;
  tree G, D, dj, sj, adj_sig, lbl_correct, lbl_err;
  gimple stmt_assign, stmt_nop, stmt_cond, stmt_call;
  gimple_stmt_iterator gsi, gsi_err;
  edge e, e_end, e_correct, e_err;
  int bb_index, i;
  int n_original_bbs;

  n_original_bbs = n_basic_blocks;

  // create runtime variables for inserting
  G = create_tmp_var (integer_type_node, "G");
  D = create_tmp_var (integer_type_node, "D");
  DECL_INITIAL (G) = build_int_cst (integer_type_node, signatures[0]);
  DECL_INITIAL (D) = build_int_cst (integer_type_node, signatures[0]);
  mark_sym_for_renaming(G);
  mark_sym_for_renaming(D);

  // create constants for inserting
  dj = build_int_cst (integer_type_node, differences[0]);
  sj = build_int_cst (integer_type_node, signatures[0]);

  // create error handling block and handle first block
  bb_first = get_bb_by_index(2);
  gsi = gsi_start_bb(bb_first);

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
    printf("%d\n", bb->index);

    bb_index = index_bb(bb);
    dj = build_int_cst (integer_type_node, differences[bb_index]);
    sj = build_int_cst (integer_type_node, signatures[bb_index]);

    gsi = gsi_start_bb(bb);

    stmt_assign = gimple_build_assign_with_ops(BIT_XOR_EXPR, G, G, dj);
    update_stmt(stmt_assign);
    gsi_insert_after (&gsi, stmt_assign, GSI_NEW_STMT);

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

static unsigned plugin_exec(void)
{
    unsigned i;
    const_tree str, op;
    basic_block bb, bb_after, bbs, bbend;
    gimple stmt;
    gimple_stmt_iterator gsi;
    edge e_after, eend;
    int str_count = 0;

    ALL_BB_print();
    // dumpcfg();

    // if (n_basic_blocks <= 1) {
    //   printf("Function with only one BB, therefore not implementing CFCSS method\n");
    //   return 0;
    // }

    if (get_identifier(current_function_name()) == get_identifier("cfcss_error_handler"))
    {
      if (cfcss_error_handler == NULL)
      {
        cfcss_error_handler = current_function_decl;
      }
      return 0;
    }

    printf("****************** START CFCSS ********************\n");

    handle_aliasing();
    
    assign_signatures();
    compute_differences();

    insert_instructions();

    printf("****************** END CFCSS ********************\n");

    ALL_BB_print();
    


    // FOR_EACH_BB(bb)
    // {
    //   bbs = bb;
    //   gsi=gsi_last_bb(bb);
    // }
    // gsi_prev(&gsi);
    // gsi_prev(&gsi);

    // create error handling block
    // basic_block first_bb = get_bb_by_index(2);
    // gimple last_statement = get_last_statement_in_bb(first_bb);
    // gimple nop = gimple_build_nop();
    // gsi = gsi_last_bb(first_bb);
    // gsi_insert_after (&gsi, nop, GSI_SAME_STMT);
    // update_stmt(nop);
    // // last_statement = gsi_stmt(gsi);
    // eend = split_block (first_bb, gsi_stmt(gsi));
    // printf("************************ VERIFY FLOW ZA EEND **********************\n");
    // verify_flow_info ();

    // // execute_on_growing_pred (eend);
    // // bbend = eend->dest;
    // printf("************************ VERIFY FLOW ZA EXECUTE ON GROWING PRED **********************\n");
    // verify_flow_info ();
    // split_block_err(get_last_bb(), get_last_statement_in_bb(get_last_bb()));
    // e = split_block (bbs, stmts); 
    // e = split_block_after_labels (e->dest);



    // ALL_BB_print();
    // print_gimple_stmt (stderr, get_last_statement_in_bb(get_last_bb()), 0, TDF_SLIM);
    // dumpcfg();

    // printf ("%d %d\n", last_basic_block, );
    // gimple_dump_bb (get_last_bb(), stderr, 0, TDF_SLIM);

    free(signatures);
    free(differences);
    free(adjusting_signatures);
    return 0;
}

static struct gimple_opt_pass plugin_pass = 
{
    .pass.type = GIMPLE_PASS,
    .pass.name = "cfcss",
    .pass.gate = plugin_gate,
    .pass.execute = plugin_exec,
};

int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *ver)
{
    struct register_pass_info pass;
    printf("initializing plugin\n");

    if (strncmp(ver->basever, plugin_ver.basever, strlen("4.6")))
      return -1;

    pass.pass = &plugin_pass.pass;
    pass.reference_pass_name = "ssa";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    printf("end of plugin init\n");

    register_callback("cfcss", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("cfcss", PLUGIN_INFO, NULL, &plugin_info);

    return 0;
}
