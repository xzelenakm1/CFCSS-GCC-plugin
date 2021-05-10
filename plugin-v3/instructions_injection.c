/******************************************************************************
 * speller.c 
 *
 * speller - GCC Spell Checking Plugin
 *
 * Copyright (C) 2011 Matt Davis (enferex) of 757Labs mattdavis9@gmail.com
 *
 * speller is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * speller is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with speller.  If not, see .
 *****************************************************************************/

#include <gcc-plugin.h>
#include <coretypes.h>
#include <diagnostic.h>
#include <gimple.h>
#include <tree.h>
#include <tree-flow.h>
#include <tree-pass.h>


#define is_alpha(c) (((c)>64 && (c)<91) || ((c)>96 && (c)<123))


int plugin_is_GPL_compatible = 1;


/* Help info about the plugin if one were to use gcc's --version --help */
static struct plugin_info speller_info =
{
    .version = "42",
    .help = "Hahahaha yeaaaaa....",
};


static struct plugin_gcc_version speller_ver =
{
    .basever = "4.6",
};


/* We don't need to run any tests before we execute our plugin pass */
static bool speller_gate(void)
{
    return true;
}



void ALL_STMT_in_BB_print(basic_block bab)
{
    gimple_stmt_iterator gsi;
    for (gsi = gsi_start_bb (bab); !gsi_end_p (gsi); gsi_next (&gsi))
    {
      print_gimple_stmt (stderr, gsi_stmt(gsi), 0, TDF_SLIM);
    }
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


static unsigned speller_exec2(void)
{
    basic_block bb;
    gimple_stmt_iterator gsi;
    gimple_seq seq;
    ALL_BB_print();
    return 0;
}

static unsigned dumpcfg(void)
{
    brief_dump_cfg(stderr); //**** NEPOZNA VER 9
    return 0;
}

static unsigned speller_exec4(void)
{
    basic_block bb;
    gimple_stmt_iterator gsi;
    gimple_seq seq;

    ALL_BB_print();
    dumpcfg();
    
    FOR_EACH_BB (bb)
    {
      //gsi=gsi_start_bb(bb);
      //gimple stmt = gsi_stmt (gsi);
      //newseq = bb_seq (bb);
      //gimple_seq_init (newseq);
    }
    ALL_BB_print();
    dumpcfg();
    return 0;
}

static unsigned speller_exec(void)
{
    unsigned i;
    basic_block bb, bbs;
    gimple_stmt_iterator gsi, gsis;
    gimple_seq seq;
    edge e;
    gimple stmt, stmts;
    
    ALL_BB_print();
    dumpcfg();

    FOR_EACH_BB (bb)
    {
      gsi=gsi_start_bb(bb);  
      for (i=0; i<10; ++i)
        gsi_next(&gsi);
      gsis = gsi;
      bbs = bb;
    }
    stmts = gsi_stmt (gsis);
    print_gimple_stmt (stderr, gsi_stmt(gsi), 0, TDF_SLIM);
    printf("---------------- before split --------- \n");
    e = split_block (bbs, stmts); 
    e = split_block_after_labels (e->dest);

    printf("---------------- after split --------- \n");
    //update_stmt (stmts);
    ALL_BB_print();
    dumpcfg();

    return 0;
}



static unsigned speller_exec_old(void)
{
    unsigned i, rm, ussa;
    const_tree str, op;
    basic_block bb;
    gimple stmt, stmtnew, stmtassign, stmtcond, stmtlblbeg, stmtlblaftercond, stmtlblend;
    gimple_stmt_iterator gsi;
    gimple_seq seq;
    tree lhs, rhs1, rhs2;
    char text;
    tree var;
    ssa_op_iter iter;
    use_operand_p use_p;
    gimple use_stmt; //na odskusanie USE_STMT
    gimple ssa_name_stmt; //na odskusanie SSA_NAME_STMT
    tree lblbeg, lblend, lblaftercond, lblbb2;
    tree jano, mato, spolu, v3000;
    int iplen;
    tree cislo;
    tree x,y,z;
    tree memlblbeg, memlblend, memlblaftercond;

    
    //cislo = INTEGER_CST(1000);
    //cislo = 1000;

    //stmtnew = gimple_build_asm_vec("mov %%eax, %%eax", NULL, NULL, NULL, NULL);
    //stmtnew=gimple_build_nop();
    //stmtnew=gimple_build_assign_with_ops(PLUS_EXPR, lhs, rhs1, rhs2);
    //stmtnew=gimple_build_assign_with_

    // ************************************************TREE_CODE(rhs1) = VAR_DECL;
    

    // from /include/gimple.h
    //extern tree create_tmp_var_raw (tree, const char *);
    //extern tree create_tmp_var_name (const char *);
    //extern tree create_tmp_var (tree, const char *);
    //extern tree create_tmp_reg (tree, const char *);
    //extern tree get_initialized_tmp_var (tree, gimple_seq *, gimple_seq *);
    //extern tree get_formal_tmp_var (tree, gimple_seq *);
    //extern void declare_vars (tree, gimple, bool);
    //extern void annotate_all_with_location (gimple_seq, location_t);


    // from gimplify.h z gcc 9 vo virtualke
    //extern void gimplify_and_add (tree, gimple_seq *);
    //extern tree get_formal_tmp_var (tree, gimple_seq *);
    //extern tree get_initialized_tmp_var (tree, gimple_seq *, gimple_seq *, bool = true);
    //extern void declare_vars (tree, gimple *, bool);
    //extern void gimple_add_tmp_var (tree);
    //extern void gimple_add_tmp_var_fn (struct function *, tree);

    //------------------ CREATE VARIABLES ---------------------

    jano = create_tmp_var (integer_type_node, "JANO");
    mato = create_tmp_var (integer_type_node, "MATO");
    spolu = create_tmp_var (integer_type_node, "SPOLU");
    //jano = add_new_static_var(integer_type_node);
    //DECL_NAME(jano) = get_identifier ("JANO");
    //DECL_INITIAL(jano) = get_identifier("1000");
    DECL_INITIAL (jano) = build_int_cst (integer_type_node, 1000);
    //DECL_INITIAL(jano) = 1000;
    //mato = add_new_static_var(integer_type_node);
    //DECL_NAME(mato) = get_identifier ("MATO");
    //DECL_INITIAL(mato) = get_identifier("2000");
    DECL_INITIAL (mato) = build_int_cst (integer_type_node, 2000);
    //spolu = add_new_static_var(integer_type_node);
    //DECL_NAME(spolu) = get_identifier ("SPOLU");
    //v3000 = get_identifier("3000");
    v3000 = build_int_cst (integer_type_node, 3000);

    //printf("PREMENNA JANO JE SSANAME %d\n", SSA_VAR_P(jano)); //ANO
    //printf("PREMENNA MATO JE SSANAME %d\n", SSA_VAR_P(mato)); //ANO
    //printf("PREMENNA SPOLU JE SSANAME %d\n", SSA_VAR_P(spolu)); //ANO
    //printf("PREMENNA JANO JE GIMPLE REG %d\n", is_gimple_reg(jano)); //NIE = VIRTUALNY OPERATOR
    //printf("PREMENNA MATO JE GIMPLE REG %d\n", is_gimple_reg(mato));  //NIE = VIRTUALNY OPERATOR
    //printf("PREMENNA SPOLU JE GIMPLE REG %d\n", is_gimple_reg(spolu));  //NIE = VIRTUALNY OPERATOR
    //printf("PREMENNA SPOLU JE TREE_EXTERNAL %d\n", TREE_EXTERNAL(spolu)); //gcc NEPOZNA TENTO PARAMETER
    //printf("PREMENNA SPOLU JE TREE_PUBLIC %d\n", TREE_PUBLIC(spolu)); //NIE, CO JE TROCHU DIVNE,LEBO JE STATIC

    mark_sym_for_renaming(jano);
    mark_sym_for_renaming(mato);
    mark_sym_for_renaming(spolu);

    //x = maybe_get_identifier ("x"); // **** NEPOZNA VER 9
    //y = maybe_get_identifier ("y");
    //z = maybe_get_identifier ("z");
    //------------------ CREATE LABELS ---------------------
    //IDENTIFIER_POINTER (lblbeg) = get_identifier ("JANO");
    lblbeg = build0_stat (LABEL_DECL, memlblbeg);
    DECL_NAME(lblbeg) = get_identifier ("LBLBEG");
    lblend = build0_stat (LABEL_DECL, memlblend);
    DECL_NAME(lblend) = get_identifier ("LBLEND");
    lblaftercond = build0_stat (LABEL_DECL, memlblaftercond);
    DECL_NAME(lblaftercond) = get_identifier ("LBLAFTERCOND");

    //lblbb2 = gimple_block_label(bb);


    //DECL_CONTEXT(lblbeg) -> cfun;
	    //DECL_SAVED_TREE(cfun);
	    //FUNCTION_DECL(cfun);
    //iplen = IDENTIFIER_LENGTH (lblbeg);
    //printf("IDENTIFIER POINTER LENGTH= %d\n", iplen);
    //put_decl_node (TREE_CODE (DECL_CONTEXT (node)) == FUNCTION_DECL
    //DECL_CONTEXT (node)
    //TYPE_NAME (DECL_CONTEXT (node)),
    //verbosity);

    //------------------ BUILD LABEL STATEMENTS ---------------------
    stmtlblbeg=gimple_build_label (lblbeg);
    stmtlblend=gimple_build_label (lblend);
    stmtlblaftercond=gimple_build_label (lblaftercond);
    //------------------ BUILD ASSIGN STATEMENTS ---------------------
    stmtassign=gimple_build_assign_with_ops(PLUS_EXPR, spolu, jano, mato); // ****NEPOZNA VER 9
    //BIT_XOR_EXPR
    
    //------------------ BUILD CONDITION STATEMENTS ---------------------
    //stmtcond=gimple_build_cond (EQ_EXPR, spolu, v3000, lblend, lblaftercond);

    
    

    //stmtnew=gimple_build_assign_with_ops(PLUS_EXPR, z, x, y);
    //

    //IDENTIFIER_POINTER (jano) = get_identifier ("JANO");
    //iplen = IDENTIFIER_LENGTH (jano);
    //printf("IDENTIFIER POINTER LENGTH= %d\n", iplen);
    //printf("DECL_NAME= %s\n", DECL_NAME(jano));
    //dump_ssaname_info (stderr, jano, 0, TDF_SLIM);
    dump_varpool(stderr);


    //SSA_NAME_DEF_STMT (var)
    //SSA_NAME_VERSION (var)

    //jano = create_tmp_var (jano, "JANO");
    //jano = create_tmp_var (jano);

    //add_new_static_var, in cgraph.h                  // ********************************************
    //tree global_var = add_new_static_var(integer_type_node);
    //if you want to name the var:
    //tree name = get_identifier("my_global_name");
    //DECL_NAME(global_var) = name;

    //dump_varpool (stderr);
    //dump_cgraph (stderr);
    
    //debug_immediate_uses (); //vypise vsetky pouzitia vsetkych premennych
    //dump_immediate_uses (stderr);  //presne to iste, ako debug_immediate_uses ()


    //stmtnew=gimple_build_assign_with_ops(PLUS_EXPR, lhs, rhs1, rhs2);
    //NON_LVALUE_EXPR


    //FOR_ALL_BB (bb)
    FOR_EACH_BB (bb)
    {
      //gimple_dump_bb (bb, stderr, 0, TDF_SLIM);
      gimple_stmt_iterator si;
      for (si = gsi_start_bb (bb); !gsi_end_p (si); gsi_next (&si))
      {
        printf("NEW STATEMENT----------------------------------------------------------\n");
        gimple stmt = gsi_stmt (si);
        //print_gimple_stmt (stderr, stmt, 0, TDF_SLIM);  //vypise vsetky statementy
        //debug_gimple_stmt (stmt);          // TO ISTE AKO print, len s flagmi DF_VOPS|TDF_MEMSYMS
        //print_gimple_expr (stderr, stmt, 0, TDF_SLIM);  printf("\n");   //TO ISTE AKO print_gimple_stmt, len bez LHS
        
        for (i=0; i<gimple_num_ops(stmt); ++i)
        {
          op = gimple_op(stmt, i); 
        }
        //FOR_EACH_SSA_TREE_OPERAND (var, stmt, iter, SSA_OP_ALL_OPERANDS) //toto vypisovalo dvojmo premenne - NEPOUZIVAT!
        //FOR_EACH_SSA_TREE_OPERAND (var, stmt, iter, SSA_OP_USE)
        //FOR_EACH_SSA_USE_OPERAND (use_p, stmt, iter, (SSA_OP_USE | SSA_OP_VUSE))
        {
          //ssa_name_stmt = SSA_NAME_DEF_STMT (var); //toto asi davalo prazdny znak = nefungovalo to
          //use_stmt = USE_STMT(use_p);
          //print_gimple_stmt (stderr, stmt, 0, TDF_SLIM);
          //printf("VUSE z TOHOTO statementu bola vytvorena tu: ");
          //print_gimple_stmt (stderr, ssa_name_stmt, 0, TDF_SLIM);
          //debug_immediate_uses_for (var);
          //dump_immediate_uses_for (stderr, var);  //to iste ako debug_immediate_uses_for
          //dump_ssaname_info_to_file (stderr, var, 0, TDF_SLIM);  //chyba - mozno treba ziskat SSA_NAME ****************
        }
        printf("************************************\n");
        // SSA_OP_VUSE
        //print_gimple_seq (FILE *, gimple_seq, int, dump_flags_t);
        //debug_gimple_seq (gimple_seq); 
        //vzorovy riadok deklaracie funkcieprint_gimple_expr (FILE *, gimple *, int, dump_flags_t = TDF_NONE);
      }
    }

    //        for var in gcc.get_variables():
    //        type_ = var.decl.type

    //        if DEBUG:
    //            print('var.decl: %r' % var.decl)
    //            print(type_)

    //        # Don't bother warning about const data:
    //        if is_const(type_):
    //            continue



    FOR_EACH_BB(bb)
    {
      gsi=gsi_last_bb(bb);
      gsi_prev(&gsi);
      gsi_insert_before (&gsi, stmtlblend, GSI_SAME_STMT);
      update_stmt (stmtlblend);
    }

    FOR_EACH_BB(bb)
    {
      gsi=gsi_after_labels(bb);
      gsi_insert_before (&gsi, stmtlblaftercond, GSI_SAME_STMT);
      update_stmt (stmtlblaftercond);
    }

    //gimple_dump_bb (bb, stderr, 0, TDF_SLIM);
    //print_gimple_stmt (stderr, gsi_stmt (gsi), 0, TDF_SLIM);

    FOR_EACH_BB(bb)
    {
      gsi=gsi_start_bb(bb);
      //gsi=gsi_after_labels(bb);
      //gsi_insert_after (&gsi, stmtlblbeg, GSI_SAME_STMT); // *** TOTO ROBI PROBLEM S GLOB PREMENNYMI ***
      //update_stmt (stmtlblbeg);
      gsi_insert_before (&gsi, stmtassign, GSI_SAME_STMT); // *** TOTO ROBI PROBLEM S GLOB PREMENNYMI ***
      update_stmt (stmtassign);

      gimple_dump_bb (bb, stderr, 0, TDF_SLIM);

      lblbb2 = gimple_block_label(bb);
      //stmtcond=gimple_build_cond (EQ_EXPR, spolu, v3000, lblend, lblaftercond);
      stmtcond=gimple_build_cond (EQ_EXPR, spolu, v3000, lblend, lblbb2);


      gsi_insert_before (&gsi, stmtcond, GSI_SAME_STMT);
      update_stmt (stmtcond);
      gimple_dump_bb (bb, stderr, 0, TDF_SLIM);

      //gsi=gsi_last_bb(bb);
      //gsi_insert_before (&gsi, stmtlblend, GSI_SAME_STMT);
      //update_stmt (stmtnew);
      

      //
      //gsi_insert_before (&gsi, nopasm, GSI_SAME_STMT);
      //gsi=gsi_start_bb(bb);
      //for (i=0; i<13; ++i)
      //  gsi_next(&gsi);
      //gsi_remove(&gsi, rm);
      //gsi_insert_after (&gsi, stmtnew, GSI_NEW_STMT);
    }
    //update_ssa(ussa); // ********* TOTO ROBI PROBLEM S GLOB AJ LOC PREMENNYMI ************************
    return 0;
}


/* See tree-pass.h for a list and desctiptions for the fields of this struct */
static struct gimple_opt_pass speller_pass = 
{
    .pass.type = GIMPLE_PASS,
    .pass.name = "speller",       /* For use in the dump file */
    .pass.gate = speller_gate,
    .pass.execute = speller_exec, /* Pass handler/callback */
};


/* Return 0 on success or error code on failure */
int plugin_init(struct plugin_name_args   *info,  /* Argument infor */
                struct plugin_gcc_version *ver)   /* Version of GCC */
{
    struct register_pass_info pass;

     if (strncmp(ver->basever, speller_ver.basever, strlen("4.6")))
       return -1; /* Incorrect version of gcc */

    pass.pass = &speller_pass.pass;
    pass.reference_pass_name = "ssa";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    /* Tell gcc we want to be called after the first SSA pass */
    register_callback("speller", PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback("speller", PLUGIN_INFO, NULL, &speller_info);

    /* Initilize our spell checker */
    //if (!(speller_g = init_spellchecker()))
    //  return -1;

    return 0;
}
