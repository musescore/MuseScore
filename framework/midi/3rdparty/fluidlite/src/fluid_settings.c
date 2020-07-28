/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include "fluidsynth_priv.h"
#include "fluid_sys.h"
#include "fluid_hash.h"
#include "fluid_synth.h"
#include "fluid_settings.h"

/* maximum allowed components of a settings variable (separated by '.') */
#define MAX_SETTINGS_TOKENS	8	/* currently only a max of 3 are used */
#define MAX_SETTINGS_LABEL	256	/* max length of a settings variable label */

static void fluid_settings_init(fluid_settings_t* settings);
static void fluid_settings_hash_delete(void* value, int type);
static int fluid_settings_tokenize(const char* s, char *buf, char** ptr);


typedef struct {
  char* value;
  char* def;
  int hints;
  fluid_list_t* options;
  fluid_str_update_t update;
  void* data;
} fluid_str_setting_t;

static fluid_str_setting_t*
new_fluid_str_setting(const char* value, char* def, int hints, fluid_str_update_t fun, void* data)
{
  fluid_str_setting_t* str;
  str = FLUID_NEW(fluid_str_setting_t);
  str->value = value? FLUID_STRDUP(value) : NULL;
  str->def = def? FLUID_STRDUP(def) : NULL;
  str->hints = hints;
  str->options = NULL;
  str->update = fun;
  str->data = data;
  return str;
}

static void delete_fluid_str_setting(fluid_str_setting_t* str)
{
  if (str) {
    if (str->value) {
      FLUID_FREE(str->value);
    }
    if (str->def) {
      FLUID_FREE(str->def);
    }
    if (str->options) {
      fluid_list_t* list = str->options;

      while (list) {
	FLUID_FREE (list->data);
	list = fluid_list_next(list);
      }

      delete_fluid_list(str->options);
    }
    FLUID_FREE(str);
  }
}




typedef struct {
  double value;
  double def;
  double min;
  double max;
  int hints;
  fluid_num_update_t update;
  void* data;
} fluid_num_setting_t;


static fluid_num_setting_t*
new_fluid_num_setting(double min, double max, double def,
		     int hints, fluid_num_update_t fun, void* data)
{
  fluid_num_setting_t* setting;
  setting = FLUID_NEW(fluid_num_setting_t);
  setting->value = def;
  setting->def = def;
  setting->min = min;
  setting->max = max;
  setting->hints = hints;
  setting->update = fun;
  setting->data = data;
  return setting;
}

static void delete_fluid_num_setting(fluid_num_setting_t* setting)
{
  if (setting) {
     FLUID_FREE(setting);
  }
}




typedef struct {
  int value;
  int def;
  int min;
  int max;
  int hints;
  fluid_int_update_t update;
  void* data;
} fluid_int_setting_t;


static fluid_int_setting_t*
new_fluid_int_setting(int min, int max, int def,
		     int hints, fluid_int_update_t fun, void* data)
{
  fluid_int_setting_t* setting;
  setting = FLUID_NEW(fluid_int_setting_t);
  setting->value = def;
  setting->def = def;
  setting->min = min;
  setting->max = max;
  setting->hints = hints;
  setting->update = fun;
  setting->data = data;
  return setting;
}

static void delete_fluid_int_setting(fluid_int_setting_t* setting)
{
  if (setting) {
     FLUID_FREE(setting);
  }
}



fluid_settings_t* new_fluid_settings()
{
  fluid_settings_t* settings = new_fluid_hashtable(fluid_settings_hash_delete);
  if (settings == NULL) {
    return NULL;
  }
  fluid_settings_init(settings);
  return settings;
}

void delete_fluid_settings(fluid_settings_t* settings)
{
  delete_fluid_hashtable(settings);
}

void fluid_settings_hash_delete(void* value, int type)
{
  switch (type) {
  case FLUID_NUM_TYPE:
    delete_fluid_num_setting((fluid_num_setting_t*) value);
    break;
  case FLUID_INT_TYPE:
    delete_fluid_int_setting((fluid_int_setting_t*) value);
    break;
  case FLUID_STR_TYPE:
    delete_fluid_str_setting((fluid_str_setting_t*) value);
    break;
  case FLUID_SET_TYPE:
    delete_fluid_hashtable((fluid_hashtable_t*) value);
    break;
  }
}

void fluid_settings_init(fluid_settings_t* settings)
{
  fluid_synth_settings(settings);
}

static int fluid_settings_tokenize(const char* s, char *buf, char** ptr)
{
  char *tokstr, *tok;
  int n = 0;

  if (strlen (s) > MAX_SETTINGS_LABEL)
  {
    FLUID_LOG(FLUID_ERR, "Setting variable name exceeded max length of %d chars",
	      MAX_SETTINGS_LABEL);
    return 0;
  }

  FLUID_STRCPY(buf, s);	/* copy string to buffer, since it gets modified */
  tokstr = buf;

  while ((tok = fluid_strtok (&tokstr, ".")))
  {
    if (n > MAX_SETTINGS_TOKENS)
    {
      FLUID_LOG(FLUID_ERR, "Setting variable name exceeded max token count of %d",
		MAX_SETTINGS_TOKENS);
      return 0;
    }

    ptr[n++] = tok;
  }

  return n;
}

/** returns 1 if the value exists, 0 otherwise */
static int fluid_settings_get(fluid_settings_t* settings,
			     char** name, int len,
			     void** value, int* type)
{
  fluid_hashtable_t* table = settings;
  int t;
  void* v;
  int n;

  for (n = 0; n < len; n++) {

    if (table == NULL) {
      return 0;
    }

    if (!fluid_hashtable_lookup(table, name[n], &v, &t)) {
      return 0;
    }

    table = (t == FLUID_SET_TYPE)? (fluid_hashtable_t*) v : NULL;
  }

  if (value) {
    *value = v;
  }

  if (type) {
    *type = t;
  }

  return 1;
}

/** returns 1 if the value has been set, zero otherwise */
static int fluid_settings_set(fluid_settings_t* settings,
			     char** name, int len,
			     void* value, int type)
{
  fluid_hashtable_t* table = settings;
  int t;
  void* v;
  int n, num = len - 1;

  for (n = 0; n < num; n++) {

    if (fluid_hashtable_lookup(table, name[n], &v, &t)) {

      if (t == FLUID_SET_TYPE) {
	table = (fluid_hashtable_t*) v;
      } else {
	/* path ends prematurely */
	FLUID_LOG(FLUID_WARN, "'%s' is not a node", name[n]);
	return 0;
      }

    } else {
      /* create a new node */
      fluid_hashtable_t* tmp;
      tmp = new_fluid_hashtable(fluid_settings_hash_delete);
      fluid_hashtable_insert(table, name[n], tmp, FLUID_SET_TYPE);
      table = tmp;
    }
  }

  fluid_hashtable_replace(table, name[num], value, type);

  return 1;
}

/** returns 1 if the value has been registered correctly, 0
    otherwise */
int fluid_settings_register_str(fluid_settings_t* settings, const char* name, char* def, int hints,
			       fluid_str_update_t fun, void* data)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;
  fluid_str_setting_t* setting;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (!fluid_settings_get(settings, tokens, ntokens, &value, &type)) {
    setting = new_fluid_str_setting(def, def, hints, fun, data);
    return fluid_settings_set(settings, tokens, ntokens, setting, FLUID_STR_TYPE);

  } else {
    /* if variable already exists, don't change its value. */
    if (type == FLUID_STR_TYPE) {
      setting = (fluid_str_setting_t*) value;
      setting->update = fun;
      setting->data = data;
      setting->def = def? FLUID_STRDUP(def) : NULL;
      setting->hints = hints;
      return 1;
    } else {
      FLUID_LOG(FLUID_WARN, "Type mismatch on setting '%s'", name);
      return 1;
    }
  }
}

/** returns 1 if the value has been register correctly, zero
    otherwise */
int fluid_settings_register_num(fluid_settings_t* settings, const char* name, double def,
			       double min, double max, int hints,
			       fluid_num_update_t fun, void* data)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (!fluid_settings_get(settings, tokens, ntokens, &value, &type)) {
    /* insert a new setting */
    fluid_num_setting_t* setting;
    setting = new_fluid_num_setting(min, max, def, hints, fun, data);
    return fluid_settings_set(settings, tokens, ntokens, setting, FLUID_NUM_TYPE);

  } else {
    if (type == FLUID_NUM_TYPE) {
      /* update the existing setting but don't change its value */
      fluid_num_setting_t* setting = (fluid_num_setting_t*) value;
      setting->update = fun;
      setting->data = data;
      setting->min = min;
      setting->max = max;
      setting->def = def;
      setting->hints = hints;
      return 1;

    } else {
      /* type mismatch */
      FLUID_LOG(FLUID_WARN, "Type mismatch on setting '%s'", name);
      return 0;
    }
  }
}

/** returns 1 if the value has been register correctly, zero
    otherwise */
int fluid_settings_register_int(fluid_settings_t* settings, const char* name, int def,
			       int min, int max, int hints,
			       fluid_int_update_t fun, void* data)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (!fluid_settings_get(settings, tokens, ntokens, &value, &type)) {
    /* insert a new setting */
    fluid_int_setting_t* setting;
    setting = new_fluid_int_setting(min, max, def, hints, fun, data);
    return fluid_settings_set(settings, tokens, ntokens, setting, FLUID_INT_TYPE);

  } else {
    if (type == FLUID_INT_TYPE) {
      /* update the existing setting but don't change its value */
      fluid_int_setting_t* setting = (fluid_int_setting_t*) value;
      setting->update = fun;
      setting->data = data;
      setting->min = min;
      setting->max = max;
      setting->def = def;
      setting->hints = hints;
      return 1;

    } else {
      /* type mismatch */
      FLUID_LOG(FLUID_WARN, "Type mismatch on setting '%s'", name);
      return 0;
    }
  }
}

int fluid_settings_get_type(fluid_settings_t* settings, const char* name)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  return (fluid_settings_get(settings, tokens, ntokens, &value, &type))? type : FLUID_NO_TYPE;
}

int fluid_settings_get_hints(fluid_settings_t* settings, const char* name)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)) {
    if (type == FLUID_NUM_TYPE) {
      fluid_num_setting_t* setting = (fluid_num_setting_t*) value;
      return setting->hints;
    } else if (type == FLUID_STR_TYPE) {
      fluid_str_setting_t* setting = (fluid_str_setting_t*) value;
      return setting->hints;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

int fluid_settings_is_realtime(fluid_settings_t* settings, const char* name)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)) {
    if (type == FLUID_NUM_TYPE) {
      fluid_num_setting_t* setting = (fluid_num_setting_t*) value;
      return setting->update != NULL;

    } else if (type == FLUID_STR_TYPE) {
      fluid_str_setting_t* setting = (fluid_str_setting_t*) value;
      return setting->update != NULL;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

int fluid_settings_setstr(fluid_settings_t* settings, const char* name, const char* str)
{
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;
  int type;
  void* value;
  fluid_str_setting_t* setting;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)) {

    if (type != FLUID_STR_TYPE) {
      return 0;
    }

    setting = (fluid_str_setting_t*) value;

    if (setting->value) {
      FLUID_FREE(setting->value);
    }
    setting->value = str? FLUID_STRDUP(str) : NULL;

    if (setting->update) {
      (*setting->update)(setting->data, name, setting->value);
    }

    return 1;

  } else {
    /* insert a new setting */
    fluid_str_setting_t* setting;
    setting = new_fluid_str_setting(str, NULL, 0, NULL, NULL);
    return fluid_settings_set(settings, tokens, ntokens, setting, FLUID_STR_TYPE);
  }
}

int fluid_settings_getstr(fluid_settings_t* settings, const char* name, char** str)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_STR_TYPE)) {
    fluid_str_setting_t* setting = (fluid_str_setting_t*) value;
    *str = setting->value;
    return 1;
  }
  *str = NULL;
  return 0;
}

int fluid_settings_str_equal(fluid_settings_t* settings, const char* name, char* s)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_STR_TYPE)) {
    fluid_str_setting_t* setting = (fluid_str_setting_t*) value;
    return FLUID_STRCMP(setting->value, s) == 0;
  }
  return 0;
}

char*
fluid_settings_getstr_default(fluid_settings_t* settings, const char* name)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_STR_TYPE)) {
    fluid_str_setting_t* setting = (fluid_str_setting_t*) value;
    return setting->def;
  } else {
    return NULL;
  }
}

int fluid_settings_add_option(fluid_settings_t* settings, const char* name, char* s)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_STR_TYPE)) {
    fluid_str_setting_t* setting = (fluid_str_setting_t*) value;
    char* copy = FLUID_STRDUP(s);
    setting->options = fluid_list_append(setting->options, copy);
    return 1;
  } else {
    return 0;
  }
}

int fluid_settings_remove_option(fluid_settings_t* settings, const char* name, char* s)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_STR_TYPE)) {

    fluid_str_setting_t* setting = (fluid_str_setting_t*) value;
    fluid_list_t* list = setting->options;

    while (list) {
      char* option = (char*) fluid_list_get(list);
      if (FLUID_STRCMP(s, option) == 0) {
	FLUID_FREE (option);
	setting->options = fluid_list_remove_link(setting->options, list);
	return 1;
      }
      list = fluid_list_next(list);
    }

    return 0;
  } else {
    return 0;
  }
}

int fluid_settings_setnum(fluid_settings_t* settings, const char* name, double val)
{
  int type;
  void* value;
  fluid_num_setting_t* setting;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)) {

    if (type != FLUID_NUM_TYPE) {
      return 0;
    }

    setting = (fluid_num_setting_t*) value;

    if (val < setting->min) {
      val = setting->min;
    } else if (val > setting->max) {
      val = setting->max;
    }

    setting->value = val;

    if (setting->update) {
      (*setting->update)(setting->data, name, val);
    }

    return 1;

  } else {
    /* insert a new setting */
    fluid_num_setting_t* setting;
    setting = new_fluid_num_setting(-1e10, 1e10, 0.0f, 0, NULL, NULL);
    setting->value = val;
    return fluid_settings_set(settings, tokens, ntokens, setting, FLUID_NUM_TYPE);
  }
}

int fluid_settings_getnum(fluid_settings_t* settings, const char* name, double* val)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_NUM_TYPE)) {
    fluid_num_setting_t* setting = (fluid_num_setting_t*) value;
    *val = setting->value;
    return 1;
  }
  return 0;
}


void fluid_settings_getnum_range(fluid_settings_t* settings, const char* name, double* min, double* max)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_NUM_TYPE)) {
    fluid_num_setting_t* setting = (fluid_num_setting_t*) value;
    *min = setting->min;
    *max = setting->max;
  }
}

double
fluid_settings_getnum_default(fluid_settings_t* settings, const char* name)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_NUM_TYPE)) {
    fluid_num_setting_t* setting = (fluid_num_setting_t*) value;
    return setting->def;
  } else {
    return 0.0f;
  }
}


int fluid_settings_setint(fluid_settings_t* settings, const char* name, int val)
{
  int type;
  void* value;
  fluid_int_setting_t* setting;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)) {

    if (type != FLUID_INT_TYPE) {
      return 0;
    }

    setting = (fluid_int_setting_t*) value;

    if (val < setting->min) {
      val = setting->min;
    } else if (val > setting->max) {
      val = setting->max;
    }

    setting->value = val;

    if (setting->update) {
      (*setting->update)(setting->data, name, val);
    }

    return 1;

  } else {
    /* insert a new setting */
    fluid_int_setting_t* setting;
    setting = new_fluid_int_setting(INT_MIN, INT_MAX, 0, 0, NULL, NULL);
    setting->value = val;
    return fluid_settings_set(settings, tokens, ntokens, setting, FLUID_INT_TYPE);
  }
}

int fluid_settings_getint(fluid_settings_t* settings, const char* name, int* val)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_INT_TYPE)) {
    fluid_int_setting_t* setting = (fluid_int_setting_t*) value;
    *val = setting->value;
    return 1;
  }
  return 0;
}


void fluid_settings_getint_range(fluid_settings_t* settings, const char* name, int* min, int* max)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_INT_TYPE)) {
    fluid_int_setting_t* setting = (fluid_int_setting_t*) value;
    *min = setting->min;
    *max = setting->max;
  }
}

int
fluid_settings_getint_default(fluid_settings_t* settings, const char* name)
{
  int type;
  void* value;
  char* tokens[MAX_SETTINGS_TOKENS];
  char buf[MAX_SETTINGS_LABEL+1];
  int ntokens;

  ntokens = fluid_settings_tokenize(name, buf, tokens);

  if (fluid_settings_get(settings, tokens, ntokens, &value, &type)
      && (type == FLUID_INT_TYPE)) {
    fluid_int_setting_t* setting = (fluid_int_setting_t*) value;
    return setting->def;
  } else {
    return 0.0f;
  }
}





