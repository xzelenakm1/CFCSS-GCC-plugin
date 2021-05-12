#include <gcc-plugin.h>
#include <coretypes.h>
#include <diagnostic.h>
#include <gimple.h>
#include <tree.h>
#include <tree-flow.h>
#include <tree-pass.h>


int plugin_is_GPL_compatible = 1;
int fnrand=-1, fn=0;
tree fnabort=NULL;

static struct plugin_info codeinject_info =
{
    .version = "15",
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

static int genFNnum(void)   //generate random function number
{
    struct cgraph_node *node;
    int num=0;
    //need to count the functions number
    for (node = cgraph_nodes; node; node = node->next)
    {
      if ((node->analyzed)&&(cgraph_function_body_availability (node)>=2))  //this is function, not a call
        num++;
      //printf("uid %d\n", node->uid);
      //printf("analyzed %d\n", node->analyzed);
      //printf("reachable %d\n", node->reachable);
      //printf("needed %d\n", node->needed);
      //debug_cgraph_node (node);
    }
    //printf("Pocet funkcii v programe %d\n", num);
    return (rand() % (num)) + 1;
}

static int genBBnum(int exist[last_basic_block])   //generate random basic block number
{
    int c;
    do {
      c = (rand() % (last_basic_block - 2)) + 2;
      //printf("b RANDOM number %d\n", c);
    } while (exist[c]!=1);   //it shoud exist and shoud not be a bad block
    //printf("c RANDOM number vysledny %d\n", c);
    return c;
}

int genSTnum(basic_block b, int Sprev)    //generate random statement number
{
    gimple_stmt_iterator gsi;
    int i, S, Snum=0;

    for (gsi = gsi_start_bb (b); !gsi_end_p (gsi); gsi_next (&gsi))
    {
      Snum++;
      //print_gimple_stmt (stderr, gsi_stmt(gsi), 0, TDF_SLIM);
    }
    //printf("Snum %d\n", Snum);
    do {
      S = (rand() % (Snum));
      //printf("S RANDOM number %d\n", S);
    } while (S==Sprev);
    //} while ((S==Sprev)||(S==Snum));   //don't want to have the same statement numbers and last statement too
    //if S is 0, goto statement will be inserted before first statement in a block, othewise after S-th statement
    //printf("S RANDOM number %d\n", S);
    //if (S>0)
    //{
      //gsi=gsi_start_bb(b);
      //printf("PRVY STATEMENT \n");
      //print_gimple_stmt (stderr, gsi_stmt(gsi), 0, TDF_SLIM);
      //for (i=0; i<S-1; ++i)
        //gsi_next(&gsi);
      //printf("VYBRANY STATEMENT \n");
      //print_gimple_stmt (stderr, gsi_stmt(gsi), 0, TDF_SLIM);
    //}
    //else
      //printf("PRED PRVY STATEMENT \n");
    return S;
}

static unsigned init_codeinject(void)  //initialisation for codeinject_exec
{
    return 0;
}

static unsigned codeinject_exec(void)  //inserts a goto statement to randomly selected function, basic block and statement
{
    basic_block bb, bbsrcnew, bbdstnew, bbsrc, bbdst, pom;
    gimple_stmt_iterator gsi, gsrc, gdst;
    gimple stsrc, stdst;
    edge esrc, edst;
    int i, BBsrc, BBdst, STsrc, STdst, Pom;
    bool is_OK=FALSE;

    printf("Current function is: %s\n", current_function_name());
    srand(time(NULL));

    if (fnrand==-1)     //fn and fnrand are global variables to help randomly select function for goto
    {
      fnrand=genFNnum();   //generate random function number
      fnrand=7;
      printf("Poradove cislo vybranej funkcie je: %d\n", fnrand);
    }
    fn++;
    if (fnrand!=fn)   //if current function is fnrand-th then continue and generate basic block and statement for goto
      return 0;
    
    //ALL_BB_print();
    //dumpcfg();
    
    //BBsrcnum = last_basic_block;
    Pom = last_basic_block_for_function(cfun); 
    printf("Pocet blokov %d\n", Pom);

    int bbexist[last_basic_block];
    //vyrobime pole kvoli neexistujucim blokom
    //niekedy sa stane, ze last_basic_block je viac, ako v skutocnosti je pocet blokov

    for (i=0; i<=last_basic_block; ++i)
      bbexist[i]=0;

    FOR_EACH_BB (bb)
    {
      if (empty_block_p (bb))
      {
        //printf("PRAZDNY BLOK \n");
	//gimple_dump_bb (bb, stderr, 0, TDF_SLIM);
      }
      bbexist[bb->index] = 1;         //blok existuje
      if (gsi_end_p (gsi_start_bb(bb)))
        bbexist[bb->index] = 2;       //takto oznacime blok, ktory moze byt pokazeny
      gsi=gsi_start_bb(bb);
      gsi_next(&gsi);
      if (gsi_end_p (gsi))
        bbexist[bb->index] = 2;       //ta
    }

    //KONTROLA NA TO, CI JE ASPON jeden prvok pola bbexist rovny 1
    for (i=0; i<=last_basic_block; ++i)
    {
      //printf("bbexist[%d]=%d\n", i, bbexist[i]);
      if (bbexist[i]==1)
        is_OK = TRUE;
    }

    if (!is_OK)
    {
	    printf("Tato funkcia nema ziaden dobry blok a preto koncim \n");
	    return 0;
    }

    //
    //generate source basic block = where to place a goto statement
    BBsrc = genBBnum(bbexist);
    printf("BBsrc RANDOM number vysledny %d\n", BBsrc);

    //generate destination basic block = where a goto statement will jump
    BBdst = genBBnum(bbexist);
    printf("BBdst RANDOM number vysledny %d\n", BBdst);

    //catch basic blocks identifiers for BBsrc and BBdst
    FOR_EACH_BB (bb)
    {
      if (bb->index == BBsrc)
	      bbsrc = bb;
      if (bb->index == BBdst)
              bbdst = bb;
    }

    calculate_dominance_info (CDI_POST_DOMINATORS);
    //najskor preverime dominatorov, aby sme v nich mohli vyhladavat
    //verify_dominators (CDI_DOMINATORS); //this is just a debug function
    //bb1 is dominatd by bb2
    //bb2 dominates to bb1

    //if (dominated_by_p (CDI_DOMINATORS, bbsrc, bbdst))
    //if ((BBsrc!=BBdst)&&(dominated_by_p (CDI_DOMINATORS, bbsrc, bbdst)||dominated_by_p (CDI_POST_DOMINATORS, bbdst, bbsrc)))
    if ((BBsrc!=BBdst)&&((!dominated_by_p (CDI_DOMINATORS, bbdst, bbsrc))||dominated_by_p (CDI_POST_DOMINATORS, bbdst, bbsrc)))
    {
      //if bbdst dominates bbsrc, it's high probability to get into infinite loop
      //so src and dst will be changed (indexes and identifiers too)
      printf("CDI_DOMINATORS PODMIENKA %d\n", dominated_by_p (CDI_DOMINATORS, bbsrc, bbdst));
      printf("!CDI_DOMINATORS PODMIENKA %d\n", !dominated_by_p (CDI_DOMINATORS, bbdst, bbsrc));
      printf("CDI_POST_DOMINATORS PODMIENKA %d\n", dominated_by_p (CDI_POST_DOMINATORS, bbdst, bbsrc));
      printf("Vymenime bloky \n");
      //printf("BBsrc %d\n", BBsrc);
      //printf("BBdst %d\n", BBdst);
      Pom = BBsrc;
      BBsrc = BBdst;
      BBdst = Pom;
      pom = bbsrc;
      bbsrc = bbdst;
      bbdst = pom;
    }
    
    //let's generate source statement, from which will be jumped = there will be inserted a goto statement

    STsrc = genSTnum(bbsrc, -1);
    STdst = genSTnum(bbdst, STsrc);
    printf("STsrc %d\n", STsrc);
    printf("STdst %d\n", STdst);

    if ((BBsrc==BBdst)&(STsrc>STdst))  
    {
      //can't jump back in the same block because of infinite loop
      //so src and dst will be changed
      printf("Vymenime statementy \n");
      //printf("STsrc %d\n", STsrc);
      //printf("STdst %d\n", STdst);
      Pom = STsrc;
      STsrc = STdst;
      STdst = Pom;
    }

    //create src and dest iterators
    gsrc=gsi_start_bb(bbsrc);  
    for (i=0; i<STsrc-1; ++i)
      gsi_next(&gsrc);
    if (STsrc==0)
      gsrc=gsi_start_bb(bbsrc);
    
    gdst=gsi_start_bb(bbdst);  
    for (i=0; i<STdst-1; ++i)
      gsi_next(&gdst);
    if (STdst==0)
      gdst=gsi_start_bb(bbdst);

    //split dest block
    bbdstnew = bbdst;
    if (STdst!=0)
    {
      stdst = gsi_stmt (gdst);
      edst = split_block (bbdst, stdst);
      bbdstnew = edst->dest;
      //printf("edst FLAGS %d\n", edst->flags);   // ************************* OVERIT, CI JE FALLTHRU
    }

    // split block if should not jump after labels, otherwise jump to the bbdst block label
    //stsrc = gimple_build_goto (gimple_block_label(bbdstnew));
    stsrc = gimple_build_nop();
    //insert goto statement into bbsrc
    if (STsrc==0)
      gsi_insert_before (&gsrc, stsrc, GSI_NEW_STMT);
    else
      gsi_insert_after (&gsrc, stsrc, GSI_NEW_STMT);
    esrc = split_block (bbsrc, stsrc);
    bbsrcnew = esrc->src;

    stsrc = gimple_build_cond (EQ_EXPR, build_int_cst (integer_type_node, 1), build_int_cst (integer_type_node, 1), gimple_block_label(bbdstnew), gimple_block_label(bbsrcnew));
    gsi_insert_after (&gsrc, stsrc, GSI_NEW_STMT);

    //printf("Miesto 1 \n");
    //***************************************** TODO*****************************************
    //1) OTESTOVAT VKLADANIE STATEMENTOV A VYTVARANIE EDGOV
    //2) OTESTOVAT PODMIENKY NA DOMINATOROV PRE PREHODENIE bbsrc a bbdst
    //3) ROZHODNUT, CI esrc NECHAT FALLTHRU ALEBO JU REDIREKTNUT A DAT JEJ FLAG GOTO = 0, T.J. ZMAZAT FALLTHRU FLAG
    // A CI VYTVORIT NOVU EDGE PRE JUMP GOTO, AK ANO, TAK AKYM SPOSOBOM - 4 MOZNOSTI

    if (esrc->flags & EDGE_FALLTHRU)
      esrc->flags = esrc->flags - EDGE_FALLTHRU;   //*** ZRUSIM FALLTHRU = SPRAVIM JUMP GOTO
    if (!(esrc->flags & EDGE_FALSE_VALUE))
      esrc->flags = esrc->flags + EDGE_FALSE_VALUE;   //*** NASTAVIM FALSE VETVU

    //execute_on_shrinking_pred(esrc);
    //remove_edge(esrc);
    //edst = make_edge (bbsrc, bbdstnew, EDGE_TRUE_VALUE);
    edst = make_edge (bbsrc, bbdstnew, EDGE_TRUE_VALUE);  //*** VYTVORIM NOVU EDGE JUMP GOTO, AK SA TAK ROZHODNEME
    //printf("Miesto 2 \n");

    //
    //redirect_edge_succ (esrc, bbdstnew);  //low level without updating instruction chain
    //edst = redirect_edge_succ_nodup (esrc, bbdstnew);  //the same, but avoid possible duplicate edge
    //edst = redirect_edge_and_branch (esrc, bbdstnew);  //may return NULL, when not successfull
    //basic_block redirect_edge_and_branch_force (esrc, bbdstnew);  //allows rediresting falltrhu edges, creates forwarder block

    if (dom_info_available_p (CDI_DOMINATORS))
    {
        set_immediate_dominator (CDI_DOMINATORS, bbsrc, recompute_dominator (CDI_DOMINATORS, bbsrc));
        set_immediate_dominator (CDI_DOMINATORS, bbsrcnew, recompute_dominator (CDI_DOMINATORS, bbsrcnew));
	set_immediate_dominator (CDI_DOMINATORS, bbdst, recompute_dominator (CDI_DOMINATORS, bbdst));
	if (bbdstnew!=bbdst)
          set_immediate_dominator (CDI_DOMINATORS, bbdstnew, recompute_dominator (CDI_DOMINATORS, bbdstnew));
    }
    //printf("Miesto 3 \n");
    if (dom_info_available_p (CDI_POST_DOMINATORS))
    {
        //printf("Miesto 4 \n");
	//set_immediate_dominator (CDI_POST_DOMINATORS, bbsrc, recompute_dominator (CDI_POST_DOMINATORS, bbsrc));
	//printf("Miesto 5 \n");
        //set_immediate_dominator (CDI_POST_DOMINATORS, bbsrcnew, recompute_dominator (CDI_POST_DOMINATORS, bbsrcnew));
	//printf("Miesto 6 \n");
        //set_immediate_dominator (CDI_POST_DOMINATORS, bbdst, recompute_dominator (CDI_POST_DOMINATORS, bbdst));
	//printf("Miesto 7 \n");
        //if (bbdstnew!=bbdst)
          //set_immediate_dominator (CDI_DOMINATORS, bbdstnew, recompute_dominator (CDI_POST_DOMINATORS, bbdstnew));
    }

    //printf("Miesto 8 \n");

    //calculate_dominance_info (CDI_DOMINATORS);
    //calculate_dominance_info (CDI_POST_DOMINATORS);

    //ALL_BB_print();
    //dumpcfg();
    //verify_flow_info ();   //*********************************** SPUSTIT KONTROLU KONZISTENCIE CFG
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
    if (init_codeinject());

    return 0;
}