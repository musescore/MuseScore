
#include "make_tables.h"

static void write_value(FILE *fp, double val, int i)
{
    fprintf(fp, "    %.15e%c    /* %d */\n",
        val,
        ',',
        i
           );
}

/* Emit an array of real numbers */
void emit_array(FILE *fp, const char *tblname, const double *tbl, int size)
{
    int i;

    fprintf(fp, "static const fluid_real_t %s[%d] = {\n", tblname, size);

    for (i = 0; i < size; i++)
    {
        write_value(fp, tbl[i], i);
    }
    fprintf(fp, "};\n\n");
}

/* Emit a matrix of real numbers */
void emit_matrix(FILE *fp, const char *tblname, emit_matrix_cb tbl_cb, int sizeh, int sizel)
{
    int i, j;

    fprintf(fp, "static const fluid_real_t %s[%d][%d] = {\n    {\n", tblname, sizeh, sizel);

    for (i = 0; i < sizeh; i++)
    {
        for (j = 0; j < sizel; j++)
        {
            write_value(fp, tbl_cb(i, j), i*sizel+j);
        }


        if (i < (sizeh-1))
            fprintf(fp, "    }, {\n");
        else
            fprintf(fp, "    }\n};\n\n");
    }
}

static void open_table(FILE**fp, const char* dir, const char* file)
{
    char buf[2048] = {0};
    
    strcat(buf, dir);
    strcat(buf, file);

    /* open the output file */
    *fp = fopen(buf, "w");
    if (*fp == NULL)
    {
        exit(-2);
    }
    
    /* Emit warning header */
    fprintf(*fp, "/* THIS FILE HAS BEEN AUTOMATICALLY GENERATED. DO NOT EDIT. */\n\n");
}

int main (int argc, char *argv[])
{
    FILE *fp;
 
    // make sure we have enough arguments
    if (argc < 2)
        return -1;
    
    open_table(&fp, argv[1], "fluid_conv_tables.inc.h");
    gen_conv_table(fp);
    fclose(fp);

    open_table(&fp, argv[1], "fluid_rvoice_dsp_tables.inc.h");
    gen_rvoice_table_dsp(fp);
    fclose(fp);

    return 0;
}
