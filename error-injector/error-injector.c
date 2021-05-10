#include <gcc-plugin.h>
#include <coretypes.h>
#include <diagnostic.h>
#include <gimple.h>
#include <tree.h>
#include <tree-flow.h>
#include <tree-pass.h>


int plugin_is_GPL_compatible = 1;


static struct plugin_info codeinject_info =
{
    .version = "5",
    .help = "This is code injection plugin for use with cfcss plugin",
};

static struct plugin_gcc_version codeinject_ver =
{
    .basever = "4.6",
};

static bool codeinject_gate(void)
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


static unsigned codeinject_exec(void)  //VLOZI GOTO do programu test2.c do funkcie main, do <bb 3> za 3-ti STATEMENT
{
    unsigned i;
    basic_block bb, bbs, bbnew, bbto;
    gimple_stmt_iterator gsi, gsis;
    gimple stmts, stmtgoto;
    edge e;

    if (get_identifier(current_function_name())!=get_identifier("main")) 
	    return 0;

    //ALL_BB_print();
    //dumpcfg();

    FOR_EACH_BB (bb)
    {
      if (bb->index == 3)
      {
        gsi=gsi_start_bb(bb);  
        for (i=0; i<3; ++i)
          gsi_next(&gsi);
        gsis = gsi;
        bbs = bb;
      }
      if (bb->index == 8)
	      bbto = bb;
    }

    stmts = gsi_stmt (gsis);
    //e = split_block (bbs, stmts);
    stmtgoto = gimple_build_goto (gimple_block_label(bbto));
    gsi_insert_after (&gsis, stmtgoto, GSI_NEW_STMT);
    e = split_block (bbs, stmtgoto);
    //printf("AFTER ----------------------------------------\n");
    //ALL_BB_print();
    //dumpcfg();
    return 0;
}

/* See tree-pass.h for a list and desctiptions for the fields of this struct */
static struct gimple_opt_pass codeinject_pass = 
{
    .pass.type = GIMPLE_PASS,
    .pass.name = "codeinject",       /* For use in the dump file */
    .pass.gate = codeinject_gate,
    .pass.execute = codeinject_exec, /* Pass handler/callback */
};


/* Return 0 on success or error code on failure */
int plugin_init(struct plugin_name_args   *info,  /* Argument infor */
                struct plugin_gcc_version *ver)   /* Version of GCC */
{
    struct register_pass_info pass;

     if (strncmp(ver->basever, codeinject_ver.basever, strlen("4.6")))
       return -1; /* Incorrect version of gcc */

    pass.pass = &codeinject_pass.pass;
    pass.reference_pass_name = "veclower";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    /* Tell gcc we want to be called after the first SSA pass */
    register_callback("codeinject", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("codeinject", PLUGIN_INFO, NULL, &codeinject_info);

    /* Initialize our code injection */
    //if (!(codeinject_g = init_codeinjectchecker()))
    //  return -1;

    return 0;
}
