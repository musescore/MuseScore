#include "test.h"
#include "fluidsynth.h"
#include "fluid_sfont.h"
#include "fluid_defsfont.h"
#include "fluid_sys.h"

static void dump_sample(fluid_sample_t *sample);
static void dump_gens(const fluid_gen_t gen[]);
static void dump_mod(const fluid_mod_t *mod);
static void dump_preset_zone(fluid_preset_zone_t *zone);
static void dump_preset(fluid_preset_t *preset);
static void dump_inst_zone(fluid_inst_zone_t *zone);
static void dump_inst(fluid_inst_t *inst);
static void dump_defsfont(fluid_defsfont_t *defsfont);
static int inst_compare_func(const void *a, const void *b);
static fluid_list_t *collect_preset_insts(fluid_preset_t *preset, fluid_list_t *inst_list);

#define FMT_BUFSIZE (4096)
static int indent_level = 0;
static FILE *output = NULL;

static void fmt(const char *format, ...);
static void indent(void);
static void outdent(void);


int main(int argc, char **argv)
{
    int ret = FLUID_FAILED;
    int id;
    fluid_sfont_t *sfont;
    fluid_defsfont_t *defsfont;
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    const char *adrivers[1] = { NULL };

    if (argc < 2)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "    dump_sfont <input_soundfont> [output_file]\n");
        return FLUID_FAILED;
    }

    fluid_audio_driver_register(adrivers);

    settings = new_fluid_settings();
    if (settings == NULL)
    {
        return FLUID_FAILED;
    }

    synth = new_fluid_synth(settings);
    if (synth == NULL)
    {
        goto EXIT;
    }

    id = fluid_synth_sfload(synth, argv[1], 1);
    if (id < 0)
    {
        goto EXIT;
    }

    sfont = fluid_synth_get_sfont_by_id(synth, id);
    if (sfont == NULL)
    {
        goto EXIT;
    }

    if (sfont->free != &fluid_defsfont_sfont_delete)
    {
        fprintf(stderr, "This tool only supports SoundFonts loaded by the default loader\n");
        goto EXIT;
    }

    defsfont = (fluid_defsfont_t *)fluid_sfont_get_data(sfont);
    if (defsfont == NULL)
    {
        goto EXIT;
    }

    if (argc < 3)
    {
        output = stdout;
    }
    else
    {
        output = fopen(argv[2], "w");
        if (output == NULL)
        {
            fprintf(stderr, "Unable to open output file %s", argv[2]);
            goto EXIT;
        }
    }

    dump_defsfont(defsfont);

    ret = FLUID_OK;

EXIT:
    if (output && output != stdout)
    {
        fclose(output);
    }
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);

    return ret;
}

static void dump_sample(fluid_sample_t *sample)
{
    fmt("name: %s", sample->name);
    fmt("source_start: %u", sample->source_start);
    fmt("source_end: %u", sample->source_end);
    fmt("source_loopstart: %u", sample->source_loopstart);
    fmt("source_loopend: %u", sample->source_loopend);

    fmt("start: %u", sample->start);
    fmt("end: %u", sample->end);
    fmt("loopstart: %u", sample->loopstart);
    fmt("loopend: %u", sample->loopend);

    fmt("samplerate: %u", sample->samplerate);
    fmt("origpitch: %u", sample->origpitch);
    fmt("pitchadj: %u", sample->pitchadj);
    fmt("sampletype: %u", sample->sampletype);
}

static void dump_gens(const fluid_gen_t gen[])
{
    int i;

    /* only dump generators if at least one is set */
    for (i = 0; i < GEN_LAST; i++)
    {
        if (gen[i].flags)
        {
            break;
        }
    }

    if (i == GEN_LAST)
    {
        return;
    }

    fmt("generators:");
    indent();
    for (i = 0; i < GEN_LAST; i++)
    {
        if (gen[i].flags)
        {
            fmt("%s: %.2f", fluid_gen_name(i), gen[i].val);
        }
    }
    outdent();
}

static void dump_mod(const fluid_mod_t *mod)
{
    fmt("dest: %s", fluid_gen_name(mod->dest));
    fmt("src1: %u", mod->src1);
    fmt("flags1: %u", mod->flags1);
    fmt("src2: %u", mod->src2);
    fmt("flags2: %u", mod->flags2);
    fmt("amount: %.2f", mod->amount);
}

static void dump_preset_zone(fluid_preset_zone_t *zone)
{
    int i;
    fluid_mod_t *mod;

    fmt("name: %s", zone->name);
    if (zone->inst)
    {
        fmt("instrument: %s (index %d)", zone->inst->name, zone->inst->source_idx);
    }
    fmt("key_range: %d - %d", zone->range.keylo, zone->range.keyhi);
    fmt("vel_range: %d - %d", zone->range.vello, zone->range.velhi);
    dump_gens(zone->gen);

    if (zone->mod)
    {
        fmt("modulators:");
        for (i = 0, mod = zone->mod; mod; mod = mod->next, i++)
        {
            fmt("- modulator: %d", i);
            indent();
            dump_mod(mod);
            outdent();
        }
    }
}

static void dump_preset(fluid_preset_t *preset)
{
    int i;
    fluid_preset_zone_t *zone;

    fluid_defpreset_t *defpreset = fluid_preset_get_data(preset);
    if (defpreset == NULL)
    {
        return;
    }

    fmt("name: %s", defpreset->name);
    fmt("bank: %u", defpreset->bank);
    fmt("num: %u", defpreset->num);

    if (defpreset->global_zone)
    {
        fmt("global_zone:");
        indent();
        dump_preset_zone(defpreset->global_zone);
        outdent();
    }

    fmt("zones:");
    for (i = 0, zone = defpreset->zone; zone; zone = fluid_preset_zone_next(zone), i++)
    {
        fmt("- zone: %d", i);
        if (zone == NULL)
        {
            continue;
        }

        indent();
        dump_preset_zone(zone);
        outdent();
    }
}

static void dump_inst_zone(fluid_inst_zone_t *zone)
{
    int i;
    fluid_mod_t *mod;

    fmt("name: %s", zone->name);
    if (zone->sample)
    {
        fmt("sample: %s", zone->sample->name);
    }
    fmt("key_range: %d - %d", zone->range.keylo, zone->range.keyhi);
    fmt("vel_range: %d - %d", zone->range.vello, zone->range.velhi);
    dump_gens(zone->gen);
    if (zone->mod)
    {
        fmt("modulators:");
        for (i = 0, mod = zone->mod; mod; mod = mod->next, i++)
        {
            fmt("- modulator: %d", i);
            indent();
            dump_mod(mod);
            outdent();
        }
    }
}

static void dump_inst(fluid_inst_t *inst)
{
    int i;
    fluid_inst_zone_t *zone;

    fmt("name: %s", inst->name);

    if (inst->global_zone)
    {
        fmt("global_zone:");
        indent();
        dump_inst_zone(inst->global_zone);
        outdent();
    }

    fmt("zones:");
    for (i = 0, zone = inst->zone; zone; zone = fluid_inst_zone_next(zone), i++)
    {
        fmt("- zone: %d", i);
        if (zone == NULL)
        {
            continue;
        }

        indent();
        dump_inst_zone(zone);
        outdent();
    }
}

static int inst_compare_func(const void *a, const void *b)
{
    const fluid_inst_t *inst_a = a;
    const fluid_inst_t *inst_b = b;

    return inst_a->source_idx - inst_b->source_idx;
}

static fluid_list_t *collect_preset_insts(fluid_preset_t *preset, fluid_list_t *inst_list)
{
    fluid_preset_zone_t *zone;
    fluid_defpreset_t *defpreset = fluid_preset_get_data(preset);
    if (defpreset == NULL)
    {
        return inst_list;
    }

    if (defpreset->global_zone && defpreset->global_zone->inst &&
        fluid_list_idx(inst_list, defpreset->global_zone->inst) == -1)
    {
        inst_list = fluid_list_prepend(inst_list, defpreset->global_zone->inst);
    }

    for (zone = defpreset->zone; zone; zone = fluid_preset_zone_next(zone))
    {
        if (zone->inst && (fluid_list_idx(inst_list, zone->inst) == -1))
        {
            inst_list = fluid_list_prepend(inst_list, zone->inst);
        }
    }

    return inst_list;
}


static void dump_defsfont(fluid_defsfont_t *defsfont)
{
    int i;
    fluid_list_t *list;
    fluid_sample_t *sample;
    fluid_preset_t *preset;
    fluid_inst_t *inst;
    fluid_list_t *inst_list = NULL;

    fmt("samplepos: %u", defsfont->samplepos);
    fmt("samplesize: %u", defsfont->samplesize);
    fmt("sample24pos: %u", defsfont->sample24pos);
    fmt("sample24size: %u", defsfont->sample24size);

    fmt("presets:");
    for (i = 0, list = defsfont->preset; list; list = fluid_list_next(list), i++)
    {
        preset = (fluid_preset_t *)fluid_list_get(list);
        fmt("- preset: %d", i);
        if (preset == NULL)
        {
            continue;
        }

        indent();
        dump_preset(preset);
        outdent();
        fmt("");

        inst_list = collect_preset_insts(preset, inst_list);
    }

    inst_list = fluid_list_sort(inst_list, inst_compare_func);

    fmt("instruments:");
    for (list = inst_list; list; list = fluid_list_next(list))
    {
        inst = (fluid_inst_t *)fluid_list_get(list);
        fmt("- instrument: %d", inst->source_idx);
        indent();
        dump_inst(inst);
        outdent();
        fmt("");
    }

    delete_fluid_list(inst_list);

    fmt("samples:");
    for (i = 0, list = defsfont->sample; list; list = fluid_list_next(list), i++)
    {
        sample = (fluid_sample_t *)fluid_list_get(list);
        fmt("- sample: %d", i);
        if (sample == NULL)
        {
            continue;
        }

        indent();
        dump_sample(sample);
        outdent();
        fmt("");
    }
}


static void fmt(const char *format, ...)
{
    char buf[FMT_BUFSIZE];
    va_list args;
    int len;
    int i;

    va_start(args, format);
    len = FLUID_VSNPRINTF(buf, FMT_BUFSIZE, format, args);
    va_end(args);

    if (len < 0)
    {
        FLUID_LOG(FLUID_ERR, "max buffer size exceeded");
        return;
    }

    buf[FMT_BUFSIZE - 1] = '\0';

    for (i = 0; i < indent_level; i++)
    {
        fprintf(output, "  ");
    }

    fwrite(buf, 1, FLUID_STRLEN(buf), output);
    fprintf(output, "\n");
}

static void indent(void)
{
    indent_level += 1;
}

static void outdent(void)
{
    if (indent_level > 0)
    {
        indent_level -= 1;
    }
}
