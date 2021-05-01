#include <stdlib.h>
#include <gcc-plugin.h>
// #include <coretypes.h>
// #include <diagnostic.h>
#include <gimple.h>
// #include <tree.h>
// #include <tree-flow.h>
#include <tree-pass.h>
#include <basic-block.h>

int plugin_is_GPL_compatible = 1;
int *signatures, *differences, *adjusting_signatures;

static struct plugin_info plugin_info =
{
    .version = "34",
    .help = "This is my first plugin",
};


static struct plugin_gcc_version plugin_ver =
{
    .basever = "9.3",
};

static bool plugin_gate(void)
{
    return true;
}

int index_bb(basic_block bb) {
  return bb->index - 2;
}

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
      else if (single_pred_p(bb)) {
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

}

static unsigned plugin_exec(void)
{
    unsigned i;
    const_tree str, op;
    basic_block bb;
    gimple stmt;
    gimple_stmt_iterator gsi;
    int str_count = 0;

    handle_aliasing();
    
    assign_signatures();
    compute_differences();

    FOR_EACH_BB(bb)
    {
      

      // printf("%d\n", bb->index - 2);
      
      // for (gsi=gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
      // {
      //     stmt = gsi_stmt(gsi);
      //     print_gimple_stmt (stderr, stmt, 0);
      //     for (i=0; i<gimple_num_ops(stmt); ++i)
      //       if ((op = gimple_op(stmt, i)) && (str = is_str_cst(op)))
      //       {
      //           str_count++;
      //       }
      // }
      // printf("\n");
    }
    
    printf("Number of readonly strings in source code: %d\n", str_count);

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
    printf("initializing plugin");

     if (strncmp(ver->basever, plugin_ver.basever, strlen("9.3")))
       return -1;

    pass.pass = &plugin_pass.pass;
    pass.reference_pass_name = "ssa";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    register_callback("cfcss", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("cfcss", PLUGIN_INFO, NULL, &plugin_info);

    return 0;
}
