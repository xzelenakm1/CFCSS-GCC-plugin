#include <iostream>
#include <gcc-plugin.h>
#include <context.h>
// #include <coretypes.h>
// #include <diagnostic.h>
// #include <gimple.h>
// #include <tree.h>
// #include <tree-flow.h>
#include <tree-pass.h>
// #include <basic-block.h>

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

const struct pass_data cfcss_pass_data = 
{
    .type = GIMPLE_PASS,
	.name = "cfcss",
	.optinfo_flags = OPTGROUP_NONE,
	.tv_id = TV_NONE,
	.properties_required = 0,
	.properties_provided = 0,
	.properties_destroyed = 0,
	.todo_flags_start = 0,
	.todo_flags_finish = 0,
};


// int index_bb(basic_block bb) {
//   return bb->index - 2;
// }

// void assign_signatures() {
//   basic_block bb;
//   printf("Number of basic blocks: %d\n", n_basic_blocks - 2);
//   signatures = (int *) xmalloc(n_basic_blocks * sizeof(int));

//   FOR_EACH_BB(bb) {
//     int bb_index = index_bb(bb);
//     signatures[bb_index] = bb_index;
//     printf("bb index: %d, predecessor count: %d\n", signatures[bb_index], EDGE_COUNT (bb->preds));
//   }
// }

// // for vj whose pred(vj) = vi
// // difference dj = si ⊕ sj
// // insert G = G ⊕ dj
// void compute_difference_single(basic_block bb) {
//   basic_block pred_bb = single_pred(bb);
//   differences[index_bb(bb)] = signatures[index_bb(bb)] ^ signatures[index_bb(pred_bb)];
// }

// void compute_difference_multi(basic_block bb) {
//   int pred_count = EDGE_COUNT (bb->preds);
//   int pred_indexes[pred_count];
//   int i = 0, j;
//   edge e;
// 	edge_iterator ei;
// 	FOR_EACH_EDGE(e, ei, bb->preds){
// 		pred_indexes[i] = index_bb(e->src);
//     i++;
// 	}
//   // TODO - handle circular loopback to block
//   printf("For BB index %d, pred_ids are: ", index_bb(bb));
//   for (j = 0; j < pred_count; j++) {
//     printf("%d ", pred_indexes[j]);
//   }
//   printf("\n");

//   differences[index_bb(bb)] = signatures[index_bb(bb)] ^ signatures[pred_indexes[0]];
//   for (j = 0; j < pred_count; j++) {
//     adjusting_signatures[pred_indexes[j]] = signatures[pred_indexes[j]] ^ signatures[pred_indexes[0]];
//   }
// }

// void compute_differences() {
//   basic_block bb;
//   int i;
//   differences = (int *) xmalloc(n_basic_blocks * sizeof(int));
//   adjusting_signatures = (int *) xmalloc(n_basic_blocks * sizeof(int));
//   for (i = 0; i < n_basic_blocks; i++) {
//     adjusting_signatures[i] = -1;
//   }

//   FOR_EACH_BB(bb) {
//     if (index_bb(bb) == 0) {
//         differences[0] = signatures[0];
//       }
//       else if (single_pred_p(bb)) {
//         compute_difference_single(bb);
//       }
//       else {
//         compute_difference_multi(bb);
//       }

//     // int bb_index = index_bb(bb);
//     // signatures[bb_index] = bb_index;
//     // printf("bb index: %d, predecessor count: %d\n", signatures[bb_index], EDGE_COUNT (bb->preds));
//   }
// }

void execute_cfcss(function *fun)
{
    
}

struct cfcss_pass : gimple_opt_pass
{
    cfcss_pass(gcc::context *ctx)
        : gimple_opt_pass(cfcss_pass_data, ctx)
    {
    }

    unsigned int execute(function *fun)
    {
        execute_cfcss(fun);
        return 0;
    }

    bool gate(function *fun)
    {
        return true;
    }
};

int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *ver)
{
    struct register_pass_info pass;

     if (strncmp(ver->basever, plugin_ver.basever, strlen("9.3")))
       return -1;

    pass.pass = new cfcss_pass(g);
    pass.reference_pass_name = "ssa";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    register_callback("cfcss", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("cfcss", PLUGIN_INFO, NULL, &plugin_info);

    return 0;
}
