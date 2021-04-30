#include <gcc-plugin.h>
#include <coretypes.h>
#include <diagnostic.h>
#include <gimple.h>
#include <tree.h>
#include <tree-flow.h>
#include <tree-pass.h>

int plugin_is_GPL_compatible = 1;

static struct plugin_info my_plugin_info =
{
    .version = "5",
    .help = "This is my first plugin",
};


static struct plugin_gcc_version my_plugin_ver =
{
    .basever = "4.6",
};

static bool my_plugin_gate(void)
{
    return true;
}


static const_tree is_str_cst(const_tree node)
{
    const_tree str = node;

    if (TREE_CODE(str) == VAR_DECL)
    {
        if (!(str = DECL_INITIAL(node)))
          return NULL_TREE;
        else if (TREE_OPERAND_LENGTH(str))
          str = TREE_OPERAND(str, 0);
    }
    else if (TREE_CODE(str) == ADDR_EXPR &&
             TREE_OPERAND_LENGTH(str) > 0)
      str = TREE_OPERAND(str, 0);

    if (TREE_CODE(str) != STRING_CST &&
        TREE_OPERAND_LENGTH(str) > 0)
      str = TREE_OPERAND(str, 0);

    if (TREE_CODE(str) != STRING_CST)
      return NULL_TREE;
    else
      return str;
}

static unsigned my_plugin_exec(void)
{
    unsigned i;
    const_tree str, op;
    basic_block bb;
    gimple stmt;
    gimple_stmt_iterator gsi;
    int str_count = 0;

    FOR_EACH_BB(bb)
      for (gsi=gsi_start_bb(bb); !gsi_end_p(gsi); gsi_next(&gsi))
      {
          stmt = gsi_stmt(gsi);
          for (i=0; i<gimple_num_ops(stmt); ++i)
            if ((op = gimple_op(stmt, i)) && (str = is_str_cst(op)))
            {
                str_count++;
            }
      }
    
    printf("Number of readonly strings in source code: %d\n", str_count);

    return 0;
}

static struct gimple_opt_pass my_plugin_pass = 
{
    .pass.type = GIMPLE_PASS,
    .pass.name = "my_plugin",
    .pass.gate = my_plugin_gate,
    .pass.execute = my_plugin_exec,
};

int plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *ver)
{
    struct register_pass_info pass;

     if (strncmp(ver->basever, my_plugin_ver.basever, strlen("4.6")))
       return -1;

    pass.pass = &my_plugin_pass.pass;
    pass.reference_pass_name = "ssa";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    register_callback("my_plugin", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("my_plugin", PLUGIN_INFO, NULL, &my_plugin_info);

    return 0;
}