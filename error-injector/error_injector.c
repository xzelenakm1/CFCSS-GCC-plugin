#include <gcc-plugin.h>
#include <coretypes.h>
#include <diagnostic.h>
#include <gimple.h>
#include <tree.h>
#include <tree-flow.h>
#include <tree-pass.h>


int plugin_is_GPL_compatible = 1;
int fn_rand = -1, fn = 0;

static struct plugin_info error_injector_info =
{
    .version = "35",
    .help = "This is error injection plugin for use with cfcss plugin. \
    It randomly generates and inserts one jump into a compiled code. \
    Source and destination location of this jump is randomply generated.",
};

static struct plugin_gcc_version error_injector_ver =
{
    .basever = "4.6",
};

static bool error_injector_gate(void)
{
    return true;
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

static unsigned dumpcfg(void)
{
    brief_dump_cfg(stderr);
    return 0;
}

static int gen_fn_num(void)   //generate random function number
{
    struct cgraph_node *node;
    int num = 0;     // to count the functions number
    for (node = cgraph_nodes; node; node = node->next)
      if ((node->analyzed) && (cgraph_function_body_availability (node) >= 2))  //distinguish between function and a call
        num++;
    return (rand() % (num)) + 1;
}

static int gen_bb_num(int bb_prev)   //generate random basic block number
{
    int bb;
    do {
      bb = (rand() % (n_basic_blocks - 2)) + 2;
    } while (bb == bb_prev);   //it shoud not be the same as source block
    return bb;
}

int gen_stmt_num(basic_block b)    //generate random statement number
{
    gimple_stmt_iterator gsi;
    int i, stmt, stmt_num = 0;

    for (gsi = gsi_start_bb (b); !gsi_end_p (gsi); gsi_next (&gsi))
    {
      stmt_num++;                   //count statements in a basic block
    }

    stmt = (rand() % (stmt_num));

    //if stmt is 0, condition statement will be inserted before first statement in a block, othewise after stmt-th statement
    return stmt;
}

static unsigned error_injector_exec(void)  //inserts a condition statement to randomly selected function, basic block and statement
{
    basic_block bb, bbsrcnew, bbdstnew, bbsrc, bbdst, pom;
    gimple_stmt_iterator gsi, gsrc, gdst;
    gimple stsrc, stdst;
    edge esrc, edst;
    int i, BBsrc, BBdst, STsrc, STdst, Pom, OKn=0;

    srand(time(NULL));

    if (fn_rand == -1)     //fn and fn_rand are global variables to help randomly select function for jump
    {
      fn_rand = gen_fn_num();   //generate random function number
    }
    fn++;
    if (fn_rand!=fn)   //if current function is fn_rand-th then continue to generate basic blocks and statements for jump
      return 0;

    printf("Current function is: %s\n", current_function_name());

    if (n_basic_blocks < 4)   //function should have at least 2 basic blocks to be able to jump between blocks
    {
      printf("This function isn't suitable for code injection!\n");
      return 0;
    }

    //generate source basic block = where to place a condition statement
    BBsrc = gen_bb_num(-1);

    //generate destination basic block = where a condition statement will jump
    BBdst = gen_bb_num(BBsrc);

    //catch basic blocks identifiers for BBsrc and BBdst
    FOR_EACH_BB (bb)
    {
      if (bb->index == BBsrc)
	      bbsrc = bb;
      if (bb->index == BBdst)
        bbdst = bb;
    }

    calculate_dominance_info (CDI_POST_DOMINATORS);  //calculate post dominance tree to be able to search there

    if ((BBsrc != BBdst) && (dominated_by_p (CDI_DOMINATORS, bbsrc, bbdst) || dominated_by_p (CDI_POST_DOMINATORS, bbdst, bbsrc)))
    {
      //if bbdst dominates bbsrc, it's high probability to get into infinite loop
      //so src and dst will be changed (indexes and identifiers too)
      Pom = BBsrc;
      BBsrc = BBdst;
      BBdst = Pom;
      pom = bbsrc;
      bbsrc = bbdst;
      bbdst = pom;
    }
    
    printf("BBsrc %d\n", BBsrc);
    printf("BBdst %d\n", BBdst);

    //generates source statement for condition statement insertion
    STsrc = gen_stmt_num(bbsrc);
    //generates destination statement - this will be a TRUE LABEL for condition statement
    STdst = gen_stmt_num(bbdst);

    printf("STsrc %d\n", STsrc);
    printf("STdst %d\n", STdst);

    //create src and dest iterators
    gsrc = gsi_start_bb(bbsrc);  
    for (i = 0; i < STsrc-1; ++i)
      gsi_next(&gsrc);
    if (STsrc == 0)
      gsrc = gsi_start_bb(bbsrc);
    
    gdst = gsi_start_bb(bbdst);  
    for (i = 0; i < STdst-1; ++i)
      gsi_next(&gdst);
    if (STdst == 0)
      gdst = gsi_start_bb(bbdst);

    //split dest block
    bbdstnew = bbdst;
    if (STdst != 0)
    {
      stdst = gsi_stmt (gdst);
      edst = split_block (bbdst, stdst);
      bbdstnew = edst->dest;
    }

    // split block if should not jump after labels, otherwise jump to the bbdst block label
    stsrc = gimple_build_nop();
    //insert nop statement into bbsrc, to be able to split block and get label of new block for condition statement
    if (STsrc == 0)
      gsi_insert_before (&gsrc, stsrc, GSI_NEW_STMT);
    else
      gsi_insert_after (&gsrc, stsrc, GSI_NEW_STMT);
    esrc = split_block (bbsrc, stsrc);
    bbsrcnew = esrc->src;

    //create condition statement and insert it into basic block
    stsrc = gimple_build_cond (EQ_EXPR, build_int_cst (integer_type_node, 1), build_int_cst (integer_type_node, 1), gimple_block_label(bbdstnew), gimple_block_label(bbsrcnew));
    gsi_insert_after (&gsrc, stsrc, GSI_NEW_STMT);

    //change esrc edge from FALLTHRU to FALSE VALUE
    if (esrc->flags & EDGE_FALLTHRU)
      esrc->flags = esrc->flags - EDGE_FALLTHRU;   
    if (!(esrc->flags & EDGE_FALSE_VALUE))
      esrc->flags = esrc->flags + EDGE_FALSE_VALUE;   

    edst = make_edge (bbsrc, bbdstnew, EDGE_TRUE_VALUE);  //createt new TRUE VALUE edge from bbsrc to bbdstnew basic blocks
    //recalculate dominators
    if (dom_info_available_p (CDI_DOMINATORS))
    {
        set_immediate_dominator (CDI_DOMINATORS, bbsrc, recompute_dominator (CDI_DOMINATORS, bbsrc));
        set_immediate_dominator (CDI_DOMINATORS, bbsrcnew, recompute_dominator (CDI_DOMINATORS, bbsrcnew));
	set_immediate_dominator (CDI_DOMINATORS, bbdst, recompute_dominator (CDI_DOMINATORS, bbdst));
	if (bbdstnew!=bbdst)
          set_immediate_dominator (CDI_DOMINATORS, bbdstnew, recompute_dominator (CDI_DOMINATORS, bbdstnew));
    }
    return 0;
}

/* See tree-pass.h for a list and desctiptions for the fields of this struct */
static struct gimple_opt_pass error_injector_pass = 
{
    .pass.type = GIMPLE_PASS,
    .pass.name = "error_injector",       /* For use in the dump file */
    .pass.gate = error_injector_gate,
    .pass.execute = error_injector_exec, /* Pass handler/callback */
};


/* Return 0 on success or error code on failure */
int plugin_init(struct plugin_name_args   *info,  /* Argument infor */
                struct plugin_gcc_version *ver)   /* Version of GCC */
{
    struct register_pass_info pass;

     if (strncmp(ver->basever, error_injector_ver.basever, strlen("4.6")))
       return -1; /* Incorrect version of gcc */

    pass.pass = &error_injector_pass.pass;
    pass.reference_pass_name = "veclower";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    /* Tell gcc we want to be called after the first SSA pass */
    register_callback("error_injector", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("error_injector", PLUGIN_INFO, NULL, &error_injector_info);

    return 0;
}